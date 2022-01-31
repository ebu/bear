import visr_bear
from visr_bear.api import Time
import pytest
from fractions import Fraction
import numpy as np
from utils import data_path


@pytest.fixture
def basic_config():
    config = visr_bear.api.Config()
    config.num_objects_channels = 1
    config.num_direct_speakers_channels = 0
    config.num_hoa_channels = 0
    config.period_size = 512
    config.data_path = data_path
    return config


def to_frac(f):
    return Fraction(f.numerator, f.denominator)


def to_time(f):
    return visr_bear.api.Time(f.numerator, f.denominator)


def dummy_process_call(renderer, config):
    def make_buffer(nch):
        return np.zeros((nch, config.period_size), dtype=np.float32)

    output = make_buffer(2)
    renderer.process(
        make_buffer(config.num_objects_channels),
        make_buffer(config.num_direct_speakers_channels),
        make_buffer(config.num_hoa_channels),
        output,
    )


def test_timing_api(basic_config):
    renderer = visr_bear.api.Renderer(basic_config)
    assert to_frac(renderer.get_block_start_time()) == 0

    dummy_process_call(renderer, basic_config)

    assert to_frac(renderer.get_block_start_time()) == Fraction(
        basic_config.period_size, basic_config.sample_rate
    )

    offset = Fraction(1, 10)
    renderer.set_block_start_time(to_time(offset))

    assert to_frac(renderer.get_block_start_time()) == offset

    dummy_process_call(renderer, basic_config)

    assert to_frac(renderer.get_block_start_time()) == offset + Fraction(
        basic_config.period_size, basic_config.sample_rate
    )


def test_future_blocks(basic_config):
    basic_config.num_direct_speakers_channels = 1
    renderer = visr_bear.api.Renderer(basic_config)

    # try pushing blocks which start in the next frame
    objects_block = visr_bear.api.ObjectsInput()
    objects_block.rtime = Time(512, 48000)
    objects_block.duration = Time(512, 48000)

    direct_speakers_block = visr_bear.api.DirectSpeakersInput()
    direct_speakers_block.rtime = Time(512, 48000)
    direct_speakers_block.duration = Time(512, 48000)

    # it doesn't work
    assert not renderer.add_objects_block(0, objects_block)
    assert not renderer.add_direct_speakers_block(0, direct_speakers_block)

    # if we advance by 1 sample, it does
    renderer.set_block_start_time(Time(1, 48000))
    assert renderer.add_objects_block(0, objects_block)
    assert renderer.add_direct_speakers_block(0, direct_speakers_block)


def test_add_error(basic_config):
    renderer = visr_bear.api.Renderer(basic_config)

    oi = visr_bear.api.ObjectsInput()
    with pytest.raises(ValueError):
        renderer.add_objects_block(2, oi)
    dummy_process_call(renderer, basic_config)

    dsi = visr_bear.api.DirectSpeakersInput()
    with pytest.raises(ValueError):
        renderer.add_direct_speakers_block(1, dsi)
    dummy_process_call(renderer, basic_config)
