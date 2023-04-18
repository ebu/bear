import numpy as np
from .data_file import get_path


class SOFAError(Exception):
    pass


def load_hdf5(file_url):
    """Load a HDF5 file from a URL of the form:

    `file:PATH`: load from file at PATH
    `resource:PATH`: load from package resource PATH
    """
    import h5py

    return h5py.File(get_path(file_url), "r")


class SOFAFile(object):
    def __init__(self, f):
        self.f = f

        self.check()

        self.listener_mats = self.get_orientations("Listener")

    def check(self):
        assert "SourcePosition" in self.f
        assert "EmitterPosition" in self.f

        assert all(
            delay_s == ir_s or delay_s == 1
            for delay_s, ir_s in zip(
                self.f["Data.Delay"].shape, self.f["Data.IR"].shape
            )
        )

    def check_convention(self, expected):
        convention = self.f.attrs["SOFAConventions"]
        if convention != expected:
            raise ValueError(f"unexpected SOFAConventions: expected {expected} but found {convention}")

    def get_positions(self, name, axis=-1, default_type_units=None):
        """Get positions of the given name, converting them to the ADM
        cartesian coordinate system, assuming that the 3 coordinated are on
        axis.
        """
        # sofa coordinate system:
        # https://www.sofaconventions.org/mediawiki/index.php/SOFA_specifications
        # same as ADM, except that x and y are rotated; the front of the room
        # is along +x in the diagram
        pos = self.f[name]

        try:
            pos_type, pos_units = pos.attrs["Type"], pos.attrs["Units"]
        except KeyError:
            if default_type_units is not None:
                pos_type, pos_units = default_type_units
            else:
                raise

        pos = np.moveaxis(pos, axis, -1)

        unit_aliases_meter = (b"meter", b"metre", b"meters")
        unit_aliases_degree = (b"degree", b"degrees")

        if pos_type == b"spherical":
            from ear.core.geom import cart

            unit_list = [v.strip() for v in pos_units.split(b",")]
            assert len(unit_list) == 3
            assert unit_list[0] in unit_aliases_degree
            assert unit_list[1] in unit_aliases_degree
            assert unit_list[2] in unit_aliases_meter
            assert pos.shape[-1] == 3
            return np.moveaxis(cart(pos[..., 0], pos[..., 1], pos[..., 2]), -1, axis)
        elif pos_type == b"cartesian":
            M = np.array([[0, 1, 0], [-1, 0, 0], [0, 0, 1]])
            assert pos_units in unit_aliases_meter
            return np.moveaxis(np.dot(pos, M), -1, axis)
        else:
            assert False

    def get_positions_rel(self, name, axis=-1):
        """Get positions relative to the look direction"""
        positions = np.moveaxis(self.get_positions(name, axis), axis, -1)
        return np.moveaxis(
            np.einsum("...ji,...j", self.listener_mats, positions), -1, axis
        )

    def get_orientations(self, name):
        """Matrix for each item which maps from a vector in that item to a
        world vector.

        For example, if name="Emitter", M is a matrix returned by this
        function, and v is a vector relative to an emitter (e.g. [0, 1, 0] for
        the direction of the emitter), then dot(M, v) is the equivalent in word
        space (e.g. the emitter view direction).
        """
        view, up = np.broadcast_arrays(
            self.get_positions(name + "View", default_type_units=(b"cartesian", b"meter"), axis=1),
            self.get_positions(
                name + "Up", default_type_units=(b"cartesian", b"meter"), axis=1
            ),
        )

        view = view / np.linalg.norm(view, axis=1, keepdims=True)
        up = up / np.linalg.norm(up, axis=1, keepdims=True)

        right = np.cross(view, up, axis=1)

        if np.max(np.abs(np.linalg.norm(right, axis=1) - 1.0)) > 1e-6:
            raise SOFAError("view and up vectors are not orthogonal")

        return np.stack((right, view, up), 2)

    def select_receivers(self):
        """Get the index of the left then right ear"""
        rp = self.get_positions("ReceiverPosition", axis=1)
        assert rp.shape == (2, 3, 1)

        x = rp[:, 0, 0]

        return np.array([0, 1] if x[0] < x[1] else [1, 0])

    @classmethod
    def _select_positions_direct(
        cls, all_positions, positions, exact, direction_only=False
    ):
        """see select_positions"""
        if direction_only:
            # Dot product returns the cosine of the angular distances
            # Both hrir grid and positions must be normalised.
            cos_angles = np.einsum(
                "kj,...j->k...",
                all_positions / np.linalg.norm(all_positions, axis=-1, keepdims=True),
                positions / np.linalg.norm(positions, axis=-1, keepdims=True),
            )
            source_idxes = np.argmax(cos_angles, axis=0)
            if exact:
                dist_limit = 0.1  # Maximum distance limit in degree
                err_acos = cos_angles[source_idxes, np.arange(source_idxes.shape[0])]
                assert np.all(err_acos >= np.cos(np.deg2rad(dist_limit)))
        else:
            distances = np.linalg.norm(
                positions[..., np.newaxis, :] - all_positions, axis=-1
            )
            source_idxes = np.argmin(distances, -1)
            if exact:
                assert np.max(np.min(distances, -1)) < 1e-5

        return source_idxes

    @classmethod
    def select_positions(cls, all_positions, positions, exact, direction_only=False):
        """Get the indices of positions in all_positions.

        Parameters:
            all_positions (ndarray of (n, 3)): positions to select indices from
            positions (ndarray of (..., 3)): positions to select
            exact (bool): must the positions match exactly?
            direction_only (bool): Whether to select by azimuth and elevation only.
        Returns:
            indices (ndarray of (...))
        """
        # this is a chunked wrapper around _select_positions_direct, which
        # reduces the allocation of the distance/angle matrices down to a more
        # manageable size

        # add an axis at the start and remove it afterwards to ensure that the
        # chunk iteration is at least 1D
        positions = positions[np.newaxis]

        pos_shape = positions.shape[:-1]

        n = int(np.product(pos_shape))
        chunk = int(1e6 // len(all_positions.flat))
        out = np.zeros(pos_shape, dtype=np.int64)
        for i in range(0, n, chunk):
            idx_flat = np.arange(i, min(i + chunk, n))
            idx = np.unravel_index(idx_flat, pos_shape)

            out[idx] = cls._select_positions_direct(
                all_positions, positions[idx], exact, direction_only
            )

        return out[0]

    @classmethod
    def apply_delays(cls, irs, delays):
        """Apply delays to a set of IRs, and strip off leading/trailing zeros.

        note: delays are rounded to the closest sample
        """
        int_delays = np.round(delays).astype(int)

        max_delay = np.max(int_delays)

        irs_delay = np.zeros(delays.shape + (irs.shape[-1] + max_delay,))

        idx = tuple(np.indices(irs.shape))
        idx_delay = idx[:-1] + (idx[-1] + int_delays[idx[:-1]],)
        irs_delay[idx_delay] = irs

        nz = np.nonzero(irs_delay)
        first_nz, last_nz = np.min(nz[-1]), np.max(nz[-1])

        return irs_delay[..., first_nz : last_nz + 1]


class SOFAFileHRIR(SOFAFile):
    """Simple SOFA interface allowing IRs to be extracted from a
    SimpleFreeFieldHRIR convention file.
    """

    def check(self):
        super(SOFAFileHRIR, self).check()
        self.check_convention(b"SimpleFreeFieldHRIR")

    def source_positions(self):
        return self.get_positions_rel("SourcePosition")

    def select_sources(self, positions, exact, direction_only=False):
        return self.select_positions(
            self.source_positions(), positions, exact, direction_only
        )

    def irs_for_positions(self, positions, fs, exact=False, direction_only=False):
        """Get the impulse responses at the given positions.

        Parameters:
            positions (ndarray of (..., 3)): positions to find
            fs (int): sampling frequency in Hz
            exact (bool): must the positions match exactly?
            direction_only (bool): Whether to select by azimuth and elevation only.

        Returns:
            n-sample impulse responses in the shape (..., 2, n), where axis -2
            contains the responses for the left then right ears
        """
        assert self.f["Data.SamplingRate"][0] == fs

        sources = self.select_sources(positions, exact, direction_only)
        recievers = self.select_receivers()

        irs = fancy_index(self.f["Data.IR"], sources[..., np.newaxis], recievers)
        delays = fancy_index_collapse(
            self.f["Data.Delay"], sources[..., np.newaxis], recievers
        )

        return self.apply_delays(irs, delays)


class SOFAFileBRIR(SOFAFile):
    """Simple SOFA interface allowing IRs to be extracted from a
    SimpleFreeFieldBRIR convention file.
    """

    def check(self):
        super(SOFAFileBRIR, self).check()
        self.check_convention(b"MultiSpeakerBRIR")
        assert np.all(np.array(self.f["Data.Delay"]) == 0)

    def emitter_positions(self):
        return self.get_positions("EmitterPosition", axis=1)[:, :, 0]

    def emitter_orientations(self):
        return self.get_orientations("Emitter")[:, :, :, 0]

    def listener_views(self):
        return self.get_positions("ListenerView")

    def irs_for_positions_views(
        self,
        positions,
        views,
        fs,
        exact_positions=True,
        exact_views=False,
        direction_only=False,
    ):
        """Get the impulse responses at the given emitter positions and views.

        Parameters:
            positions (ndarray of (..., 3)): emitter positions to find
            views (ndarray of (..., 3)): listener views to find
            fs (int): sampling frequency in Hz
            exact_positions (bool): must positions match exactly?
            exact_views (bool): must views match exactly?
            direction_only (bool): Whether to ignore emitter positions and
                select by direction only.
        Returns:
            n-sample impulse responses in the shape (..., 2, n), where axis -2
            contains the responses for the left then right ears
        """
        positions, views = np.broadcast_arrays(positions, views)

        position_idx = self.select_positions(
            self.emitter_positions(), positions, exact_positions, direction_only
        )
        view_idx = self.select_positions(
            self.listener_views(), views, exact_views, direction_only=True
        )
        recievers = self.select_receivers()

        return fancy_index(
            self.f["Data.IR"],
            view_idx[..., np.newaxis],
            recievers,
            position_idx[..., np.newaxis],
        )


def fancy_index(dataset, *indices):
    """Fancy indexing for h5py datasets which works around some of the restrictions."""
    indices = np.broadcast_arrays(*indices)

    # because of h5py restrictions we can only index along one axis before the
    # data is turned into a numpy array; pick this axis; the one which reduces
    # the dataset size the most
    axis = np.argmin(
        [len(np.unique(idx)) / dataset.shape[i] for i, idx in enumerate(indices)]
    )
    axis_indices = indices[axis]

    # indices have to be unique and increasing
    h5_idx, inverse = np.unique(axis_indices, return_inverse=True)
    inverse = inverse.reshape(axis_indices.shape)

    # select just along axis; this results in an array the same shape as dataset,
    # except along axis
    h5_idx_expr = list(
        np.s_[
            :,
        ]
        * len(indices)
    )
    h5_idx_expr[axis] = h5_idx
    subset = dataset[tuple(h5_idx_expr)]

    # complete the indexing; this is the same as if we'd just turned the full
    # array into a numpy dataset, except along axis; this was reordered, so the
    # indices are given by inverse
    np_idx_expr = list(indices)
    np_idx_expr[axis] = inverse
    return subset[tuple(np_idx_expr)]


def fancy_index_collapse(dataset, *indices):
    """Fancy indexing, except that any axes in the dataset of size 1 are not
    indexed. This is used in SOFA files to represent axes where all the values
    are the same."""
    indices = [
        np.zeros_like(i) if size == 1 else i for i, size in zip(indices, dataset.shape)
    ]

    return fancy_index(dataset, *indices)
