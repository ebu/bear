import bear.tensorfile
import numpy as np
from scipy import signal
import bear.irs


def parse_args():
    import argparse

    parser = argparse.ArgumentParser()

    parser.add_argument("--tf-in", help="input TF data file", required=True)
    parser.add_argument(
        "--in-delays",
        help="input .npy file containing delays for IR sets with multiple views",
    )
    parser.add_argument(
        "--out", help="output .npy file containing merged delays", required=True
    )

    parser.add_argument(
        "--opt-steps", help="number of optimisation steps", type=int, default=840000
    )
    parser.add_argument(
        "--default-schedule",
        help="use default optimisation schedule",
        action="store_true",
    )

    return parser.parse_args()


def apply_delays_to_cc(cc_mat, delays):
    """apply delays to a cross-correlation matrix; this is roughly equivalent
    to applying delays to IRs before calculating the CC matrix."""
    result = np.zeros_like(cc_mat)
    for i, j in np.ndindex(cc_mat.shape[:2]):
        delay = (delays[j] - delays[i]).round().astype(int)
        result[
            i, j, max(0, delay) : min(cc_mat.shape[-1], cc_mat.shape[-1] + delay)
        ] = cc_mat[
            i, j, max(0, -delay) : min(cc_mat.shape[-1], cc_mat.shape[-1] + -delay)
        ]
    return result


def main(args):
    with open(args.tf_in, "rb") as f:
        input_data = bear.tensorfile.read(f)

        positions = input_data["positions"]
        # shape (view, loudspeaker, ear, sample)
        all_brirs = input_data["brirs"]
        views = input_data["views"]

        osa = 4

        if len(views) > 1:
            if args.in_delays is None:
                raise RuntimeError("must specify --in-delays to process multiple views")

            rot_delays_dict = np.load(args.in_delays, allow_pickle=True)[()][
                "view_align_results"
            ]
            # shape: (speaker, ear, view)
            rot_delays = np.array(
                [
                    [rot_delays_dict[(i, j)][0] for j in range(2)]
                    for i in range(len(positions))
                ]
            )

            view_samples = np.linspace(0, len(views), 6, endpoint=False).astype(int)

            # compute cc matrix for each sample
            ccs_for_view = []
            for sample in view_samples:
                irs = all_brirs[sample]  # (loudspeaker, ear, sample)
                irs = irs.reshape((-1, irs.shape[-1]))
                irs_osa = signal.resample(irs, irs.shape[-1] * osa, axis=-1)
                cc_mat = bear.irs.get_cc_mat(irs_osa[..., :5000], 100 * osa)
                cc_mat_norm = bear.irs.norm_cc_mat(cc_mat)
                ccs_for_view.append(cc_mat_norm)
            # sample, ir idx, ir idx, delay
            ccs_for_view = np.array(ccs_for_view)

            # cc matrix for each view, with delay applied relative to the front
            ccs_delayed = np.array(
                [
                    apply_delays_to_cc(
                        cc_mat,
                        -(
                            rot_delays[..., view_samples[i]]
                            - rot_delays[..., view_samples[0]]
                        ).flatten(),
                    )
                    for i, cc_mat in enumerate(ccs_for_view)
                ]
            )

            ccs = np.mean(ccs_delayed, axis=0)
        else:
            irs = all_brirs[0]  # (loudspeaker, ear, sample)
            irs = irs.reshape((-1, irs.shape[-1]))
            irs_osa = signal.resample(irs, irs.shape[-1] * osa, axis=-1)
            cc_unnorm = bear.irs.get_cc_mat(irs_osa[..., :5000], 100 * osa)
            ccs = bear.irs.norm_cc_mat(cc_unnorm)

        if args.default_schedule:
            sched = {"tmax": 50.0, "tmin": 0.00058, "updates": 100}
        else:
            acc = bear.irs.AlignCCs(ccs)
            sched = acc.auto(0.5)

        sched["steps"] = args.opt_steps

        acc = bear.irs.AlignCCs(ccs, log=True)
        acc.set_schedule(sched)
        acc.anneal()

        delays = acc.best_state
        # shape: (speaker, ear)
        global_delays = (delays.reshape(-1, 2) - np.min(delays)) / osa

        if len(views) > 1:
            # shape: (speaker, ear, view)
            overall_delays = (
                global_delays[..., np.newaxis]
                + (rot_delays - rot_delays[..., [0]]) / osa
            )
            # shape: (view, speaker, ear)
            overall_delays = np.moveaxis(overall_delays, 2, 0)
            # TODO: rearrange rot_delays to avoid this
        else:
            overall_delays = global_delays[np.newaxis]

        np.save(args.out, dict(overall_delays=overall_delays, log=acc.log))


if __name__ == "__main__":
    main(parse_args())
