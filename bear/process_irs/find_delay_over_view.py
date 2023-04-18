import bear.irs
import bear.tensorfile
from scipy import signal
from ear.core.geom import azimuth, elevation
import dask
from dask import delayed
import numpy as np


def parse_args():
    import argparse

    parser = argparse.ArgumentParser()

    parser.add_argument("--tf-in", help="input TF data file", required=True)
    parser.add_argument(
        "--out", help="output .npy file containing delays", required=True
    )

    parser.add_argument(
        "--opt-steps", help="number of optimisation steps", type=int, default=90000
    )
    parser.add_argument(
        "--default-schedule",
        help="use default optimisation schedule",
        action="store_true",
    )

    return parser.parse_args()


def main(args):
    with open(args.tf_in, "rb") as f:
        input_data = bear.tensorfile.read(f)

        positions = input_data["positions"]
        # shape (view, loudspeaker, ear, sample)
        brirs = input_data["brirs"]
        views = input_data["views"]

        osa = 4
        all_results = {}

        max_samples = np.argmax(brirs, axis=-1)
        win_start = max(0, np.min(max_samples) - 100)
        win_end = np.max(max_samples) + 100
        brirs = brirs[..., win_start:win_end]

        if args.default_schedule:
            sched = {
                "tmax": 5.4,
                "tmin": 0.0017,
                "steps": args.opt_steps,
                "updates": 100,
            }
        else:
            # determine schedule using front loudspeaker
            # this is a little inefficient (calculates CC matrices for this
            # speaker twice), but allows more parallelism compared to running the whole thing
            front_loudspeaker = np.argmin(np.linalg.norm(positions - [0, 1, 0], axis=1))

            irs = brirs[:, front_loudspeaker, 0]
            irs_osa = signal.resample(irs, irs.shape[-1] * osa, axis=-1)
            acc = bear.irs.align_ccs_smooth(
                irs_osa,
                azimuth(views),
                osa,
                estimate_delays=bear.irs.est_delays_integral,
                return_annealer=True,
            )
            sched = acc.auto(0.5)
            sched["steps"] = args.opt_steps

        for speaker_idx in range(len(positions)):
            for ear_idx in 0, 1:
                irs = brirs[:, speaker_idx, ear_idx]
                irs_osa = delayed(signal.resample)(irs, irs.shape[-1] * osa, axis=-1)
                delays, extra = delayed(bear.irs.align_ccs_smooth, nout=2)(
                    irs_osa,
                    azimuth(views),
                    osa,
                    estimate_delays=bear.irs.est_delays_integral,
                    sched=sched,
                    smooth=elevation(positions[speaker_idx]) > 10,
                )
                all_results[(speaker_idx, ear_idx)] = (delays, extra)

        all_results_comp = delayed(all_results).compute()

        np.save(args.out, dict(view_align_results=all_results_comp))


if __name__ == "__main__":
    dask.config.set(scheduler="multiprocessing")
    from dask.diagnostics import ProgressBar

    ProgressBar().register()
    main(parse_args())
