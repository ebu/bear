from visr_bear import DynamicRenderer
import visr_bear.api
from visr_bear.api import Config, Renderer, Listener
from test_changes import do_render, RendererInput
import numpy as np
import time
import pytest
from utils import data_path
import warnings


def make_rt_pre_block_cb():
    """make a pre-block callback which delays to make the rendering process run in
    real-time; this is useful for DynamicRenderer, as otherwise there may not
    be time for the background construction to complete"""
    start_time = None

    def pre_block_cb(renderer, frame, period):
        nonlocal start_time
        if frame == 0:
            start_time = time.time()
        else:
            delay_req = (start_time + frame * period / 48000) - time.time()
            if delay_req > 0:
                time.sleep(delay_req)

    return pre_block_cb


def get_objects_input_dynamic() -> RendererInput:
    t = 10
    samples = 0.1 * np.random.normal(size=(1, t * 48000)).astype(np.float32)

    blocks = []
    for i in range(t):
        oi = visr_bear.api.ObjectsInput()
        oi.rtime = visr_bear.api.Time(i, 1)
        oi.duration = visr_bear.api.Time(1, 1)
        oi.type_metadata.position = visr_bear.api.PolarPosition(10 * (t - i), 0, 1)
        blocks.append(oi)

    return RendererInput(objects_blocks=[blocks], objects_audio=samples)


def get_objects_input_static() -> RendererInput:
    t = 10
    samples = 0.1 * np.random.normal(size=(1, t * 48000)).astype(np.float32)

    oi = visr_bear.api.ObjectsInput()
    oi.rtime = visr_bear.api.Time(0, 1)
    oi.duration = visr_bear.api.Time(t, 1)
    oi.type_metadata.position = visr_bear.api.PolarPosition(0, 0, 1)

    return RendererInput(objects_blocks=[[oi]], objects_audio=samples)


def setup_objects_config(config: Config):
    config.num_objects_channels = 1


def expand_objects_config(config: Config):
    config.num_objects_channels += 1


def get_direct_speakers_input_dynamic() -> RendererInput:
    t = 10
    samples = 0.1 * np.random.normal(size=(1, t * 48000)).astype(np.float32)

    blocks = []
    for i in range(t):
        oi = visr_bear.api.DirectSpeakersInput()
        oi.rtime = visr_bear.api.Time(i, 1)
        oi.duration = visr_bear.api.Time(1, 1)
        oi.type_metadata.position = visr_bear.api.PolarSpeakerPosition(
            10 * (t - i), 0, 1
        )
        blocks.append(oi)

    return RendererInput(direct_speakers_blocks=[blocks], direct_speakers_audio=samples)


def get_direct_speakers_input_static() -> RendererInput:
    t = 10
    samples = 0.1 * np.random.normal(size=(1, t * 48000)).astype(np.float32)

    oi = visr_bear.api.DirectSpeakersInput()
    oi.rtime = visr_bear.api.Time(0, 1)
    oi.duration = visr_bear.api.Time(t, 1)
    oi.type_metadata.position = visr_bear.api.PolarSpeakerPosition(0, 0, 1)

    return RendererInput(direct_speakers_blocks=[[oi]], direct_speakers_audio=samples)


def setup_direct_speakers_config(config: Config):
    config.num_direct_speakers_channels = 1


def expand_direct_speakers_config(config: Config):
    config.num_direct_speakers_channels += 1


def get_hoa_input(order=1, norm="SN3D", azimuth=0.0) -> RendererInput:
    from ear.core import hoa

    acn = np.arange((order + 1) ** 2)
    n, m = hoa.from_acn(acn)

    vec = hoa.sph_harm(n, m, -np.radians(azimuth), 0.0, norm=hoa.norm_functions[norm])

    t = 10
    samples = 0.1 * np.random.normal(size=t * 48000).astype(np.float32)
    samples_enc = samples * vec[:, np.newaxis]

    hi = visr_bear.api.HOAInput()
    hi.rtime = visr_bear.api.Time(0, 1)
    hi.duration = visr_bear.api.Time(t, 1)
    hi.channels = acn
    hi.type_metadata.orders = n
    hi.type_metadata.degrees = m
    hi.type_metadata.normalization = norm

    return RendererInput(hoa_blocks=[[hi]], hoa_audio=samples_enc)


def setup_hoa_config(config: Config):
    config.num_hoa_channels = 4


def expand_hoa_config(config: Config):
    config.num_hoa_channels += 1


@pytest.mark.parametrize(
    "period,set_config_blocking,renderer_input,setup_config_base,expand_config",
    [
        (
            512,
            False,
            get_objects_input_dynamic(),
            setup_objects_config,
            expand_objects_config,
        ),
        (
            512,
            False,
            get_objects_input_static(),
            setup_objects_config,
            expand_objects_config,
        ),
        (
            512,
            False,
            get_direct_speakers_input_dynamic(),
            setup_direct_speakers_config,
            expand_direct_speakers_config,
        ),
        (
            512,
            False,
            get_direct_speakers_input_static(),
            setup_direct_speakers_config,
            expand_direct_speakers_config,
        ),
        (512, False, get_hoa_input(), setup_hoa_config, expand_hoa_config),
        (
            64,
            False,
            get_objects_input_dynamic(),
            setup_objects_config,
            expand_objects_config,
        ),
        (
            512,
            True,
            get_objects_input_dynamic(),
            setup_objects_config,
            expand_objects_config,
        ),
    ],
    ids=[
        "objects_dynamic",
        "objects_static",
        "direct_speakers_dynamic",
        "direct_speakers_static",
        "hoa",
        "objects_dynamic_64",
        "objects_dynamic_blocking",
    ],
)
def test_dynamic_renderer(
    period, set_config_blocking, renderer_input, setup_config_base, expand_config
):
    """test that the dynamic renderer matches the static renderer, except for
    some fading up/down when it's configuring"""
    r = DynamicRenderer(period, 100)

    def base_config():
        c = Config()
        c.period_size = period
        c.data_path = data_path

        setup_config_base(c)

        return c

    # common for static and dynamic
    def pre_block_cb_common(renderer, frame, period):
        if frame == int((5 * 48000) / period):
            listener = Listener()
            s = 0.5**0.5
            listener.orientation_quaternion = (s, 0, -s, 0)
            renderer.set_listener(listener, None)

    delay_rt = make_rt_pre_block_cb()

    # config changes for dynamic renderer
    def pre_block_cb_dynamic(renderer, frame, period):
        delay_rt(renderer, frame, period)
        pre_block_cb_common(renderer, frame, period)

        renderer.throw_error()

        if frame == 0:
            c = base_config()

            if set_config_blocking:
                renderer.set_config_blocking(c)
            else:
                renderer.set_config(c)
        elif frame == int((6 * 48000) / period):
            assert renderer.is_running()

            c = base_config()
            expand_config(c)
            renderer.set_config(c)

    renderer_output_dynamic = do_render(
        r, period, renderer_input, pre_block_cb=pre_block_cb_dynamic
    )

    assert r.is_running()

    c = base_config()
    r = Renderer(c)

    renderer_output_static = do_render(
        r, period, renderer_input, pre_block_cb=pre_block_cb_common
    )

    # check that the fading profile is as expected by determining if each block
    # fade-up/fade-down/passthrough/silence and checking that the order of
    # those is sensible
    fade_up = np.arange(period) / period
    fade_down = 1 - fade_up

    faded_down = not set_config_blocking
    seen_passthrough = False
    for block_idx in range(renderer_output_static.shape[1] // period):
        s = np.s_[block_idx * period : (block_idx + 1) * period]
        block_static = renderer_output_static[:, s]
        block_dynamic = renderer_output_dynamic[:, s]

        allclose_args = dict(atol=1e-6)
        # ignore silent bit at the start
        if np.allclose(block_static, 0.0, **allclose_args):
            continue

        block_is_fade_up = np.allclose(
            block_dynamic, block_static * fade_up, **allclose_args
        )
        block_is_fade_down = np.allclose(
            block_dynamic, block_static * fade_down, **allclose_args
        )
        block_is_silence = np.allclose(block_dynamic, 0.0, **allclose_args)
        block_is_passthrough = np.allclose(block_dynamic, block_static, **allclose_args)

        assert (
            block_is_fade_up
            or block_is_fade_down
            or block_is_silence
            or block_is_passthrough
        )

        if faded_down:
            assert block_is_fade_up or block_is_silence
            if block_is_fade_up:
                faded_down = False
        else:
            assert block_is_fade_down or block_is_passthrough
            if block_is_fade_down:
                faded_down = True

        if block_is_passthrough:
            seen_passthrough = True

    if not seen_passthrough:
        warnings.warn(
            "no output; this is caused by a logic error, or renderer "
            "construction being too slow"
        )

    if seen_passthrough and faded_down:
        warnings.warn(
            "no output at end; this is caused by a logic error around config "
            "switching, or renderer construction being too slow"
        )


def test_dynamic_renderer_construct():
    r = DynamicRenderer(512, 100)
    c = Config()
    c.period_size = 512
    c.num_objects_channels = 1
    c.data_path = data_path

    r.set_config(c)


def test_dynamic_renderer_error():
    r = DynamicRenderer(512, 100)
    c = Config()
    c.period_size = 512
    c.num_objects_channels = 1
    c.data_path = "data/does_not_exist.tf"

    r.set_config(c)

    with pytest.raises(RuntimeError, match="No such file or directory"):
        for i in range(100):
            time.sleep(0.1)
            r.throw_error()


if __name__ == "__main__":
    test_dynamic_renderer()
