import visr_bear
from visr_bear.api import Time
import pytest
import numpy as np
import numpy.testing as npt
from utils import data_path, make_flow
import warnings

with warnings.catch_warnings():
    warnings.simplefilter("ignore")
    import quaternion


@pytest.fixture(scope="module")
def basic_config():
    config = visr_bear.api.Config()
    config.num_objects_channels = 1
    config.num_direct_speakers_channels = 0
    config.num_hoa_channels = 0
    config.period_size = 512
    config.data_path = data_path
    return config


@pytest.fixture
def flow(basic_config):
    return make_flow(basic_config, visr_bear.SelectBRIR)


@pytest.fixture
def transform_rotation(flow):
    listener_in = flow.parameterReceivePort("listener_in")
    listener_out = flow.parameterSendPort("listener_out")
    brir_index_out = flow.parameterSendPort("brir_index_out")

    def transform_rotation(q):
        listener_in.data().orientation = q.components
        listener_in.swapBuffers()

        flow.process()

        assert listener_out.changed()
        assert brir_index_out.changed()

        idx = brir_index_out.data().value
        new_q = quaternion.quaternion(*listener_out.data().orientation)
        return idx, new_q

    return transform_rotation


def test_basic(flow, transform_rotation):
    # no rotation
    q = quaternion.quaternion(1, 0, 0, 0)
    idx, new_q = transform_rotation(q)
    assert idx == 0
    npt.assert_allclose(new_q, quaternion.quaternion(1, 0, 0, 0), atol=1e-6)

    # rotate onto view we have; results in no residual
    q = quaternion.from_rotation_vector(np.array([0, 0, -1]) * np.radians(2))
    idx, new_q = transform_rotation(q)
    assert idx == 1
    npt.assert_allclose(new_q, quaternion.quaternion(1, 0, 0, 0), atol=1e-6)

    # rotate up/down; all in residual
    q = quaternion.from_rotation_vector(np.array([1, 0, 0]) * np.radians(2))
    idx, new_q = transform_rotation(q)
    assert idx == 0
    npt.assert_allclose(new_q, q)

    # rotate just past view, residual should be just past front in the same direction
    q = quaternion.from_rotation_vector(np.array([0, 0, -1]) * np.radians(2.5))
    expected_q = quaternion.from_rotation_vector(np.array([0, 0, -1]) * np.radians(0.5))
    idx, new_q = transform_rotation(q)
    assert idx == 1
    npt.assert_allclose(new_q, expected_q)


s = 0.5 ** 0.5


# check that rotations using the listener API correspond with what we expect
# and the quaternion library
@pytest.mark.parametrize(
    "q, look, right, up",
    [
        # no rotation
        ((1, 0, 0, 0), (0, 1, 0), (1, 0, 0), (0, 0, 1)),
        # rotate up and down
        ((s, -s, 0, 0), (0, 0, 1), (1, 0, 0), (0, -1, 0)),
        ((s, s, 0, 0), (0, 0, -1), (1, 0, 0), (0, 1, 0)),
        # rotate right and left around +y
        ((s, 0, -s, 0), (0, 1, 0), (0, 0, -1), (1, 0, 0)),
        ((s, 0, s, 0), (0, 1, 0), (0, 0, 1), (-1, 0, 0)),
        # rotate left and right
        ((s, 0, 0, -s), (-1, 0, 0), (0, 1, 0), (0, 0, 1)),
        ((s, 0, 0, s), (1, 0, 0), (0, -1, 0), (0, 0, 1)),
    ],
)
def test_quaternion_examples(q, look, right, up):
    q_obj = quaternion.quaternion(*q)
    l = visr_bear.api.Listener()
    l.orientation_quaternion = q

    npt.assert_allclose(
        look, quaternion.rotate_vectors(q_obj.conjugate(), [0, 1, 0]), atol=1e-6
    )
    npt.assert_allclose(look, l.look, atol=1e-6)

    npt.assert_allclose(
        right,
        quaternion.rotate_vectors(q_obj.conjugate(), [1, 0, 0]),
        atol=1e-6,
    )
    npt.assert_allclose(right, l.right, atol=1e-6)

    npt.assert_allclose(
        up, quaternion.rotate_vectors(q_obj.conjugate(), [0, 0, 1]), atol=1e-6
    )
    npt.assert_allclose(up, l.up, atol=1e-6)
