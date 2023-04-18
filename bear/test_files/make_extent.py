from ear.fileio.adm.builder import ADMBuilder
from ear.fileio.adm.elements import AudioBlockFormatObjects
from ear.core.geom import PolarPosition
from fractions import Fraction
from .utils import write_adm
import numpy as np


def interp_frac(x, xp, fp):
    """same as np.interp, but works with Fraction"""
    if x <= xp[0]:
        return fp[0]
    if x >= xp[-1]:
        return fp[-1]
    for i in range(len(xp) - 1):
        j = i + 1

        if xp[i] <= x < xp[j]:
            p = (x - xp[i]) / (xp[j] - xp[i])
            return p * (fp[j] - fp[i]) + fp[i]


def scale_angle(angle):
    """Scale an extent angle so that linear input results in linear extent area."""
    return np.degrees(2 * np.arccos(np.clip(1 - 2 * (angle / 360), -1, 1)))


def make_extent_ramp_adm(azimuth, elevation, diffuse, width, height):
    ramp_t_frac = [Fraction(0)]
    ramp_size = [0.0]
    ramp_time = Fraction(4)
    for i in range(5):
        ramp_t_frac.append(ramp_t_frac[-1] + ramp_time / 2**i)
        ramp_t_frac.append(ramp_t_frac[-1] + ramp_time / 2**i)
        ramp_size.extend([360, 0])

    start_t, end_t = min(ramp_t_frac), max(ramp_t_frac)

    blocks_per_sec = 50

    sample_t_frac = [
        Fraction(i, blocks_per_sec)
        for i in range(int((end_t - start_t) * blocks_per_sec + 1))
    ]

    def bf(i):
        start_time = sample_t_frac[i - 1] if i > 0 else sample_t_frac[i]
        end_time = sample_t_frac[i]

        size = interp_frac(sample_t_frac[i], ramp_t_frac, ramp_size)

        return AudioBlockFormatObjects(
            rtime=start_time,
            duration=end_time - start_time,
            position=PolarPosition(azimuth, elevation, 1.0),
            width=size if width else 0.0,
            height=size if height else 0.0,
            diffuse=diffuse,
        )

    block_formats = [bf(i) for i in range(len(sample_t_frac))]

    builder = ADMBuilder()
    builder.load_common_definitions()
    programme = builder.create_programme(audioProgrammeName="MyProgramme")
    content = builder.create_content(audioContentName="MyContent", parent=programme)

    builder.create_item_objects(
        0, "MyObject 0", parent=content, block_formats=block_formats
    )

    return builder.adm


def parse_args():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("in_fname")
    parser.add_argument("out_fname")
    parser.add_argument("--azimuth", type=float, default=0.0)
    parser.add_argument("--elevation", type=float, default=0.0)
    parser.add_argument(
        "--width", action="store_true", help="enable varying width; zero otherwise"
    )
    parser.add_argument(
        "--height", action="store_true", help="enable varying height; zero otherwise"
    )
    parser.add_argument("--diffuse", type=float, default=1.0)

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    write_adm(
        make_extent_ramp_adm(
            azimuth=args.azimuth,
            elevation=args.elevation,
            diffuse=args.diffuse,
            width=args.width,
            height=args.height,
        ),
        args.in_fname,
        args.out_fname,
    )
