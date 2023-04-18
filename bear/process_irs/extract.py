from bear.sofa import load_hdf5, SOFAFile, SOFAFileBRIR, SOFAFileHRIR
from bear.layout import get_layout, layout_to_json
from bear import tensorfile
import dask.array as da
import numpy as np
from ear.core.geom import cart, elevation


def parse_args():
    import argparse

    parser = argparse.ArgumentParser()

    in_group = parser.add_mutually_exclusive_group(required=True)
    in_group.add_argument("--brir-in", help="input SOFA BRIR file")
    in_group.add_argument("--hrir-in", help="input SOFA HRIR file")
    parser.add_argument("--tf-out", help="output TF data file", required=True)
    parser.add_argument("--layout", help="layout name", default="9+10+3_extra_rear")

    parser.add_argument(
        "--ir-selection-algo",
        help="algorithm used to select sources from IR set",
        default="smart",
        choices=("smart", "closest"),
    )

    parser.add_argument(
        "--num-views",
        help="number of views to select (use all available if not specified)",
        type=int,
    )

    return parser.parse_args()


def adapt_layout(layout, positions):
    from ear.core.layout import Speaker, PolarPosition
    from ear.core.geom import azimuth, elevation

    def round_if_close(x):
        """round values close to whole numbers"""
        if abs(x - round(x)) < 1e-6:
            return float(round(x))
        else:
            return x

    speakers = [
        Speaker(
            channel=i,
            names=[channel.name],
            polar_position=PolarPosition(
                azimuth=round_if_close(azimuth(pos)),
                elevation=round_if_close(elevation(pos)),
                distance=1.0,
            ),
        )
        for i, (channel, pos) in enumerate(zip(layout.channels, positions))
    ]
    layout, upmix = layout.with_speakers(speakers)
    assert upmix.shape == (len(layout.channels), len(layout.channels))
    assert np.all(upmix == np.eye(upmix.shape[0]))

    return layout


def unique_within_tolerance(xs, tol):
    unique = []
    for x in xs:
        if not any(abs(x - other_x) < 1e-6 for other_x in unique):
            unique.append(x)
    return np.array(unique)


def select_positions_smart(source_positions, speaker_positions):
    """select positions from source_positions for speaker_positions, with some
    heuristics to avoid problems with just selecting the closest

    There are three rules which affect which sources can be chosen for a given
    loudspeaker:

    - if the speaker is on the median plane, only select sources on the median
      plane

    - if the speaker is a left-to-right mirror of a speaker which has already
      been allocated, the position used is the mirror of the source position of
      the mirror speaker. this means that, if the IR set is symmetric, pairs of
      speakers are allocated to symmetrical sources

    - all speakers with the same elevation (within tolerance) are allocated to
      sources with the same elevation (again, within tolerance). this means
      that if the speakers are exactly between two layers, the choice between
      layers with be consistent, not decided randomly per-loudspeaker, based on
      tolerance

    The closest source position that satisfies these rules for each speaker is
    chosen.

    This can go wrong, for example with quadrature layouts where we would not
    expect to be able to find sources with consistent elevations.

    Parameters:
        source_positions (ndarray of (n, 3)): source/IR positions
        speaker_positions (ndarray of (m, 3)): loudspeaker positions

    Returns:
        ndarray of (m,):  source index for each speaker
    """

    source_positions = source_positions / np.linalg.norm(
        source_positions, axis=1, keepdims=True
    )
    speaker_positions = speaker_positions / np.linalg.norm(
        speaker_positions, axis=1, keepdims=True
    )

    source_els = elevation(source_positions)
    unique_source_els = unique_within_tolerance(source_els, 1e-6)
    speaker_els = unique_within_tolerance(elevation(speaker_positions), 1e-6)

    idxes = np.zeros(len(speaker_positions), dtype=int)

    median_plane_sources = np.abs(source_positions[:, 0]) < 1e-6

    for speaker_i, speaker_pos in enumerate(speaker_positions):
        selected_sources = np.ones(len(source_positions), dtype=bool)

        if np.abs(speaker_pos[0]) < 1e-6:
            # median-plane speakers are only allocated median-plane sources
            selected_sources &= median_plane_sources
        elif speaker_i > 0:
            # if we've already allocated the mirror of this speaker, use the
            # mirrored source position
            mirrored_speaker_pos = speaker_pos * [-1, 1, 1]
            mirrored_distances = np.linalg.norm(
                speaker_positions[:speaker_i] - mirrored_speaker_pos, axis=1
            )
            if np.min(mirrored_distances) < 1e-5:
                mirrored_source_pos = source_positions[
                    idxes[np.argmin(mirrored_distances)]
                ]
                speaker_pos = mirrored_source_pos * [-1, 1, 1]

        # select the closest unique elevation
        el = elevation(speaker_pos)
        closest_unique_el = min(speaker_els, key=lambda unique_el: abs(el - unique_el))
        closest_source_el = min(
            unique_source_els, key=lambda source_el: abs(closest_unique_el - source_el)
        )
        selected_sources &= np.abs(source_els - closest_source_el) < 1.5e-6

        if not np.any(selected_sources):
            raise RuntimeError("IR selection failed - no sources selected")

        distances = np.linalg.norm(
            source_positions[selected_sources] - speaker_pos, axis=1
        )
        closest_i_distances = np.argmin(distances)
        closest_i = np.where(selected_sources)[0][closest_i_distances]

        idxes[speaker_i] = closest_i

    return idxes


def main(args):
    if args.brir_in is not None:
        sofa_in_f = load_hdf5(args.brir_in)
        sofa_in = SOFAFileBRIR(sofa_in_f)
        irs = da.from_array(sofa_in_f["Data.IR"], chunks=sofa_in_f["Data.IR"].chunks)
        # irs is (view, ear, emitter, sample), want (view, emitter, ear, sample)
        irs = da.swapaxes(irs, 1, 2)
        irs = irs[:, :, sofa_in.select_receivers()]

        positions = sofa_in.emitter_positions()
        views = sofa_in.listener_views()
    elif args.hrir_in is not None:
        sofa_in_f = load_hdf5(args.hrir_in)
        sofa_in = SOFAFileHRIR(sofa_in_f)

        irs = da.from_array(sofa_in_f["Data.IR"], chunks=sofa_in_f["Data.IR"].chunks)
        # irs is (emitter, ear, sample), want (view, emitter, ear, sample)
        irs = irs[np.newaxis]
        irs = irs[:, :, sofa_in.select_receivers()]

        positions = sofa_in.source_positions()
        views = np.array([[0, 1, 0]], dtype=float)
    else:
        assert False

    fs = sofa_in_f["Data.SamplingRate"][0]

    assert len(irs.shape) == 4
    assert irs.shape[2] == 2

    assert positions.shape == (irs.shape[1], 3)
    positions /= np.linalg.norm(positions, axis=1, keepdims=True)

    assert views.shape == (irs.shape[0], 3)
    views /= np.linalg.norm(views, axis=1, keepdims=True)

    layout = get_layout(args.layout).without_lfe

    if args.ir_selection_algo == "smart":
        position_idx = select_positions_smart(positions, layout.positions)
    elif args.ir_selection_algo == "closest":
        position_idx = SOFAFile.select_positions(
            positions, layout.positions, exact=False, direction_only=True
        )
    else:
        assert False

    assert len(position_idx) == len(layout.channels)
    if len(set(position_idx)) < len(position_idx):
        raise RuntimeError("IR selection returned duplicate IRs")

    irs = irs[:, position_idx]
    positions = positions[position_idx]

    if args.num_views is not None:
        view_select_az = np.linspace(0, 360, args.num_views, endpoint=None)
        view_select_pos = cart(view_select_az, 0, 1)
        views_idx = SOFAFile.select_positions(
            views, view_select_pos, exact=True, direction_only=True
        )

        views = views[views_idx]
        irs = irs[views_idx]

    layout = adapt_layout(layout, positions)

    output = dict(
        views=views.astype(np.float32),
        # shape (view, loudspeaker, ear, sample)
        brirs=irs.astype(np.float32).compute(),
        fs=fs,
        positions=positions.astype(np.float32),
        layout=layout_to_json(layout),
    )

    with open(args.tf_out, "wb") as f:
        tensorfile.write(f, output)


if __name__ == "__main__":
    main(parse_args())
