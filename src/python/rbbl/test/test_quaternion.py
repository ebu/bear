
import numpy as np
import scipy.spatial as ssp
from scipy.spatial.transform import Rotation

import pytest

import rbbl


quaternionTypes = [ rbbl.Quaternion, rbbl.QuaternionDouble ]

def coordinateType( pos ):
    if pos == rbbl.Quaternion:
        return np.float32
    elif pos == rbbl.QuaternionDouble:
        return np.float64
    else:
        return None

def quaternionType( pos ):
    if pos == rbbl.Position3D:
        return rbbl.Quaternion
    elif pos == rbbl.Position3DDouble:
        return rbbl.QuaternionDouble
    else:
        return None


def positionType( quat ):
    """
    Helper function to get the corresponding position type for a quaternion.
    """
    if quat == rbbl.Quaternion:
        return rbbl.Position3D
    elif quat == rbbl.QuaternionDouble:
        return rbbl.Position3DDouble
    else:
        return None

@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionDefaultInit(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    refQuat = np.array([1, 0, 0, 0 ], dtype=dtype)
    assert np.abs( quat.w - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quat.x - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionInitScalar(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    
    quat = quatType(refQuat)
    assert np.abs( quat.w - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quat.x - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionInitVector(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    quat = quatType(refQuat)
    assert np.abs( quat.w - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quat.x - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps
    
    
@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionInitFromYPR(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    
    quat = quatType.fromYPR( ypr[0], ypr[1], ypr[2])
    quatVec = quat.data
    assert np.abs( quatVec[0] - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quatVec[1] - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quatVec[2] - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quatVec[3] - refQuat[3] ) < np.finfo(dtype).eps
    
    assert np.abs(quat.yaw - ypr[0]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.pitch - ypr[1]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.roll - ypr[2]) < 4*np.finfo(dtype).eps    
    
    
@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionInitFromRotationVector(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    
    rotVecRaw = rot.as_rotvec()
    rotAngle = np.linalg.norm(rotVecRaw)
    rotVec = rotVecRaw / rotAngle
    
    quat = quatType.fromRotationVector( positionType(quatType)(rotVec), rotAngle)
    quatVec = quat.data
    assert np.abs( quatVec[0] - refQuat[0] ) < 4*np.finfo(dtype).eps
    assert np.abs( quatVec[1] - refQuat[1] ) < 4*np.finfo(dtype).eps
    assert np.abs( quatVec[2] - refQuat[2] ) < 4*np.finfo(dtype).eps
    assert np.abs( quatVec[3] - refQuat[3] ) < 4*np.finfo(dtype).eps
    
    assert np.abs(quat.yaw - ypr[0]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.pitch - ypr[1]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.roll - ypr[2]) < 4*np.finfo(dtype).eps
    
    rotVecRes = np.asarray(quat.rotationVector)
    assert np.linalg.norm(rotVecRes - rotVec) < 4 * np.finfo(dtype).eps
    assert np.abs(quat.rotationAngle - rotAngle) < 4*np.finfo(dtype).eps 


    
@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionAccessors(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    quat = quatType(refQuat)
    assert np.abs( quat.w - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quat.x - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps
    
    newW = 0.123
    quat.w = newW
    assert np.abs( quat.w - newW ) < np.finfo(dtype).eps
    assert np.abs( quat.x - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps
    newX = -0.31425
    quat.x = newX
    assert np.abs( quat.w - newW ) < np.finfo(dtype).eps
    assert np.abs( quat.x - newX ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps
    
    newY = 0.9
    quat.y = newY
    assert np.abs( quat.w - newW ) < np.finfo(dtype).eps
    assert np.abs( quat.x - newX ) < np.finfo(dtype).eps
    assert np.abs( quat.y - newY ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps
        
    newZ = -0.425
    quat.z = newZ
    assert np.abs( quat.w - newW ) < np.finfo(dtype).eps
    assert np.abs( quat.x - newX ) < np.finfo(dtype).eps
    assert np.abs( quat.y - newY ) < np.finfo(dtype).eps
    assert np.abs( quat.z - newZ ) < np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionSetScalar(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    
    quat = quatType()
    quat.set(refQuat[0], refQuat[1], refQuat[2], refQuat[3])
    assert np.abs( quat.w - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quat.x - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionSetVector(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    quat = quatType()
    quat.data = refQuat
    assert np.abs( quat.w - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quat.x - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quat.y - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quat.z - refQuat[3] ) < np.finfo(dtype).eps
    
    
@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionSetYPR(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    
    quat = quatType()
    quat.setYPR(ypr[0], ypr[1], ypr[2])
    quatVec = quat.data
    assert np.abs( quatVec[0] - refQuat[0] ) < np.finfo(dtype).eps
    assert np.abs( quatVec[1] - refQuat[1] ) < np.finfo(dtype).eps
    assert np.abs( quatVec[2] - refQuat[2] ) < np.finfo(dtype).eps
    assert np.abs( quatVec[3] - refQuat[3] ) < np.finfo(dtype).eps
    
    assert np.abs(quat.yaw - ypr[0]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.pitch - ypr[1]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.roll - ypr[2]) < 4*np.finfo(dtype).eps    
    
    
@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionSetRotationVector(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()
    
    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    
    rotVecRaw = rot.as_rotvec()
    rotAngle = np.linalg.norm(rotVecRaw)
    rotVec = rotVecRaw / rotAngle
    
    quat = quatType()
    quat.setRotationVector( positionType(quatType)(rotVec), rotAngle)
    quatVec = quat.data
    assert np.abs( quatVec[0] - refQuat[0] ) < 4*np.finfo(dtype).eps
    assert np.abs( quatVec[1] - refQuat[1] ) < 4*np.finfo(dtype).eps
    assert np.abs( quatVec[2] - refQuat[2] ) < 4*np.finfo(dtype).eps
    assert np.abs( quatVec[3] - refQuat[3] ) < 4*np.finfo(dtype).eps
    
    assert np.abs(quat.yaw - ypr[0]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.pitch - ypr[1]) < 4*np.finfo(dtype).eps
    assert np.abs(quat.roll - ypr[2]) < 4*np.finfo(dtype).eps 
    
    rotVecRes = np.asarray(quat.rotationVector)
    assert np.linalg.norm(rotVecRes - rotVec) < 4 * np.finfo(dtype).eps
    assert np.abs(quat.rotationAngle - rotAngle) < 4*np.finfo(dtype).eps 

@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNorm(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()

    refQuat = np.asarray( [0.7, 0.3, -0.8, 0.3875] ) # Random, non-unit quaternion
    quat = quatType(refQuat)    
    refNorm = np.linalg.norm(refQuat)
    quatNorm = quat.norm()    
    assert np.abs(refNorm-quatNorm) < 4*np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNormSquare(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()

    refQuat = np.asarray( [0.7, 0.3, -0.8, 0.3875] ) # Random, non-unit quaternion
    quat = quatType(refQuat)    
    refNormSqr = np.dot(refQuat, refQuat)
    quatNormSqr = quat.normSquare()
    
    assert np.abs(refNormSqr-quatNormSqr) < 4*np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNormalise(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()

    # Random, non-unit quaternion with negative w (to check that sign adjustment
    # is performed only when requested)
    refQuat = np.asarray( [-0.7, 0.3, -0.8, 0.3875] )
    refQuatNormed = refQuat / np.linalg.norm( refQuat)
    
    quat = quatType(refQuat)
    quat.normalise()
    
    assert np.abs( quat.norm() - 1.0 ) < 4*np.finfo(dtype).eps
    
    quatNormedVec = quat.data
    assert np.abs( np.linalg.norm(quatNormedVec) - 1.0 ) < 4*np.finfo(dtype).eps
    assert np.linalg.norm(quatNormedVec - refQuatNormed ) < 4*np.finfo(dtype).eps
    
    
    # Same but with explicit 'adjustSign'
    quat = quatType(refQuat)
    quat.normalise(adjustSign=False)
    
    assert np.abs( quat.norm() - 1.0 ) < 4*np.finfo(dtype).eps
    
    quatNormedVec = quat.data
    assert np.abs( np.linalg.norm(quatNormedVec) - 1.0 ) < 4*np.finfo(dtype).eps
    assert np.linalg.norm(quatNormedVec - refQuatNormed ) < 4*np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNormaliseZeroVector(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()

    refQuat = np.asarray( [0.25*np.finfo(dtype).eps,0,0,0], dtype=dtype )    
    quat = quatType(refQuat)
    
    with pytest.raises(Exception):
        quat.normalise(silentDivideByZero=False)
    with pytest.raises(Exception):
        quat.normalise() # Default does the same
    
    # Does not raise an exception
    quat.normalise(silentDivideByZero=True)


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNormaliseAdjustSign(quatType):
    dtype = coordinateType(quatType)
    quat = quatType()

    # Random, non-unit quaternion with negative w (to check that sign adjustment
    # is performed only when requested)
    refQuat = np.asarray( [-0.7, 0.3, -0.8, 0.3875] )
    refQuatNormed = np.sign(refQuat[0])*refQuat / np.linalg.norm( refQuat)
    
    quat = quatType(refQuat)
    quat.normalise(adjustSign=True)
    
    assert np.abs( quat.norm() - 1.0 ) < 4*np.finfo(dtype).eps
    
    quatNormedVec = quat.data
    assert np.abs( np.linalg.norm(quatNormedVec) - 1.0 ) < 4*np.finfo(dtype).eps
    assert np.linalg.norm(quatNormedVec - refQuatNormed ) < 4*np.finfo(dtype).eps
 
    
@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNormaliseFunction(quatType):
    dtype = coordinateType(quatType)

    # Random, non-unit quaternion with negative w (to check that sign adjustment
    # is performed only when requested)
    refQuat = np.asarray( [-0.7, 0.3, -0.8, 0.3875] )
    refQuatNormed = refQuat / np.linalg.norm( refQuat)
    
    quat = quatType(refQuat)
    quatNew = rbbl.normalise(quat)
    
    assert np.abs( quatNew.norm() - 1.0 ) < 4*np.finfo(dtype).eps
    
    quatNormedVec = quatNew.data
    assert np.abs( np.linalg.norm(quatNormedVec) - 1.0 ) < 4*np.finfo(dtype).eps
    assert np.linalg.norm(quatNormedVec - refQuatNormed ) < 4*np.finfo(dtype).eps
    
    
    # Same but with explicit 'adjustSign'
    quat = quatType(refQuat)
    quatNew = rbbl.normalise(quat, adjustSign=False)
    
    assert np.abs( quatNew.norm() - 1.0 ) < 4*np.finfo(dtype).eps
    
    quatNormedVec = quatNew.data
    assert np.abs( np.linalg.norm(quatNormedVec) - 1.0 ) < 4*np.finfo(dtype).eps
    assert np.linalg.norm(quatNormedVec - refQuatNormed ) < 4*np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNormaliseFunctionZeroVector(quatType):
    dtype = coordinateType(quatType)

    refQuat = np.asarray( [0.25*np.finfo(dtype).eps,0,0,0], dtype=dtype )    
    quat = quatType(refQuat)
    
    with pytest.raises(Exception):
        rbbl.normalise(quat, silentDivideByZero=False)
    with pytest.raises(Exception):
        rbbl.normalise(quat) # Default does the same
    
    # Does not raise an exception
    rbbl.normalise(quat, silentDivideByZero=True)


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNormaliseFunctionAdjustSign(quatType):
    dtype = coordinateType(quatType)

    # Random, non-unit quaternion with negative w (to check that sign adjustment
    # is performed only when requested)
    refQuat = np.asarray( [-0.7, 0.3, -0.8, 0.3875] )
    refQuatNormed = np.sign(refQuat[0])*refQuat / np.linalg.norm( refQuat)
    
    quat = quatType(refQuat)
    quatNew = rbbl.normalise(quat, adjustSign=True)
    
    assert np.abs( quatNew.norm() - 1.0 ) < 4*np.finfo(dtype).eps
    
    quatNormedVec = quatNew.data
    assert np.abs( np.linalg.norm(quatNormedVec) - 1.0 ) < 4*np.finfo(dtype).eps
    assert np.linalg.norm(quatNormedVec - refQuatNormed ) < 4*np.finfo(dtype).eps
 
    
@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionConjugate(quatType):
    dtype = coordinateType(quatType)

    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    refConj = refQuat * np.asarray([1,-1,-1,-1], dtype=dtype)
    
    quat = quatType(refQuat)
    quat.conjugate()
    quatRes = quat.data

    assert np.linalg.norm(quatRes - refConj) < 4*np.finfo(dtype).eps
    

@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionConjugateFunction(quatType):
    dtype = coordinateType(quatType)

    # Create an arbitrary quaternion
    ypr = np.deg2rad([45, -15, 83])
    rot = Rotation.from_euler('ZYX', ypr)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot.as_quat()[[3,0,1,2]]
    refConj = refQuat * np.asarray([1,-1,-1,-1], dtype=dtype)
    
    quat = quatType(refQuat)
    quatNew = rbbl.conjugate(quat)
    quatRes = quatNew.data

    assert np.linalg.norm(quatRes - refConj) < 4*np.finfo(dtype).eps
    

@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionRotate(quatType):
    dtype = coordinateType(quatType)


    # Create an arbitrary quaternion
    ypr1 = np.deg2rad([45, 15, 83])
    rot1 = Rotation.from_euler('ZYX', ypr1)
    # Note: The The scipy.spatial Quaternion representation stores w last.
    refQuat = rot1.as_quat()[[3,0,1,2]]
    quat = quatType(refQuat)
    
    ypr2 = np.deg2rad([30, -29.7, 135.487])
    rot2 = Rotation.from_euler('ZYX', ypr2)
    rotQuatVec = rot2.as_quat()[[3,0,1,2]]
    rotQuat = quatType(rotQuatVec)
    
    rotRef = rot2 * rot1
    rotRefQuat = rotRef.as_quat()[[3,0,1,2]]

    quat.rotate(rotQuat)
    quatResVec = quat.data
    
    assert np.linalg.norm(quatResVec - rotRefQuat) < 4*np.finfo(dtype).eps


@pytest.mark.parametrize("quatType", quaternionTypes)
def test_quaternionNegate(quatType):
    dtype = coordinateType(quatType)

    refQuat = np.asarray( [-0.7, 0.3, -0.8, 0.3875] )
    
    quat = quatType(refQuat)
    quat2 = -quat
    quat2Vec = quat2.data 

    # Check whether they zero out
    assert np.linalg.norm(quat2Vec + refQuat ) < 4*np.finfo(dtype).eps


if __name__ == "__main__":
    for quatType in quaternionTypes:
        test_quaternionDefaultInit(quatType)
        test_quaternionInitScalar(quatType)
        test_quaternionInitVector(quatType)
        test_quaternionInitFromYPR(quatType)
        test_quaternionInitFromRotationVector(quatType)

        test_quaternionAccessors(quatType)
        test_quaternionSetScalar(quatType)
        test_quaternionSetVector(quatType)
        test_quaternionSetYPR(quatType)
        test_quaternionSetRotationVector(quatType)
        
        test_quaternionNorm(quatType)
        test_quaternionNormSquare(quatType)
        test_quaternionNormalise(quatType)
        test_quaternionNormaliseZeroVector(quatType)
        test_quaternionNormaliseAdjustSign(quatType)
        test_quaternionNormaliseFunction(quatType)
        test_quaternionNormaliseFunctionZeroVector(quatType)
        test_quaternionNormaliseFunctionAdjustSign(quatType)
        test_quaternionConjugate(quatType)
        test_quaternionConjugateFunction(quatType)
        # TODO: Free functions angle, angleNormalised, dot, interpolateSpherical
        test_quaternionRotate(quatType)
        
        test_quaternionNegate(quatType)
        # TODO: operators +=, -=, *= *=(scalar) /=(scalar), +, -, *, *(scalar)
