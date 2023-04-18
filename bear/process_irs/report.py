import bear.tensorfile
import matplotlib.pyplot as plt
import numpy as np
from bear.irs import apply_delays
from bear.layout import layout_from_json
from contextlib import contextmanager
from ear.core.geom import azimuth, elevation, relative_angle, cart
from matplotlib.backends.backend_pdf import PdfPages
from scipy import signal


def parse_args():
    import argparse

    parser = argparse.ArgumentParser()

    parser.add_argument("--tf-in", help="input TF data file", required=True)
    parser.add_argument(
        "--in-delays",
        help="input .npy file containing delays for IR sets with multiple views",
    )

    parser.add_argument(
        "--in-merged-delays",
        help="input .npy file containing merged delays",
        required=True,
    )

    parser.add_argument(
        "--out",
        help="output PDF file",
        required=True,
    )

    parser.add_argument("--extra-in", help="extra input from finalise")

    return parser.parse_args()


@contextmanager
def pdf_page(pdf, figsize=(10, 8), *subplots_args, **subplots_kwargs):
    fig, ax = plt.subplots(*subplots_args, figsize=figsize, **subplots_kwargs)
    yield fig, ax
    pdf.savefig(fig)
    plt.close(fig)


def spherical_head_model(r, ear_vec, sound_vec):
    """basic spherical head model, with no interaction when the ear is pointing
    towards the source, and diffraction when it is pointing away

    Parameters:
        r: head radius in meters
        ear_vec: direction of the ear
        sound_vec: direction from which the sound arrives
    Returns:
        time of arrival at the ear in seconds, relative to the center of the
        head
    """
    ear_vec = ear_vec / np.linalg.norm(ear_vec)
    sound_vec = sound_vec / np.linalg.norm(sound_vec)

    dot_prod = np.dot(ear_vec, sound_vec)

    if dot_prod <= 0:
        angle = np.arccos(np.clip(dot_prod, -1, 1))
        distance = r * (angle - np.pi / 2)
    else:
        distance = -r * dot_prod

    mach = 331.46  # m/s
    return distance / mach


@np.vectorize
def spherical_head_model_itd(azimuth, elevation, head_r=0.089, ear_az=92.0, ear_el=2.0):
    """estimate ITD based on spherical_head_model

    default parameters are found by least-squares minimisation on the delay
    results for bbcrdlr_all_speakers.sofa

    Parameters:
        azimuth (float): adm-format azimuth of source in degrees
        elevation (float): adm-format elevation of source in degrees

    Returns:
        ITD (right delay minus left) in seconds
    """
    left_ear = cart(ear_az, ear_el, 1)
    right_ear = cart(-ear_az, ear_el, 1)

    vec = cart(azimuth, elevation, 1)

    return spherical_head_model(head_r, right_ear, vec) - spherical_head_model(
        head_r, left_ear, vec
    )


def main(args):
    # no need for an interactive backend
    import matplotlib

    matplotlib.use("Agg")

    with PdfPages(args.out) as pdf:

        with open(args.tf_in, "rb") as f:
            input_data = bear.tensorfile.read(f)

            positions = input_data["positions"]
            # shape (view, loudspeaker, ear, sample)
            all_brirs = input_data["brirs"]
            views = input_data["views"]

            layout = layout_from_json(input_data["layout"])
            fs = input_data["fs"]
            front_loudspeaker = input_data["front_loudspeaker"]

        if args.extra_in is not None:
            with open(args.extra_in, "rb") as f:
                extra_input_data = bear.tensorfile.read(f)
        else:
            extra_input_data = None

        view_azimuths = azimuth(views)
        view_order = np.argsort(view_azimuths)
        spk_names = layout.channel_names
        ear_names = "left", "right"

        merged_delays_dict = np.load(args.in_merged_delays, allow_pickle=True)[()]

        overall_delays = merged_delays_dict["overall_delays"]

        def plot_anneal_log(ax, log):
            steps = [entry["step"] for entry in log]
            energys = [entry["E"] for entry in log]
            ax.plot(steps, energys)
            ax.set_xlabel("step")
            ax.set_ylabel("energy")
            ax.axvline(
                steps[np.where(np.diff(energys) < 0)[0][-1]],
                label="last improvement",
                c="r",
            )
            ax.legend()
            ax.grid()

        def plot_speaker_ear_delays(ax, final_delays, extras, title_context):
            ax.plot(
                view_azimuths[view_order],
                10 + extras["est_delays"][view_order],
                label="estimate",
            )
            ax.plot(
                view_azimuths[view_order],
                10 + extras["smooth_est_delays"][view_order],
                label="smoothed estimate",
            )
            ax.plot(
                view_azimuths[view_order],
                extras["anneal_delays"][view_order],
                label="anneal",
            )
            ax.plot(view_azimuths[view_order], final_delays[view_order], label="final")
            ax.set_title(f"delays for {title_context}")
            ax.legend()
            ax.grid()

        if args.in_delays is not None:
            rot_delays_dict = np.load(args.in_delays, allow_pickle=True)[()][
                "view_align_results"
            ]

            for (spk_idx, ear_idx), (final_delays, extras) in rot_delays_dict.items():
                title_context = f"{spk_names[spk_idx]}, {ear_names[ear_idx]} ear"
                with pdf_page(pdf) as (fig, ax):
                    plot_speaker_ear_delays(ax, final_delays, extras, title_context)

                with pdf_page(pdf) as (fig, ax):
                    plot_anneal_log(ax, extras["log"])
                    ax.set_title(f"simulated annealing log for {title_context}")

        with pdf_page(pdf) as (fig, ax):
            plot_anneal_log(ax, merged_delays_dict["log"])
            ax.set_title("simulated annealing log when merging speaker+ear delays")

        def plot_ITDs_subset(ax, itds, angles, subset):
            for i, (angles_i, itds_i) in enumerate(
                zip(angles, np.moveaxis(itds, 1, 0))
            ):
                if not subset[i]:
                    continue

                if len(angles_i) > 1:
                    order = np.argsort(angles_i)
                    angles_ic = np.concatenate((angles_i[order] - 360, angles_i[order]))
                    itds_ic = np.concatenate((itds_i[order], itds_i[order]))

                    ax.plot(angles_ic, itds_ic, alpha=0.7, label=spk_names[i])
                else:
                    ax.scatter(
                        ((angles_i[0] + 180) % 360) - 180, itds_i[0], label=spk_names[i]
                    )

            el = np.mean(elevation(positions[subset]))
            az = np.arange(-180, 181)
            est_itds = spherical_head_model_itd(az, el)
            plt.plot(az, est_itds * fs, label="spherical model")

            ax.legend()
            ax.set_xlim(-195, 195)
            ax.set_xlabel("relative azimuth between view and speaker [deg]")
            ax.set_xticks(np.arange(-180, 181, 30))
            ax.set_ylabel("itd [samples at 48kHz]")
            ax.grid()

        def get_layers():
            for label_start, description in [
                ("M", "mid-layer"),
                ("U", "upper layer"),
                ("T", "top layer"),
                ("B", "bottom layer"),
            ]:
                subset = [n.startswith(label_start) for n in spk_names]
                if any(subset):
                    yield description, np.array(subset)

        def plot_ITDs():
            itds = overall_delays[:, :, 1] - overall_delays[:, :, 0]
            angles = np.vectorize(relative_angle)(
                0, azimuth(positions)[:, np.newaxis] - azimuth(views)[np.newaxis, :]
            )
            for description, subset in get_layers():
                with pdf_page(pdf) as (fig, ax):
                    plot_ITDs_subset(ax, itds, angles, subset)
                    ax.set_title(
                        f"ITDs for all head orientations for {description} loudspeakers"
                    )

        plot_ITDs()

        def plot_cross_correlations(ax):
            for ear_idx in 0, 1:
                cross_correlations = []
                for view_idx in range(len(views)):
                    osa = 4
                    irs = all_brirs[view_idx, :, ear_idx]
                    delays = overall_delays[view_idx, :, ear_idx]
                    irs_osa = signal.resample(irs, irs.shape[-1] * osa, axis=-1)

                    delays_to_apply = np.round(osa * (np.max(delays) - delays)).astype(
                        int
                    )
                    delayed = apply_delays(irs_osa, delays_to_apply, osa=osa)

                    all_pairs_correlation = np.sum(
                        delayed[np.newaxis] * delayed[:, np.newaxis]
                    )
                    cross_correlations.append(all_pairs_correlation)

                ax.plot(
                    view_azimuths[view_order],
                    np.array(cross_correlations)[view_order],
                    label=ear_names[ear_idx],
                )

            ax.set_title("all-pairs cross-correlation vs. view angle")
            ax.set_xlabel("view angle [deg]")
            ax.set_ylabel(
                "un-normalised all-pairs correlation within view for calculated delays"
            )

            ax.legend()
            ax.grid()

        if len(views) > 1:
            with pdf_page(pdf) as (fig, ax):
                plot_cross_correlations(ax)

        peak = np.argmax(np.max(np.abs(all_brirs[0]), axis=(0, 1)))
        ir_plot_len = min(all_brirs.shape[-1], int(peak * 2.5))

        def plot_irs():
            osa = 4
            t = np.arange(ir_plot_len * osa) / (fs * osa)
            scale = 30
            for description, subset in get_layers():
                with pdf_page(pdf) as (fig, ax):
                    angles = azimuth(positions[subset])

                    irs = all_brirs[0, subset, 0, :ir_plot_len]
                    irs_osa = signal.resample(irs, irs.shape[-1] * osa, axis=-1)
                    ax.plot(t, irs_osa.T * scale + angles, alpha=0.75)

                    irs = all_brirs[0, subset, 1, :ir_plot_len]
                    irs_osa = signal.resample(irs, irs.shape[-1] * osa, axis=-1)
                    ax.plot(t, irs_osa.T * scale - angles, alpha=0.75)

                    ax.grid()
                    ax.set_ylabel("azimuth[deg] + amplitude")
                    ax.set_xlabel("time [s]")
                    ax.set_title(f"IRs for {description} loudspeakers")

        plot_irs()

        def plot_max_irs_db():
            with pdf_page(pdf) as (fig, ax):
                max_abs = np.max(np.abs(all_brirs[..., :ir_plot_len]), axis=(0, 1, 2))
                plt.plot(np.arange(len(max_abs)) / fs, np.log10(max_abs) * 20)

                if extra_input_data is not None and "window" in extra_input_data:
                    win = extra_input_data["window"]["samples"]
                    win_fs = extra_input_data["window"]["fs"]

                    win = win[: int((ir_plot_len / fs) * win_fs)]
                    plt.plot(
                        np.arange(len(win))[win > 0] / win_fs,
                        np.log10(win[win > 0]) * 20,
                        label="window",
                    )

                ax.grid()
                ax.legend()
                ax.set_xlabel("time [s]")
                ax.set_ylabel("amplitude [dBFS]")
                ax.set_title(
                    "maximum amplitude of the start of all irs and the applied window"
                )

        plot_max_irs_db()

        def plot_one_ir_db():
            with pdf_page(pdf) as (fig, ax):
                ir = all_brirs[0, front_loudspeaker, 0]
                plt.plot(np.arange(len(ir)) / fs, np.log10(np.abs(ir)) * 20)

                if extra_input_data is not None and "window" in extra_input_data:
                    win = extra_input_data["window"]["samples"]
                    win_fs = extra_input_data["window"]["fs"]

                    plt.plot(
                        np.arange(len(win))[win > 0] / win_fs,
                        np.log10(win[win > 0]) * 20,
                        label="window",
                    )

                ax.grid()
                ax.legend()
                ax.set_xlabel("time [s]")
                ax.set_ylabel("amplitude [dBFS]")
                ax.set_title(
                    "amplitude of the front IR for the left ear and the applied window"
                )

        plot_one_ir_db()

        def plot_it_positions():
            with pdf_page(pdf) as (fig, ax):
                s = 10
                layout_az = [c.polar_nominal_position.azimuth for c in layout.channels]
                layout_el = [
                    c.polar_nominal_position.elevation for c in layout.channels
                ]
                ax.scatter(layout_az, layout_el, label="nominal positions", s=s * 3)

                ax.scatter(
                    -azimuth(positions),
                    elevation(positions),
                    label="flipped IR positions",
                    s=s * 2,
                )

                ax.scatter(
                    azimuth(positions),
                    elevation(positions),
                    label="IR positions",
                    s=s * 1,
                )

                # only label the first to avoid duplicate labels
                ax.plot(
                    [layout_az, azimuth(positions)],
                    [layout_el, elevation(positions)],
                    label=[
                        "nominal positions to IR positions" if i == 0 else ""
                        for i in range(len(positions))
                    ],
                    c="r",
                )

                ax.set_xlim(-185, 185)
                ax.set_ylim(-95, 95)
                ax.grid()
                ax.grid(which="minor", alpha=0.3)

                ax.set_xticks(np.arange(-180, 181, 20))
                ax.set_xticks(np.arange(-180, 181, 5), minor=True)

                ax.set_yticks(np.arange(-80, 91, 20))
                ax.set_yticks(np.arange(-90, 91, 5), minor=True)

                ax.legend()
                ax.set_xlabel("azimuth [deg]")
                ax.set_ylabel("elevation [deg]")
                ax.set_title("IR positions")

        plot_it_positions()


if __name__ == "__main__":
    main(parse_args())
