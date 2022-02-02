
import numpy as np
import scipy.spatial as ssp
from scipy.spatial.transform import Rotation

import pytest

import rbbl


positionTypes = [ rbbl.Position3D, rbbl.Position3DDouble ]

def coordinateType( pos ):
    if pos == rbbl.Position3D:
        return np.float32
    elif pos == rbbl.Position3DDouble:
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


@pytest.mark.parametrize("posType", positionTypes)
def test_positionDefaultInit(posType):
    dtype = coordinateType(posType)
    pos = posType()
    refPos = np.zeros(3, dtype=dtype)
    assert np.linalg.norm( np.asarray(pos) - refPos ) < 1e-7


@pytest.mark.parametrize("posType", positionTypes)
def test_positionInit(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [0.3, -0.3251, 0.75], dtype=dtype)
    pos = posType( posVec[0], posVec[1], posVec[2])
    assert np.linalg.norm( np.asarray(pos) - posVec ) < 1e-7


@pytest.mark.parametrize("posType", positionTypes)
def test_positionKeywordInit(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [0.3, -0.3251, 0.75], dtype=dtype)
    pos = posType( z=posVec[2], x=posVec[0], y=posVec[1])
    assert np.linalg.norm( np.asarray(pos) - posVec ) < 1e-7


@pytest.mark.parametrize("posType", positionTypes)
def test_positionVecInit(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [0.3, -0.3251, 0.75], dtype=dtype)
    pos = posType( posVec )
    assert np.linalg.norm( np.asarray(pos) - posVec ) < 1e-7


@pytest.mark.parametrize("posType", positionTypes)
def test_positionIndexAccess(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.3, 0.3251, -0.75], dtype=dtype)
    pos = posType( posVec )
    assert np.abs( pos[0] - posVec[0] ) < 1e-7
    assert np.abs( pos[1] - posVec[1] ) < 1e-7
    assert np.abs( pos[2] - posVec[2] ) < 1e-7
    

@pytest.mark.parametrize("posType", positionTypes)
def test_positionElementAccess(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.3, 0.3251, -0.75])
    pos = posType( posVec )
    assert np.abs( pos.x - posVec[0] ) < 1e-7
    assert np.abs( pos.y - posVec[1] ) < 1e-7
    assert np.abs( pos.z - posVec[2] ) < 1e-7


@pytest.mark.parametrize("posType", positionTypes)
def test_positionIndexedSet(posType):
    dtype = coordinateType(posType)
    initPos = np.asarray( [1.3, 0.3251, -0.75], dtype=dtype)
    newPos = np.asarray( [-0.35, 0.251, 1.75], dtype=dtype)
    pos = posType( initPos )
    assert np.linalg.norm( np.asarray(pos) - initPos ) < 1e-7

    pos[0] = newPos[0]
    assert np.abs( pos[0] - newPos[0] ) < 1e-7
    assert np.abs( pos[1] - initPos[1] ) < 1e-7
    assert np.abs( pos[2] - initPos[2] ) < 1e-7
    pos[1] = newPos[1]
    assert np.abs( pos[0] - newPos[0] ) < 1e-7
    assert np.abs( pos[1] - newPos[1] ) < 1e-7
    assert np.abs( pos[2] - initPos[2] ) < 1e-7
    pos[2] = newPos[2]
    assert np.abs( pos[0] - newPos[0] ) < 1e-7
    assert np.abs( pos[1] - newPos[1] ) < 1e-7
    assert np.abs( pos[2] - newPos[2] ) < 1e-7


@pytest.mark.parametrize("posType", positionTypes)
def test_positionPropertySet(posType):
    dtype = coordinateType(posType)
    initPos = np.asarray( [1.3, 0.3251, -0.75], dtype=dtype)
    newPos = np.asarray( [-0.35, 0.251, 1.75], dtype=dtype)
    pos = posType( initPos )
    assert np.linalg.norm( np.asarray(pos) - initPos ) < 1e-7

    pos.x = newPos[0]
    assert np.abs( pos[0] - newPos[0] ) < 1e-7
    assert np.abs( pos[1] - initPos[1] ) < 1e-7
    assert np.abs( pos[2] - initPos[2] ) < 1e-7
    pos.y = newPos[1]
    assert np.abs( pos[0] - newPos[0] ) < 1e-7
    assert np.abs( pos[1] - newPos[1] ) < 1e-7
    assert np.abs( pos[2] - initPos[2] ) < 1e-7
    pos.z = newPos[2]
    assert np.abs( pos[0] - newPos[0] ) < 1e-7
    assert np.abs( pos[1] - newPos[1] ) < 1e-7
    assert np.abs( pos[2] - newPos[2] ) < 1e-7


@pytest.mark.parametrize("posType", positionTypes)
def test_positionNorm(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [-0.35, 0.251, 1.75], dtype=dtype)
    pos = posType( posVec )
    assert np.abs( pos.norm() - np.linalg.norm(posVec)) < np.finfo(dtype).eps

@pytest.mark.parametrize("posType", positionTypes)
def test_positionNormSquare(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [-0.35, 0.251, 1.75], dtype=dtype)
    pos = posType( posVec )
    assert np.abs( pos.normSquare() - np.linalg.norm(posVec)**2) < 4*np.finfo(dtype).eps

@pytest.mark.parametrize("posType", positionTypes)
def test_positionNormalise(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [-0.35, 0.251, 1.75], dtype=dtype)
    pos = posType( posVec )
    pos.normalise()
    assert np.abs( pos.norm() - 1.0 ) < np.finfo(dtype).eps
    assert np.abs( np.linalg.norm(np.array(pos)) - 1.0 ) < np.finfo(dtype).eps
    refNormed = posVec / np.linalg.norm(posVec)
    assert np.linalg.norm(pos - refNormed) < np.finfo(dtype).eps

@pytest.mark.parametrize("posType", positionTypes)
def test_positionNormaliseZero(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [0,0,0], dtype=dtype)
    pos = posType( posVec )
    with pytest.raises(Exception):
        pos.normalise(silentDivideByZero=False)
    with pytest.raises(Exception):
        pos.normalise() # Default does the same

    pos = posType( posVec )
    pos.normalise(silentDivideByZero=True)
    assert not np.any( np.isfinite(np.array(pos)))

@pytest.mark.parametrize("posType", positionTypes)
def test_positionUnaryAdd(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos = posType( posVec )
    addVec = np.asarray( [0.5, 0.25, -0.333], dtype=dtype)
    addPos = posType(addVec)
    resVec = posVec + addVec
    pos += addPos
    assert np.linalg.norm(np.array(pos)-resVec) < np.finfo(dtype).eps

@pytest.mark.parametrize("posType", positionTypes)
def test_positionUnarySub(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos = posType( posVec )
    subVec = np.asarray( [0.5, 0.25, -0.333], dtype=dtype)
    subPos = posType(subVec)
    resVec = posVec - subVec
    pos -= subPos
    assert np.linalg.norm(np.array(pos)-resVec) < np.finfo(dtype).eps

@pytest.mark.parametrize("posType", positionTypes)
def test_positionUnaryScale(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos = posType( posVec )
    scale = 0.375
    resVec = scale * posVec
    pos *= scale
    assert np.linalg.norm(np.array(pos)-resVec) < np.finfo(dtype).eps

@pytest.mark.parametrize("posType", positionTypes)
def test_positionTranslate(posType):
    dtype = coordinateType(posType)
    posInit = np.array( [ 0.3, -0.25, 1.35 ], dtype=dtype)
    translationVec = np.array( [ 1, 0.25, -0.5 ], dtype=dtype)
    refPos = posInit + translationVec

    pos =posType(posInit)
    translatePos = posType(translationVec)
    pos.translate(translatePos)
    assert np.linalg.norm(np.array(pos)-refPos) < np.finfo(dtype).eps
    
@pytest.mark.parametrize("posType", positionTypes)
def test_positionRotate(posType):
    quatType = quaternionType(posType)
    dtype = coordinateType(posType)
    posInit = np.array( [ 0.3, -0.25, 1.35 ], dtype=dtype)
    rotYPR = np.deg2rad(np.array([15, 45, -30], dtype=dtype))
    rotQuat = quatType.fromYPR(rotYPR)
    refRot= Rotation.from_euler('ZYX', rotYPR)
    refPos = refRot.apply(posInit)
    pos = posType(posInit )
    pos.rotate(rotQuat)
    tol = 10.0*np.finfo(dtype).eps # Reasonable error limit
    assert np.linalg.norm(np.asarray(pos)-refPos) < tol

@pytest.mark.parametrize("posType", positionTypes)
def test_positionTransform(posType):
    quatType = quaternionType(posType)
    dtype = coordinateType(posType)
    posInit = np.array( [ 0.3, -0.25, 1.35 ], dtype=dtype)
    transVec = np.array([-0.7, 0.34, -12.3], dtype=dtype)
    transPos = posType(transVec)
    
    rotYPR = np.deg2rad(np.array([15, 45, -30], dtype=dtype))
    rotQuat = quatType.fromYPR(rotYPR)
    refRot= Rotation.from_euler('ZYX', rotYPR)
    refPos = refRot.apply(posInit) + transVec
    pos = posType(posInit )
    pos.transform(rotQuat, transPos)
    tol = 10.0*np.finfo(dtype).eps # Reasonable error limit
    assert np.linalg.norm(np.asarray(pos)-refPos) < tol


@pytest.mark.parametrize("posType", positionTypes)
def test_positionAddFunction(posType):
    dtype = coordinateType(posType)
    pos1Vec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos1 = posType( pos1Vec )
    pos2Vec = np.asarray( [0.5, 0.25, -0.333], dtype=dtype)
    pos2 = posType(pos2Vec)
    resVec = pos1Vec + pos2Vec
    resPos = pos1 + pos2
    assert np.linalg.norm(np.array(resPos)-resVec) < np.finfo(dtype).eps

@pytest.mark.parametrize("posType", positionTypes)
def test_positionSubFunction(posType):
    dtype = coordinateType(posType)
    pos1Vec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos1 = posType( pos1Vec )
    pos2Vec = np.asarray( [0.5, 0.25, -0.333], dtype=dtype)
    pos2 = posType(pos2Vec)
    resVec = pos1Vec - pos2Vec
    resPos = pos1 - pos2
    assert np.linalg.norm(np.array(resPos)-resVec) < np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionNegFunction(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos = posType( posVec )
    resVec = -posVec
    resPos = -pos
    assert np.linalg.norm(np.array(resPos)-resVec) < np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionMulScalarFunction(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos = posType( posVec )
    scale = 0.3754
    resVec = scale * posVec
    resPos = scale * pos
    assert np.linalg.norm(np.array(resPos)-resVec) < np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionDivScalarFunction(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos = posType( posVec )
    scale = 0.3754
    resVec = posVec / scale
    resPos = pos / scale
    assert np.linalg.norm(np.array(resPos)-resVec) < np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionAngleFunction(posType):
    dtype = coordinateType(posType)
    pos1Vec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos1 = posType( pos1Vec )
    pos2Vec = np.asarray( [0.5, 0.25, -0.333], dtype=dtype)
    pos2 = posType(pos2Vec)
    refAngle = np.arccos(np.dot(pos1Vec,pos2Vec)/
                         (np.linalg.norm(pos1Vec)*np.linalg.norm(pos2Vec)))
    angle = rbbl.angle( pos1, pos2 )
    assert np.abs(angle-refAngle) < 4 * np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionAngleNormalisedFunction(posType):
    dtype = coordinateType(posType)
    pos1Vec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos1VecNormed = pos1Vec / np.linalg.norm(pos1Vec)
    pos1 = posType( pos1VecNormed )
    pos2Vec = np.asarray( [0.5, 0.25, -0.333], dtype=dtype)
    pos2VecNormed = pos2Vec / np.linalg.norm(pos2Vec)
    pos2 = posType(pos2VecNormed)
    refAngle = np.arccos(np.dot(pos1VecNormed,
                                pos2VecNormed))
    angle = rbbl.angleNormalised( pos1, pos2 )
    assert np.abs(angle-refAngle) < 4 * np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionDotFunction(posType):
    dtype = coordinateType(posType)
    pos1Vec = np.asarray( [1.0, -0.5, 0.25], dtype=dtype)
    pos1 = posType( pos1Vec )
    pos2Vec = np.asarray( [0.5, 0.25, -0.333], dtype=dtype)
    pos2 = posType(pos2Vec)
    refDot = np.dot(pos1Vec,pos2Vec)
    dot = rbbl.dot( pos1, pos2 )
    assert np.abs(dot-refDot) < 4 * np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionNormaliseFunction(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [-0.35, 0.251, 1.75], dtype=dtype)
    pos = posType( posVec )
    posNorm = rbbl.normalise(pos)
    assert np.abs( posNorm.norm() - 1.0 ) < np.finfo(dtype).eps
    assert np.abs( np.linalg.norm(np.array(posNorm)) - 1.0 ) < 4 * np.finfo(dtype).eps
    refNormed = posVec / np.linalg.norm(posVec)
    assert np.linalg.norm(np.array(posNorm) - refNormed) < 4 * np.finfo(dtype).eps


@pytest.mark.parametrize("posType", positionTypes)
def test_positionNormaliseFunctionZero(posType):
    dtype = coordinateType(posType)
    posVec = np.asarray( [0,0,0], dtype=dtype)
    pos = posType( posVec )
    with pytest.raises(Exception):
        rbbl.normalise( pos, silentDivideByZero=False)
    with pytest.raises(Exception):
        rbbl.normalise(pos) # Default does the same

    posNormed = rbbl.normalise(pos, silentDivideByZero=True)
    assert not np.any( np.isfinite(np.array(posNormed)))


# Note there's no free function 'translate', operator+ performs this op.

    
@pytest.mark.parametrize("posType", positionTypes)
def test_positionRotateFunction(posType):
    quatType = quaternionType(posType)
    dtype = coordinateType(posType)
    posInit = np.array( [ 0.3, -0.25, 1.35 ], dtype=dtype)
    rotYPR = np.deg2rad(np.array([15, 45, -30], dtype=dtype))
    rotQuat = quatType.fromYPR(rotYPR)
    refRot= Rotation.from_euler('ZYX', rotYPR)
    refPos = refRot.apply(posInit)
    pos = posType(posInit )
    posNew = rbbl.rotate(pos, rotQuat)
    tol = 10.0*np.finfo(dtype).eps # Reasonable error limit
    assert np.linalg.norm(np.asarray(posNew)-refPos) < tol


@pytest.mark.parametrize("posType", positionTypes)
def test_positionTransformFunction(posType):
    quatType = quaternionType(posType)
    dtype = coordinateType(posType)
    posInit = np.array( [ 0.3, -0.25, 1.35 ], dtype=dtype)
    transVec = np.array([-0.7, 0.34, -12.3], dtype=dtype)
    transPos = posType(transVec)
    
    rotYPR = np.deg2rad(np.array([15, 45, -30], dtype=dtype))
    rotQuat = quatType.fromYPR(rotYPR)
    refRot= Rotation.from_euler('ZYX', rotYPR)
    refPos = refRot.apply(posInit) + transVec
    pos = posType(posInit )
    posNew = rbbl.transform(pos, rotQuat, transPos)
    tol = 10.0*np.finfo(dtype).eps # Reasonable error limit
    assert np.linalg.norm(np.asarray(posNew)-refPos) < tol


@pytest.mark.parametrize("posType", positionTypes)
def test_positionSphericalInterpolationNormalised(posType, plot=False):
    dtype = coordinateType(posType)
    pos0Ref = np.array( [0.2, 0.7, -0.5], dtype=dtype )
    pos1Ref = np.array( [0.2, -0.5, 0.3], dtype=dtype )
    pos0Ref /= np.linalg.norm(pos0Ref)
    pos1Ref /= np.linalg.norm(pos1Ref)
    pos0 = posType(pos0Ref)
    pos1 = posType(pos1Ref)

    numVals = 101
    t = np.linspace( 0.0, 1.0, numVals, endpoint=True)
    # Requitres SciPy >= 1.5
    # refOut = ssp.geometric_slerp( pos0Ref, pos1Ref, t )

    out = np.full((numVals,3), fill_value=np.NaN, dtype=dtype)
    for idx in range(numVals):
        interpOut = rbbl.interpolateSpherical(pos0, pos1, t[idx])
        out[idx,:] = np.asarray(interpOut)

    if plot:
        import matplotlib.pyplot as plt
        from mpl_toolkits.mplot3d import proj3d
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        # PLot the unit sphere
        u = np.linspace(0, 2 * np.pi, 100)
        v = np.linspace(0, np.pi, 100)
        x = np.outer(np.cos(u), np.sin(v))
        y = np.outer(np.sin(u), np.sin(v))
        z = np.outer(np.ones(np.size(u)), np.cos(v))
        ax.quiver( 0, 0, 0, pos0Ref[0], pos0Ref[1], pos0Ref[2], color='b')
        ax.quiver( 0, 0, 0, pos1Ref[0], pos1Ref[1], pos1Ref[2], color='r')
        ax.plot_surface(x, y, z, color='y', alpha=0.1)
        ax.plot(out[...,0],
                out[...,1],
                out[...,2],
                c='k')
        plt.title('Spherical interpolation (unit vectors).')
        plt.show()

    # diff = np.linalg.norm(out - refOut)
    # assert np.all( diff < 1e-7)

@pytest.mark.parametrize("posType", positionTypes)
def test_positionSphericalInterpolation(posType, plot=False):
    dtype = coordinateType(posType)
    pos0Ref = np.array( [0.2, 0.7, -0.5], dtype=dtype )
    pos1Ref = np.array( [0.2, -0.5, 0.3], dtype=dtype )
    pos0Ref *= 0.75 / np.linalg.norm(pos0Ref)
    pos1Ref /= np.linalg.norm(pos1Ref)
    pos0 = posType(pos0Ref)
    pos1 = posType(pos1Ref)

    numVals = 101
    t = np.linspace( 0.0, 1.0, numVals, endpoint=True)
    # Requitres SciPy >= 1.5
    # refOut = ssp.geometric_slerp( pos0Ref, pos1Ref, t )

    out = np.full((numVals,3), fill_value=np.NaN, dtype=np.dtype)
    for idx in range(numVals):
        interpOut = rbbl.interpolateSpherical(pos0, pos1, t[idx])
        out[idx,:] = np.asarray(interpOut)

    if plot:
        import matplotlib.pyplot as plt
        from mpl_toolkits.mplot3d import proj3d
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        # PLot the unit sphere
        u = np.linspace(0, 2 * np.pi, 100)
        v = np.linspace(0, np.pi, 100)
        x = np.outer(np.cos(u), np.sin(v))
        y = np.outer(np.sin(u), np.sin(v))
        z = np.outer(np.ones(np.size(u)), np.cos(v))
        ax.quiver( 0, 0, 0, pos0Ref[0], pos0Ref[1], pos0Ref[2], color='b')
        ax.quiver( 0, 0, 0, pos1Ref[0], pos1Ref[1], pos1Ref[2], color='r')
        ax.plot_surface(x, y, z, color='y', alpha=0.1)
        ax.plot(out[...,0],
                out[...,1],
                out[...,2],
                c='k')
        plt.title('Spherical interpolation (unnormalised).')
        plt.show()


if __name__ == "__main__":
    for posType in positionTypes:
        test_positionDefaultInit(posType)
        test_positionInit(posType)
        test_positionKeywordInit(posType)
        test_positionVecInit(posType)
        test_positionIndexAccess(posType)
        test_positionElementAccess(posType)
        test_positionIndexedSet(posType)
        test_positionPropertySet(posType) 
        test_positionNorm(posType)
        test_positionNormSquare(posType)
        test_positionNormalise(posType)
        test_positionNormaliseZero(posType)
        test_positionUnaryAdd(posType)
        test_positionUnarySub(posType)
        test_positionUnaryScale(posType)
        test_positionTranslate(posType)
        test_positionRotate(posType)
        test_positionTransform(posType)
        test_positionRotateFunction(posType)
        test_positionTransformFunction(posType)
        test_positionAddFunction(posType)
        test_positionSubFunction(posType)
        test_positionNegFunction(posType)
        test_positionMulScalarFunction(posType)
        test_positionDivScalarFunction(posType)
        test_positionAngleFunction(posType)
        test_positionDotFunction(posType)
        test_positionAngleNormalisedFunction(posType)
        test_positionNormaliseFunction(posType)
        test_positionNormaliseFunctionZero(posType)
        test_positionSphericalInterpolationNormalised(posType, plot=True)
        test_positionSphericalInterpolation(posType, plot=True)
