import visr_bear
from visr_bear.api import Time
import pytest
import numpy as np
import numpy.testing as npt
from utils import data_path, make_flow


@pytest.fixture(scope="module")
def basic_config():
    config = visr_bear.api.Config()
    config.num_objects_channels = 1
    config.num_direct_speakers_channels = 1
    config.num_hoa_channels = 0
    config.period_size = 512
    config.data_path = data_path
    return config


@pytest.fixture
def flow(basic_config):
    return make_flow(basic_config, visr_bear.GainCalcObjects)


@pytest.fixture(scope="module")
def blocks_and_gain_vectors(basic_config):
    flow = make_flow(basic_config, visr_bear.GainCalcObjects)
    metadata_in = flow.parameterReceivePort("metadata_in")
    gains_out = flow.parameterSendPort("gains_out")

    out = []

    for azimuth in [0, 30, 60]:
        oi = visr_bear.api.ObjectsInput()
        oi.type_metadata.position = visr_bear.api.PolarPosition(azimuth, 0.0, 1.0)
        metadata_in.enqueue(visr_bear.ADMParameterObjects(0, oi))
        flow.process()
        out.append((oi, np.array(gains_out.data())[:, 0]))
    return out


@pytest.fixture(scope="module")
def example_blocks(blocks_and_gain_vectors):
    return [block for block, gv in blocks_and_gain_vectors]


@pytest.fixture(scope="module")
def example_gain_vectors(blocks_and_gain_vectors):
    return [gv for block, gv in blocks_and_gain_vectors]


@pytest.fixture(scope="module")
def zero_gains(example_gain_vectors):
    return np.zeros_like(example_gain_vectors[0])


@pytest.fixture
def push(flow):
    metadata_in = flow.parameterReceivePort("metadata_in")

    def f(block, **kwargs):
        block = visr_bear.api.ObjectsInput(block)
        for key, value in kwargs.items():
            setattr(block, key, value)
        metadata_in.enqueue(visr_bear.ADMParameterObjects(0, block))

    return f


@pytest.fixture
def expect(flow):
    gains_out = flow.parameterSendPort("gains_out")

    def f(expected, **kwargs):
        flow.process()
        actual = np.array(gains_out.data())[:, 0]
        npt.assert_allclose(actual, expected, **kwargs)

    return f


def test_single_block(push, expect, example_blocks, example_gain_vectors, zero_gains):
    push(example_blocks[0], rtime=Time(0, 48000), duration=Time(512, 48000))
    expect(example_gain_vectors[0])
    expect(zero_gains)


def test_multiple_time_aligned_blocks(
    push, expect, example_blocks, example_gain_vectors, zero_gains
):
    push(example_blocks[0], rtime=Time(0, 48000), duration=Time(512, 48000))
    expect(example_gain_vectors[0])

    push(example_blocks[1], rtime=Time(512, 48000), duration=Time(512, 48000))
    expect(example_gain_vectors[1])

    push(example_blocks[2], rtime=Time(1024, 48000), duration=Time(1024, 48000))
    expect(0.5 * example_gain_vectors[1] + 0.5 * example_gain_vectors[2])
    expect(example_gain_vectors[2])

    expect(zero_gains)


def test_silence_between_blocks(
    push, expect, example_blocks, example_gain_vectors, zero_gains
):
    for i in range(2):
        push(
            example_blocks[i],
            rtime=Time(512 * i * 3, 48000),
            duration=Time(512 * 2, 48000),
        )
        expect(example_gain_vectors[i])
        expect(example_gain_vectors[i])

        expect(zero_gains)


def test_multiple_blocks_in_round(
    push, expect, example_blocks, example_gain_vectors, zero_gains
):
    push(example_blocks[0], rtime=Time(0, 48000), duration=Time(256, 48000))
    push(example_blocks[1], rtime=Time(256, 48000), duration=Time(256, 48000))
    expect(example_gain_vectors[1])

    push(example_blocks[2], rtime=Time(512, 48000), duration=Time(256, 48000))
    push(example_blocks[0], rtime=Time(768, 48000), duration=Time(256, 48000))
    expect(example_gain_vectors[0])

    expect(zero_gains)


def test_distance_behaviour(
    push, expect, example_blocks, example_gain_vectors, zero_gains
):
    distances = []
    absoluteDistances = []

    class MyDistanceBehaviour(visr_bear.api.DistanceBehaviour):
        def __init__(self):
            visr_bear.api.DistanceBehaviour.__init__(self)

        def get_gain(self, distance_m, absoluteDistance):
            distances.append(distance_m)
            absoluteDistances.append(absoluteDistance)
            return 0.5

    def expect_distance(expected_distance, expected_absoluteDistance):
        [distance] = distances
        [absoluteDistance] = absoluteDistances
        distances.clear()
        absoluteDistances.clear()

        assert distance == expected_distance
        assert expected_absoluteDistance == absoluteDistance

    # HACK: we need to keep this alive manually, as pybind11 doesn't do it for us
    distance_behaviour = MyDistanceBehaviour()

    block_distance = visr_bear.api.ObjectsInput(example_blocks[0])
    block_distance.distance_behaviour = distance_behaviour
    push(block_distance, rtime=Time(512, 48000), duration=Time(512, 48000))
    expect(0.5 * example_gain_vectors[0])
    expect_distance(1.0, None)

    block_distance = visr_bear.api.ObjectsInput(example_blocks[0])
    block_distance.type_metadata.position.distance = 2.0
    block_distance.audioPackFormat_data.absoluteDistance = 0.5
    block_distance.distance_behaviour = distance_behaviour
    push(block_distance, rtime=Time(2 * 512, 48000), duration=Time(512, 48000))
    expect(0.5 * example_gain_vectors[0])
    expect_distance(1.0, 0.5)
