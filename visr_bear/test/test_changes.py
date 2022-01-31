from attr import attrs, attrib, Factory
import visr_bear.api
import numpy as np
import numpy.testing as npt
from ear.core import hoa
from pathlib import Path
import pytest
from functools import partial
from utils import data_path

import warnings

with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import quaternion


@attrs
class RendererInput:
    """Some corresponding input metadata and audio"""

    objects_blocks = attrib(default=Factory(list))
    direct_speakers_blocks = attrib(default=Factory(list))
    hoa_blocks = attrib(default=Factory(list))

    objects_audio = attrib(default=Factory(lambda: np.zeros((0, 0))))
    direct_speakers_audio = attrib(default=Factory(lambda: np.zeros((0, 0))))
    hoa_audio = attrib(default=Factory(lambda: np.zeros((0, 0))))

    def num_samples(self):
        return max(
            x.shape[1]
            for x in [self.objects_audio, self.direct_speakers_audio, self.hoa_audio]
            if x is not None
        )


def concat_inputs(inputs):
    """combine multiple RendererInputs"""
    length = max(input.num_samples() for input in inputs)

    def concat_samples(samples):
        new_samples = np.zeros(
            (sum(s.shape[0] for s in samples), length), dtype=np.float32
        )
        i = 0
        for s in samples:
            new_samples[i : i + s.shape[0], : s.shape[1]] = s
            i += s.shape[0]
        return new_samples

    def update_hoa_block_channels(block, offset):
        block = visr_bear.api.HOAInput(block)
        block.channels = list(range(offset, offset + len(block.type_metadata.orders)))
        return block

    def concat_hoa_blocks(input_blocks):
        out = []

        i = 0
        for blocks in input_blocks:
            for block_channel in blocks:
                out.append(
                    [update_hoa_block_channels(block, i) for block in block_channel]
                )
                i += len(block_channel[0].type_metadata.orders)

        return out

    return RendererInput(
        objects_blocks=sum((i.objects_blocks for i in inputs), []),
        direct_speakers_blocks=sum((i.direct_speakers_blocks for i in inputs), []),
        hoa_blocks=concat_hoa_blocks(i.hoa_blocks for i in inputs),
        objects_audio=concat_samples([i.objects_audio for i in inputs]),
        direct_speakers_audio=concat_samples([i.direct_speakers_audio for i in inputs]),
        hoa_audio=concat_samples([i.hoa_audio for i in inputs]),
    )


class MetadataFeeder:
    """takes metadata from a RendererInput and calls renderer.add_*_block"""

    def __init__(self, input: RendererInput):
        self.objects_blocks = [blocks[:] for blocks in input.objects_blocks]
        self.direct_speakers_blocks = [
            blocks[:] for blocks in input.direct_speakers_blocks
        ]
        self.hoa_blocks = [blocks[:] for blocks in input.hoa_blocks]

    def feed(self, renderer):
        for i, objects_blocks in enumerate(self.objects_blocks):
            while objects_blocks:
                if not renderer.add_objects_block(i, objects_blocks[0]):
                    break
                objects_blocks.pop(0)

        for i, direct_speakers_blocks in enumerate(self.direct_speakers_blocks):
            while direct_speakers_blocks:
                if not renderer.add_direct_speakers_block(i, direct_speakers_blocks[0]):
                    break
                direct_speakers_blocks.pop(0)

        for i, hoa_blocks in enumerate(self.hoa_blocks):
            while hoa_blocks:
                if not renderer.add_hoa_block(i, hoa_blocks[0]):
                    break
                hoa_blocks.pop(0)


def null_pre_block_cb(renderer, frame, period):
    pass


def do_render(
    renderer,
    period: int,
    input: RendererInput,
    pre_block_cb=null_pre_block_cb,
):
    """Use a MetadataFeeder to render an input with the given Renderer.
    pre_block_cb will be called before each block is processed with the renderer, frame
    number and period."""
    feeder = MetadataFeeder(input)

    length = input.num_samples()
    output = np.zeros((2, length), dtype=np.float32)

    def convert(samples):
        new_samples = np.zeros((samples.shape[0], length), dtype=np.float32)
        new_samples[:, : samples.shape[1]] = samples
        return new_samples

    objects = convert(input.objects_audio)
    direct_speakers = convert(input.direct_speakers_audio)
    hoa = convert(input.hoa_audio)

    for i in range(length // period):
        pre_block_cb(renderer, i, period)
        feeder.feed(renderer)
        s = np.s_[:, i * period : (i + 1) * period]
        renderer.process(objects[s], direct_speakers[s], hoa[s], output[s])
    return output


def render_input_default(input, period=512, pre_block_cb=null_pre_block_cb):
    """render an input using the default data file"""
    config = visr_bear.api.Config()
    config.num_objects_channels = len(input.objects_audio)
    config.num_direct_speakers_channels = len(input.direct_speakers_audio)
    config.num_hoa_channels = len(input.hoa_audio)
    config.period_size = period
    config.data_path = data_path
    renderer = visr_bear.api.Renderer(config)

    return do_render(renderer, period, input, pre_block_cb=pre_block_cb)


def listener_pre_block_cb(listeners):
    """push listener listeners[i] before frame i"""

    def pre_block_cb(renderer, frame, period):
        if frame < len(listeners):
            renderer.set_listener(listeners[frame], None)

    return pre_block_cb


files_dir = Path(__file__).parent / "files"

input_file = files_dir / "input.npy"


def get_input(roll=0):
    """Get some sample data, consistent across runs. roll is the number of
    samples to shift by, which allows re-use of samples for different tests"""
    if not input_file.exists():
        samples = 0.1 * np.random.normal(size=(1, 48000)).astype(np.float32)
        np.save(input_file, samples)

    return np.roll(np.load(input_file), roll, axis=1)


def get_listeners():
    """Get a list of Listener objects containing dummy data."""
    t = np.arange(100) * (512 / 48000)
    positions = np.stack(
        (0.3 * np.sin(t * 2), 0.3 * np.cos(t * 2), 0.1 * np.sin(t * 4.7)), 1
    )

    rot_lr = t * 7
    q_lr = quaternion.from_rotation_vector(
        np.column_stack(np.broadcast_arrays(0, 0, rot_lr))
    )

    rot_ss = np.radians(15 * np.sin(t * 3.5))
    q_ss = quaternion.from_rotation_vector(
        np.column_stack(np.broadcast_arrays(0, rot_ss, 0))
    )

    rot_ud = np.radians(15 * np.sin(t * 6.5))
    q_ud = quaternion.from_rotation_vector(
        np.column_stack(np.broadcast_arrays(rot_ud, 0, 0))
    )

    orientations = q_lr * q_ss * q_ud

    listeners = []

    for position, orientation in zip(positions, orientations):
        listener = visr_bear.api.Listener()

        listener.orientation_quaternion = orientation.components
        listener.position_cart = position

        listeners.append(listener)

    return listeners


def get_objects_input_simple() -> RendererInput:
    samples = get_input()

    oi = visr_bear.api.ObjectsInput()
    oi.rtime = visr_bear.api.Time(0, 1)
    oi.duration = visr_bear.api.Time(1, 1)
    oi.type_metadata.position = visr_bear.api.PolarPosition(0, 0, 1)

    return RendererInput(objects_blocks=[[oi]], objects_audio=samples)


def get_objects_input_dynamic() -> RendererInput:
    samples = get_input(1000)

    blocks = []
    oi = visr_bear.api.ObjectsInput()
    oi.rtime = visr_bear.api.Time(0, 1)
    oi.duration = visr_bear.api.Time(1, 2)
    oi.type_metadata.position = visr_bear.api.PolarPosition(30, 0, 1)
    blocks.append(oi)

    oi = visr_bear.api.ObjectsInput()
    oi.rtime = visr_bear.api.Time(1, 2)
    oi.duration = visr_bear.api.Time(1, 2)
    oi.type_metadata.position = visr_bear.api.PolarPosition(-30, 0, 1)
    blocks.append(oi)

    return RendererInput(objects_blocks=[blocks], objects_audio=samples)


def get_direct_speakers_input_simple() -> RendererInput:
    samples = get_input(2000)

    dsi = visr_bear.api.DirectSpeakersInput()
    dsi.rtime = visr_bear.api.Time(0, 1)
    dsi.duration = visr_bear.api.Time(1, 1)
    dsi.type_metadata.position = visr_bear.api.PolarSpeakerPosition(30, 0, 1)

    return RendererInput(direct_speakers_blocks=[[dsi]], direct_speakers_audio=samples)


def get_hoa_input(order, norm, azimuth=0.0, roll=3000) -> RendererInput:
    acn = np.arange((order + 1) ** 2)
    n, m = hoa.from_acn(acn)

    vec = hoa.sph_harm(n, m, -np.radians(azimuth), 0.0, norm=hoa.norm_functions[norm])

    samples = get_input(roll)
    samples_enc = samples * vec[:, np.newaxis]

    hi = visr_bear.api.HOAInput()
    hi.rtime = visr_bear.api.Time(0, 1)
    hi.duration = visr_bear.api.Time(1, 1)
    hi.channels = acn
    hi.type_metadata.orders = n
    hi.type_metadata.degrees = m
    hi.type_metadata.normalization = norm

    return RendererInput(hoa_blocks=[[hi]], hoa_audio=samples_enc)


def check_matches_file(fname, actual, **allclose_kwargs):
    """Check that the samples in fname match actual. If fname does not exist,
    then it is written instead."""
    path = files_dir / fname
    if path.exists():
        expected = np.load(path)
        npt.assert_allclose(actual, expected, **allclose_kwargs)
        return []
    else:
        np.save(path, actual)
        return [f"generated {path}"]


@pytest.mark.parametrize("head_track", [False, True])
def test_outputs(head_track):
    """check that the renderer outputs are consistent"""
    # ideally these would be separate tests, but we want to use the renderer
    # outputs more than once (for individual reference tests, and multi-object
    # tests), and this avoids re-calculating them

    skips = []

    ht = "head_track" if head_track else "no_head_track"
    if head_track:
        ht = "head_track"
        listeners = get_listeners()
        pre_block_cb = listener_pre_block_cb(listeners)
        render = partial(render_input_default, pre_block_cb=pre_block_cb)
    else:
        ht = "no_head_track"
        render = render_input_default

    objects_input_simple = get_objects_input_simple()
    objects_output_simple = render(objects_input_simple)
    skips.extend(
        check_matches_file(
            f"{ht}_objects_out_simple.npy", objects_output_simple, atol=1e-4
        )
    )

    objects_input_dynamic = get_objects_input_dynamic()
    objects_output_dynamic = render(objects_input_dynamic)
    skips.extend(
        check_matches_file(
            f"{ht}_objects_out_dynamic.npy", objects_output_dynamic, atol=1e-4
        )
    )

    direct_speakers_input_simple = get_direct_speakers_input_simple()
    direct_speakers_output = render(direct_speakers_input_simple)
    skips.extend(
        check_matches_file(
            f"{ht}_direct_speakers_out.npy", direct_speakers_output, atol=1e-4
        )
    )

    hoa_input = get_hoa_input(1, "SN3D")
    hoa_output = render(hoa_input)
    skips.extend(check_matches_file(f"{ht}_hoa_out.npy", hoa_output, atol=1e-4))

    # check that when we render multiple items, the result is the same as
    # rendering them individually; checks for possible routing errors.

    multi_input = concat_inputs(
        [
            objects_input_simple,
            objects_input_dynamic,
            direct_speakers_input_simple,
            hoa_input,
        ]
    )
    multi_output = render(multi_input)

    npt.assert_allclose(
        multi_output,
        objects_output_simple
        + objects_output_dynamic
        + direct_speakers_output
        + hoa_output,
        atol=1e-5,
    )

    if skips:
        pytest.skip("; ".join(skips))
