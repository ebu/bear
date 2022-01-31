import visr_bear
from visr_bear.api import Time
import pytest
import numpy as np
import numpy.testing as npt
from utils import data_path, make_flow
from ear.core import hoa

import warnings

with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import quaternion


@pytest.fixture(scope="module")
def basic_config():
    config = visr_bear.api.Config()
    config.num_objects_channels = 0
    config.num_direct_speakers_channels = 0
    config.num_hoa_channels = 5
    config.period_size = 512
    config.data_path = data_path
    return config


@pytest.fixture
def flow(basic_config):
    return make_flow(basic_config, visr_bear.GainCalcHOA)


def test_basic(flow):
    metadata_in = flow.parameterReceivePort("metadata_in")
    gains_out = flow.parameterSendPort("gains_out")

    metadata = visr_bear.api.HOAInput()
    metadata.channels = [1, 2, 3, 4]
    metadata.type_metadata.orders = [0, 1, 1, 1]
    metadata.type_metadata.degrees = [0, -1, 0, 1]
    metadata.type_metadata.normalization = "SN3D"

    metadata_in.enqueue(visr_bear.ADMParameterHOA(0, metadata))

    for rep in range(5):
        flow.process()
        gains = np.array(gains_out.data())
        npt.assert_allclose(gains[:4, 1:], np.eye(4))

    # check normalisation different from internal

    metadata.type_metadata.normalization = "N3D"
    metadata_in.enqueue(visr_bear.ADMParameterHOA(0, metadata))

    for rep in range(5):
        flow.process()
        gains = np.array(gains_out.data())
        n, m = hoa.from_acn(np.arange(4))
        norm = hoa.norm_SN3D(n, np.abs(m)) / hoa.norm_N3D(n, np.abs(m))
        npt.assert_allclose(gains[:4, 1:], np.diag(norm))


def test_high_order(basic_config):
    """Check for errors with input orders greater than the order of the decode
    matrix
    """
    max_order = 6
    acn = np.arange((max_order + 1) ** 2)
    orders, degrees = hoa.from_acn(acn)

    config = visr_bear.api.Config(basic_config)
    config.num_hoa_channels = len(acn)
    flow = make_flow(config, visr_bear.GainCalcHOA)

    metadata_in = flow.parameterReceivePort("metadata_in")
    gains_out = flow.parameterSendPort("gains_out")

    metadata = visr_bear.api.HOAInput()
    metadata.channels = acn
    metadata.type_metadata.orders = orders
    metadata.type_metadata.degrees = degrees
    metadata.type_metadata.normalization = "SN3D"

    metadata_in.enqueue(visr_bear.ADMParameterHOA(0, metadata))

    for rep in range(5):
        flow.process()
        gains = np.array(gains_out.data())
        assert gains.shape[1] == len(acn)
        assert gains.shape[1] > gains.shape[0], "need to increase max_order"
        npt.assert_allclose(gains, np.eye(gains.shape[0], gains.shape[1]))


def get_encoded_rot(order, q):
    """Get some points encoded as HOA before and after being rotated by q."""
    n, m = hoa.from_acn(np.arange((order + 1) ** 2))

    points = hoa.load_points()

    az = -np.arctan2(points[:, 0], points[:, 1])
    el = np.arctan2(points[:, 2], np.hypot(points[:, 0], points[:, 1]))
    encoded = hoa.sph_harm(
        n[:, np.newaxis],
        m[:, np.newaxis],
        az[np.newaxis],
        el[np.newaxis],
        norm=hoa.norm_N3D,
    )

    points_rot = quaternion.rotate_vectors(q, points)

    az = -np.arctan2(points_rot[:, 0], points_rot[:, 1])
    el = np.arctan2(points_rot[:, 2], np.hypot(points_rot[:, 0], points_rot[:, 1]))
    encoded_rot = hoa.sph_harm(
        n[:, np.newaxis],
        m[:, np.newaxis],
        az[np.newaxis],
        el[np.newaxis],
        norm=hoa.norm_N3D,
    )

    return encoded, encoded_rot


def test_rotation(flow):
    metadata_in = flow.parameterReceivePort("metadata_in")
    listener_in = flow.parameterReceivePort("listener_in")
    gains_out = flow.parameterSendPort("gains_out")

    metadata = visr_bear.api.HOAInput()
    metadata.channels = [1, 2, 3, 4]
    metadata.type_metadata.orders = [0, 1, 1, 1]
    metadata.type_metadata.degrees = [0, -1, 0, 1]
    metadata.type_metadata.normalization = "SN3D"
    metadata_in.enqueue(visr_bear.ADMParameterHOA(0, metadata))

    # orientation translates from world coordinates to listener coordinates
    q = quaternion.quaternion(1, 0, 0, 1).normalized()
    listener_in.data().orientation = q.components
    listener_in.swapBuffers()

    encoded, encoded_rot = get_encoded_rot(order=1, q=q)

    for rep in range(5):
        flow.process()
        gains = np.array(gains_out.data())

        # this is the same as extracting the relevant part from gains (as
        # above) and multiplying by encoded, but ensures that we don't
        # accidentally have it transposed (because this way the shapes wouldn't
        # match up, whereas the actual bit we care about is square) that could
        # hide a transpose in the component
        encoded_pad = np.zeros((5, encoded.shape[1]))
        encoded_pad[1:] = encoded
        encoded_trans = np.dot(gains, encoded_pad)

        npt.assert_allclose(encoded_trans[:4], encoded_rot)
        npt.assert_allclose(encoded_trans[4:], 0)


def test_timing(flow):
    """check a period-size block"""
    metadata_in = flow.parameterReceivePort("metadata_in")
    gains_out = flow.parameterSendPort("gains_out")

    metadata = visr_bear.api.HOAInput()
    metadata.rtime = Time(0, 48000)
    metadata.duration = Time(512, 48000)
    metadata.channels = [1, 2, 3, 4]
    metadata.type_metadata.orders = [0, 1, 1, 1]
    metadata.type_metadata.degrees = [0, -1, 0, 1]
    metadata.type_metadata.normalization = "SN3D"

    metadata_in.enqueue(visr_bear.ADMParameterHOA(0, metadata))

    flow.process()
    gains = np.array(gains_out.data())
    npt.assert_allclose(gains[:4, 1:], np.eye(4))

    flow.process()
    gains = np.array(gains_out.data())
    npt.assert_allclose(gains[:4, 1:], np.zeros((4, 4)))
