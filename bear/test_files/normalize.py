import soundfile
import numpy as np


def get_loudness_pyln(fname):
    import pyloudnorm

    d, sr = soundfile.read(fname)

    meter = pyloudnorm.Meter(sr)
    return meter.integrated_loudness(d)


measurement_methods = dict(
    pyloudnorm=get_loudness_pyln,
)

default_measurement_method = "pyloudnorm"


def get_max_abs(fname):
    d, sr = soundfile.read(fname)
    return np.max(np.abs(d))


def to_db(x):
    return 20 * np.log10(x)


def db_to_abs(x):
    return 10 ** (x / 20.0)


def parse_args():
    import argparse

    parser = argparse.ArgumentParser(
        description="loudless normalise multiple files",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--method",
        choices=tuple(measurement_methods.keys()),
        default=default_measurement_method,
        help="loudness measurement method",
    )

    parser.add_argument(
        "--measurements-file",
        help="write measurements as newline-delimited JSON to FILE",
        metavar="FILE",
        type=argparse.FileType("w"),
    )

    parser.add_argument(
        "files",
        nargs="+",
        metavar="file",
        help="input files then output files, separated by -",
    )

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    inputs_f = args.files[: args.files.index("-")]
    outputs_f = args.files[args.files.index("-") + 1 :]
    assert len(inputs_f) == len(outputs_f)

    get_loudness = measurement_methods[args.method]

    loudnesses = np.array([get_loudness(f) for f in inputs_f])

    print(
        "input loudnesses: {}".format(
            ",".join(f"{loudness:3g}" for loudness in loudnesses)
        )
    )
    if args.measurements_file is not None:
        with args.measurements_file as f:
            import json

            for fname, loudness in zip(inputs_f, loudnesses):
                json.dump(dict(fname=fname, loudness_db=loudness), f)
                f.write("\n")

    max_abs_vols = np.array([get_max_abs(f) for f in inputs_f])
    headrooms = -to_db(max_abs_vols)

    target_loudness = -23
    gains_for_target = target_loudness - loudnesses
    gains = gains_for_target

    min_headroom_after = np.min(headrooms - gains_for_target)

    min_allowed_headroom = 1
    amount_over_limit = min_allowed_headroom - min_headroom_after
    if amount_over_limit > 0:
        gains -= amount_over_limit
        out_loudness = target_loudness - amount_over_limit
        print(
            "not enough headroom to normalise correctly; reducing overall "
            f"loudness by {amount_over_limit:3g} dB to {out_loudness:3g}"
        )

    for input_f, output_f, gain in zip(inputs_f, outputs_f, gains):
        d, sr = soundfile.read(input_f)
        d *= db_to_abs(gain)
        soundfile.write(output_f, d, sr, subtype="PCM_24")
