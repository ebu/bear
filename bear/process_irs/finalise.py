import bear.tensorfile
import dask.array as da
import ear.core.layout
import ear.core.objectbased.decorrelate
import numpy as np
import scipy.signal as sig
from bear.irs import apply_delays
from bear.layout import layout_from_json, layout_to_json
from bear.sofa import load_hdf5


def parse_args():
    import argparse

    parser = argparse.ArgumentParser()

    parser.add_argument("--tf-in", help="input TF file", required=True)
    parser.add_argument("--tf-out", help="output TF data file", required=True)
    parser.add_argument(
        "--delays", help="delays for sofa IRs in .npy format", required=True
    )
    parser.add_argument(
        "--hoa-decoder",
        help="SOFA file containing HOA decoder",
    )
    parser.add_argument(
        "--use-dask-for-factors",
        help="use dask to compute factors; fast but uses lots of memory",
        action="store_true",
    )

    parser.add_argument(
        "--window",
        choices=("none", "raised_cos", "exponential"),
        default="raised_cos",
        help="apply window to impulse responses (for BRIRs)",
    )

    parser.add_argument("--end-win-start", default=0.007, type=float)
    parser.add_argument("--end-win-end", default=0.06, type=float)

    parser.add_argument("--extra-out", help="output TF for extra data for report")

    parser.add_argument('--gain-norm', action=argparse.BooleanOptionalAction,
                        default=True,
                        help="enable gain normalisation factors",
    )

    return parser.parse_args()


def dask_delay(irs, delays, out_len=None, osa=1):
    import dask.array as da

    if out_len is None:
        max_delay = da.max(delays).compute()
        extra_samples = ((max_delay + osa - 1) // osa) * osa
        out_len = irs.shape[-1] + extra_samples

    delays = delays[..., np.newaxis]
    irs, delays = da.broadcast_arrays(irs, delays)
    delays = delays[..., 0]

    return da.map_blocks(
        lambda ir, delay: apply_delays(ir, delay[..., 0], out_len=out_len),
        irs.rechunk(irs.chunks[:-1] + (-1,)),
        delays.rechunk(irs.chunks[:-1])[..., np.newaxis],
        chunks=irs.chunks[:-1] + (out_len,),
        dtype=irs.dtype,
    )


def resample(signals, length, axis):
    return da.apply_along_axis(
        lambda arr: sig.resample(arr, length),
        axis,
        signals,
        shape=(length,),
        dtype=signals.dtype,
    )


def make_raised_cos_window(fs, args):
    start_win_start = -int(0.002 * fs)
    start_win_end = -int(0.001 * fs)
    end_win_start = int(args.end_win_start * fs)
    end_win_end = int(args.end_win_end * fs)

    window = 0.5 * (
        1
        - np.cos(
            np.interp(
                np.arange(start_win_start, end_win_end),
                [start_win_start, start_win_end, end_win_start, end_win_end],
                [0, np.pi, np.pi, 0],
            )
        )
    )

    return start_win_start, window


def make_exponential_window(
    fs, end_win_start=0.007, end_win_end=0.06, end_win_end_db=-30
):
    start_win_start = -int(0.002 * fs)
    start_win_end = -int(0.001 * fs)

    end_cos_win_start = int((end_win_end - 0.001) * fs)
    end_cos_win_end = int((end_win_end + 0.001) * fs)

    end_win_start = int(end_win_start * fs)
    end_win_end = int(end_win_end * fs)

    t = np.arange(start_win_start, end_cos_win_end)

    cos_window = 0.5 * (
        1
        - np.cos(
            np.interp(
                t,
                [start_win_start, start_win_end, end_cos_win_start, end_cos_win_end],
                [0, np.pi, np.pi, 0],
            )
        )
    )

    exp_window_db = np.interp(
        t,
        [end_win_start, end_win_end],
        [0, end_win_end_db],
    )
    exp_window = 10.0 ** (exp_window_db / 20)

    window = cos_window * exp_window

    return start_win_start, window


def make_window(fs, window, args):
    if window == "raised_cos":
        return make_raised_cos_window(fs, args)
    if window == "exponential":
        return make_exponential_window(fs)
    else:
        raise ValueError(f"unknown window type {window}")


def normalise_front(irs, positions):
    """Normalise the brir set such that the gain of the front BRIRs is 1."""
    front = np.array([0.0, 1.0, 0.0])
    front_idx = np.argmin(np.linalg.norm(positions - front, axis=-1))
    comp = 1.0 / np.linalg.norm(irs[0, front_idx])

    return irs * comp


def calc_factor_matrix(brirs, decorrelation_filters, delays, dec_delay, fs):
    # delays to apply in diffuse path
    round_delays = np.round(delays).astype(int)

    # delays to apply in direct path
    max_delay = np.max(np.ceil(delays).astype(int)) + 2
    delay_samples = np.arange(max_delay)

    # (view, loudspeaker, ear, sample)
    brirs_decorrelated = sig.oaconvolve(
        brirs, decorrelation_filters[np.newaxis, :, np.newaxis], axes=-1
    )

    ir_len = max(
        brirs_decorrelated.shape[-1] + np.max(round_delays),
        brirs.shape[-1] + dec_delay + np.max(delay_samples),
    )

    brirs_decorrelated_delayed = apply_delays(
        brirs_decorrelated, np.round(delays).astype(int), out_len=ir_len
    )

    n_views, n_speakers, n_ears = brirs.shape[:-1]

    brirs = da.from_array(brirs, chunks=(1, -1, -1, -1))
    brirs_decorrelated_delayed = da.from_array(
        brirs_decorrelated_delayed, chunks=(1, -1, -1, -1)
    )

    # (delay, view, loudspeaker, ear, sample)
    brirs_delayed = dask_delay(
        brirs[np.newaxis],
        delay_samples[:, np.newaxis, np.newaxis, np.newaxis] + dec_delay,
        out_len=ir_len,
    )

    # (delay, view, loudspeaker*2, ear, sample)
    all_brirs = da.concatenate(
        da.broadcast_arrays(brirs_delayed, brirs_decorrelated_delayed[np.newaxis]),
        axis=2,
    )

    # (delay, view, loudspeaker*2, loudspeaker*2, ear)
    factors = da.einsum("dvles,dvmes->dvlme", all_brirs, all_brirs, optimize="optimal")

    return factors.compute()


def calc_factor_matrix_manual(brirs, decorrelation_filters, delays, dec_delay, fs):
    # delays to apply in diffuse path
    round_delays = np.round(delays).astype(int)

    # delays to apply in direct path
    max_delay = np.max(np.ceil(delays).astype(int)) + 2
    delay_samples = np.arange(max_delay)

    # (view, loudspeaker, ear, sample)
    brirs_decorrelated = sig.oaconvolve(
        brirs, decorrelation_filters[np.newaxis, :, np.newaxis], axes=-1
    )

    ir_len = max(
        brirs_decorrelated.shape[-1] + np.max(round_delays),
        brirs.shape[-1] + dec_delay + np.max(delay_samples),
    )

    brirs_decorrelated_delayed = apply_delays(
        brirs_decorrelated, np.round(delays).astype(int), out_len=ir_len
    )

    n_views, n_speakers, n_ears = brirs.shape[:-1]

    factors = np.zeros(
        (max_delay, n_views, n_speakers * 2, n_speakers * 2, n_ears), dtype=np.float32
    )
    for delay in range(max_delay):
        for view_i in range(brirs.shape[0]):
            # (loudspeaker, ear, sample)
            brirs_delayed = apply_delays(
                brirs[view_i],  # speaker, ear, sample
                np.array(delay + dec_delay)[np.newaxis, np.newaxis],
                out_len=ir_len,
            )

            # (loudspeaker*2, ear, sample)
            all_brirs = np.concatenate(
                (brirs_delayed, brirs_decorrelated_delayed[view_i]),
                axis=0,
            )

            # (loudspeaker*2, loudspeaker*2, ear)
            factors_i = np.einsum("les,mes->lme", all_brirs, all_brirs)

            factors[delay, view_i] = factors_i

    return factors


def design_decorrelators(layout):
    size = 128

    # original file has a (harmless) bug where the IDs are inverted compared to
    # how the EAR generates them; replicate that
    dec_ids = np.argsort(layout.channel_names)
    decorrelators = np.array(
        [
            ear.core.objectbased.decorrelate.design_decorrelator_basic(
                dec_id, size=size
            )
            for dec_id in dec_ids
        ]
    )

    delay = (size - 1) // 2

    # normalise decorrelators
    decorrelators /= np.sqrt(np.sum(decorrelators**2, axis=1, keepdims=True))

    return decorrelators, delay


def get_hoa_delay(brirs, delays, hoa_irs, decorrelation_delay, front_loudspeaker):
    import scipy.signal as sig

    brir = brirs[0, front_loudspeaker, 0]
    hoa_ir = hoa_irs[0, 0]

    osa = 4
    brir_osa = sig.resample(brir, len(brir) * osa)
    hoa_ir_osa = sig.resample(hoa_ir, len(hoa_ir) * osa)

    corr_osa = np.correlate(brir_osa, hoa_ir_osa, mode="full")
    delay_osa = np.argmax(corr_osa) - (len(hoa_ir_osa) - 1)
    delay = delay_osa / osa

    return delay + decorrelation_delay + delays[0, front_loudspeaker, 0]


def make_hoa_dec_symmetrical(decoder):
    """Given a HOA decode matrix (acn, shape (h, 2, n) with h channels and n
    samples), construct a symmetric version from only the left channel
    samples."""
    n, m = ear.core.hoa.from_acn(np.arange(decoder.shape[0]))

    decoder_symmetric = decoder.copy()
    decoder_symmetric[:, 1] = (
        decoder_symmetric[:, 0] * np.where(m >= 0, 1, -1)[:, np.newaxis]
    )

    # check that there's a good positive correlation, meaning we haven't
    # flipped the wrong channels
    correlations = np.sum(decoder * decoder_symmetric, axis=-1) / np.sum(
        decoder**2, axis=-1
    )
    assert np.all(correlations > 0.4)

    return decoder_symmetric


def main(args):
    with open(args.tf_in, "rb") as f:
        input_data = bear.tensorfile.read(f)

        positions = input_data["positions"]
        # shape (view, loudspeaker, ear, sample)
        irs = da.from_array(input_data["brirs"], chunks=(1, -1, -1, -1))
        views = input_data["views"]

        fs = input_data["fs"]

        layout = layout_from_json(input_data["layout"])

        delays = np.load(args.delays, allow_pickle=True)[()]
        if isinstance(delays, dict):
            delays = delays["overall_delays"]
        else:
            # support old format with delays in a plain array
            # shape: (speaker, ear. views)
            delays = np.moveaxis(delays, 2, 0)
            # shape: (view, speaker, ear)

    extra_output = {}

    # oversample and line up
    osa = 4
    delays_int = (delays * osa).round().astype(int)
    irs_osa = resample(irs, irs.shape[-1] * osa, -1)
    delayed = dask_delay(irs_osa, np.max(delays_int) - delays_int, osa=osa)

    # compute onset based on threshold of max absolute value at each sample
    abs_max = da.max(da.absolute(delayed), axis=tuple(range(len(delayed.shape) - 1)))
    thresh = 0.1 * da.max(abs_max)
    # compute the onset here so that the oversampled IRs aren't stored
    print("computing onset")
    onset = da.argmax(abs_max > thresh).compute()
    print("onset: ", onset / osa)

    if args.window == "none":
        # TODO: trim silence from the start of IRs, though this isn't necessary
        # for most HRIR datasets
        irs_win_osa = delayed
    else:
        win_fs = fs * osa
        start, window = make_window(win_fs, args.window, args)
        extra_output["window"] = dict(start=start, samples=window, fs=win_fs)

        win_start = onset + start
        assert win_start >= 0
        irs_win_osa = delayed[..., slice(win_start, win_start + len(window))] * window

    print("computing irs")
    assert irs_win_osa.shape[-1] % osa == 0
    irs = resample(irs_win_osa, irs_win_osa.shape[-1] // osa, -1).compute()
    print("done")

    delays -= np.min(delays)

    # normalise it based on the front loudspeaker
    irs = normalise_front(irs, positions)

    # design decorrelation filters
    decorrelation_filters, decorrelation_delay = design_decorrelators(layout)

    # select front loudspeaker idx
    front_loudspeaker = np.argmin(np.linalg.norm(positions - [0, 1, 0], axis=1))

    output = dict(
        views=views.astype(np.float32),
        # shape (view, loudspeaker, ear, sample)
        brirs=irs.astype(np.float32),
        # shape (view, loudspeaker, ear), in samples
        delays=delays.astype(np.float32),
        fs=fs,
        positions=positions.astype(np.float32),
        layout=layout_to_json(layout),
        decorrelation_filters=decorrelation_filters.astype(np.float32),
        # in samples
        decorrelation_delay=decorrelation_delay,
        front_loudspeaker=int(front_loudspeaker),
    )

    if args.gain_norm:
        print("computing factors")
        if args.use_dask_for_factors:
            factors = calc_factor_matrix(
                irs, decorrelation_filters, delays, decorrelation_delay, fs
            )
        else:
            factors = calc_factor_matrix_manual(
                irs, decorrelation_filters, delays, decorrelation_delay, fs
            )
        print("done")
        output["gain_norm_quick"] = dict(factors=factors.astype(np.float32))

    if args.hoa_decoder is not None:
        hoa = output["hoa"] = {}
        f = load_hdf5(args.hoa_decoder)
        decoder = np.array(f["Data.IR"]).astype(np.float32)
        hoa["irs"] = make_hoa_dec_symmetrical(decoder)
        hoa["delay"] = float(
            get_hoa_delay(
                output["brirs"],
                output["delays"],
                hoa["irs"],
                output["decorrelation_delay"],
                output["front_loudspeaker"],
            )
        )

    with open(args.tf_out, "wb") as f:
        bear.tensorfile.write(f, output)

    if args.extra_out is not None:
        with open(args.extra_out, "wb") as f:
            bear.tensorfile.write(f, extra_output)


if __name__ == "__main__":
    # dask.config.set(scheduler='synchronous')
    from dask.diagnostics import ProgressBar

    ProgressBar().register()
    main(parse_args())
