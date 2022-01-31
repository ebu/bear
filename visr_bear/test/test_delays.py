import visr_bear
import numpy as np
import numpy.testing as npt
from pathlib import Path
import scipy.signal as sig
from utils import data_path


def do_render(renderer, period, objects=None, direct_speakers=None, hoa=None):
    not_none = [x for x in [objects, direct_speakers, hoa] if x is not None][0]
    length = not_none.shape[1]
    dummy_samples = np.zeros((0, length), dtype=np.float32)
    output = np.zeros((2, length), dtype=np.float32)

    def convert(samples):
        if samples is None:
            return dummy_samples
        return samples.astype(np.float32, order="C", copy=False)

    objects = convert(objects)
    direct_speakers = convert(direct_speakers)
    hoa = convert(hoa)

    for i in range(length // period):
        s = np.s_[:, i * period : (i + 1) * period]
        renderer.process(objects[s], direct_speakers[s], hoa[s], output[s])
    return output


def correlate(a, b):
    """returns (delay, correlation), where correlation
    is the full cross-correlation, and delay is a vector of
    delays corresponding to the delay from a to b for each
    sample in correlation."""
    correlation = np.correlate(b, a, mode="full")
    delay = np.arange(len(correlation)) - (len(a) - 1)
    return delay, correlation


period = 512


def render_directspeakers_front(data_file, samples):
    config = visr_bear.api.Config()
    config.num_objects_channels = 0
    config.num_direct_speakers_channels = 1
    config.period_size = period
    config.data_path = data_file
    renderer = visr_bear.api.Renderer(config)

    dsi = visr_bear.api.DirectSpeakersInput()
    dsi.rtime = visr_bear.api.Time(0, 1)
    dsi.duration = visr_bear.api.Time(1, 1)
    renderer.add_direct_speakers_block(0, dsi)
    return do_render(renderer, period, direct_speakers=samples)


def render_objects_front(data_file, samples):
    config = visr_bear.api.Config()
    config.num_objects_channels = 1
    config.num_direct_speakers_channels = 0
    config.period_size = period
    config.data_path = data_file
    renderer = visr_bear.api.Renderer(config)

    oi = visr_bear.api.ObjectsInput()
    oi.rtime = visr_bear.api.Time(0, 1)
    oi.duration = visr_bear.api.Time(1, 1)
    oi.type_metadata.position = visr_bear.api.PolarPosition(0, 0, 1)
    renderer.add_objects_block(0, oi)

    return do_render(renderer, period, objects=samples)


def render_diffuse_front(data_file, samples):
    config = visr_bear.api.Config()
    config.num_objects_channels = 1
    config.num_direct_speakers_channels = 0
    config.period_size = period
    config.data_path = data_file
    renderer = visr_bear.api.Renderer(config)

    oi = visr_bear.api.ObjectsInput()
    oi.rtime = visr_bear.api.Time(0, 1)
    oi.duration = visr_bear.api.Time(1, 1)
    oi.type_metadata.position = visr_bear.api.PolarPosition(0, 0, 1)
    oi.type_metadata.diffuse = 1.0
    renderer.add_objects_block(0, oi)

    return do_render(renderer, period, objects=samples)


def render_hoa_omni(data_file, samples):
    config = visr_bear.api.Config()
    config.num_objects_channels = 0
    config.num_direct_speakers_channels = 0
    config.num_hoa_channels = 1
    config.period_size = period
    config.data_path = data_file
    renderer = visr_bear.api.Renderer(config)

    hi = visr_bear.api.HOAInput()
    hi.rtime = visr_bear.api.Time(0, 1)
    hi.duration = visr_bear.api.Time(1, 1)
    hi.channels = [0]
    hi.type_metadata.orders = [0]
    hi.type_metadata.degrees = [0]
    hi.type_metadata.normalization = "SN3D"
    renderer.add_hoa_block(0, hi)

    return do_render(renderer, period, hoa=samples)


def test_objects_direct_speakers_delays():
    """check that delays between direct/diffuse/directspeakers paths match.
    These share the same IRs so can be tested exactly."""
    files_dir = Path(__file__).parent / "files"
    data_file = str(files_dir / "unity_brirs_decorrelators.tf")

    input_samples = np.random.normal(size=(1, 48000)).astype(np.float32)
    direct_speakers_samples = render_directspeakers_front(data_file, input_samples)
    objects_samples = render_objects_front(data_file, input_samples)
    diffuse_samples = render_diffuse_front(data_file, input_samples)

    # skip 2 periods, because the gains settle during the first period, and
    # some of this will still be coming through the delays in the second period
    npt.assert_allclose(
        direct_speakers_samples[:, 2 * period :],
        objects_samples[:, 2 * period :],
        atol=2e-4,
    )
    npt.assert_allclose(
        direct_speakers_samples[:, 2 * period :],
        diffuse_samples[:, 2 * period :],
        atol=2e-4,
    )


def test_objects_hoa_delays():
    """check that delays between objects and HOA paths match. These use
    different IRs, so check with cross-correlation."""
    input_samples = np.zeros(shape=(1, 10240)).astype(np.float32)
    input_samples[:, 4800] = 1.0
    objects_samples = render_objects_front(data_path, input_samples)
    hoa_samples = render_hoa_omni(data_path, input_samples)

    def check_delay(a, b):
        osa = 4
        a_osa = sig.resample(a, len(a) * osa)
        b_osa = sig.resample(b, len(b) * osa)

        delay, correlation = correlate(a_osa, b_osa)
        # check that 0 delay is a peak comparable with the delay that has the
        # highest correlation
        assert correlation[np.where(delay == 0)[0][0]] > 0.50 * np.max(correlation)

    skip = period * 2 + 3000

    check_delay(objects_samples[0, skip:], hoa_samples[0, skip:])
    check_delay(objects_samples[1, skip:], hoa_samples[1, skip:])
