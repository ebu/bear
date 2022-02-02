# Copyright Andreas Franck 2019 - All rights reserved.
# Copyright (c) 2019 Institute of Sound and Vibration Research, University of Southampton and VISR contributors --- All rights reserved.

import numpy as np

import pytest

import visr
import rbbl
import rcl
import pml
import rrl

def homogeniseQuaternions( q ):
    """
    Negate quaternions if necessary to make the first (w) component nonnegative.
    This is
    """
    return np.sign(q[:,0])[:,np.newaxis] * q

def test_parseListenerPositionNoOrientation():
    bs = 64
    fs = 48000
    cc = visr.SignalFlowContext( bs, fs )

    numOrientations = 1000
    positions = np.array(10*(np.random.random_sample( (numOrientations, 3) )-0.5),
                        dtype=np.float32)
    posRet = np.full( positions.shape, np.inf, positions.dtype )

    comp = rcl.PositionDecoder( context=cc, name="decoder", parent=None)
    flow = rrl.AudioSignalFlow( comp )
    msgIn = flow.parameterReceivePort( 'messageInput' )
    msgOut = flow.parameterSendPort( 'positionOutput' )

    for bi in range(numOrientations):
        posMsg = """{ "x": %f, "y": %f, "z": %f }"""\
            % tuple(positions[bi,:].tolist())

        msgIn.enqueue( pml.StringParameter( posMsg ))

        flow.process()

        if msgOut.changed():
            lp = msgOut.data()
            posRet[bi,:] = lp.position
            msgOut.resetChanged()
        else:
            print( "Did not receive a listener position in iteration %i" % bi )

    maxErr = np.max( np.linalg.norm( positions - posRet, axis=-1 ) )
    assert maxErr < 1e-6, "Difference between and output position exceeds tolerance."

def test_parseListenerPositionOrientationYPR():
    bs = 64
    fs = 48000
    cc = visr.SignalFlowContext( bs, fs )

    numOrientations = 1000
    positions = np.array(10*(np.random.random_sample( (numOrientations, 3) )-0.5),
                        dtype=np.float32)
    orientationsYPR = np.array(2*np.pi*np.random.random_sample( (numOrientations, 3)),
                            dtype=np.float32)
    # Store and compare them as quaternions, because the same orientation could
    # be represented by different Euler angle triplets
    orientationsQuat = np.asarray([rbbl.Quaternion.fromYPR(o).data
                                   for o in orientationsYPR] )
    orientationsQuat = homogeniseQuaternions( orientationsQuat )

    posRet = np.full( positions.shape, np.inf, positions.dtype )
    orRet = np.full( orientationsQuat.shape, np.inf, positions.dtype )

    comp = rcl.PositionDecoder( context=cc, name="decoder", parent=None)
    flow = rrl.AudioSignalFlow( comp )
    msgIn = flow.parameterReceivePort( 'messageInput' )
    msgOut = flow.parameterSendPort( 'positionOutput' )

    for bi in range(numOrientations):
        posMsg = """{ "x": %f, "y": %f, "z": %f,\
            "orientation": { "yaw": %f, "pitch": %f, "roll": %f } }"""\
            % (tuple(positions[bi,:].tolist())
               +tuple(np.rad2deg(orientationsYPR[bi,:]).tolist()))
        msgIn.enqueue( pml.StringParameter( posMsg ))
        flow.process()
        if msgOut.changed():
            lp = msgOut.data()
            posRet[bi,:] = lp.position
            orRet[bi,:] = lp.orientationQuaternion.data
            msgOut.resetChanged()
        else:
            print( "Did not receive a listener position in iteration %i" % bi )

    orRet = homogeniseQuaternions( orRet )

    maxPosErr = np.max( np.linalg.norm( positions - posRet, axis=-1 ) )
    assert maxPosErr < 1e-6, "Difference between and output position."
    maxOrErr = np.max( np.linalg.norm( orientationsQuat - orRet, axis=-1 ) )
    assert maxOrErr < 1e-6, "Difference between and output position."

def test_parseListenerPositionTranslation():
    bs = 64
    fs = 48000
    cc = visr.SignalFlowContext( bs, fs )

    coordTranslation = np.asarray( [0.328, -3.786, 0.15386] )
    coordTranslationPos = rbbl.Position3D(coordTranslation)

    numOrientations = 1000
    positions = np.array(10*(np.random.random_sample( (numOrientations, 3) )-0.5),
                        dtype=np.float32)
    positionsRef = positions + coordTranslation[np.newaxis,:]
    orientationsYPR = np.array(2*np.pi*np.random.random_sample( (numOrientations, 3)),
                            dtype=np.float32)
    orientationsQuat = np.asarray([rbbl.Quaternion.fromYPR(o).data
                                   for o in orientationsYPR] )
    orientationsQuat = homogeniseQuaternions( orientationsQuat )

    posRet = np.full( positions.shape, np.inf, positions.dtype )
    orRet = np.full( orientationsQuat.shape, np.inf, positions.dtype )

    comp = rcl.PositionDecoder(context=cc, name="decoder", parent=None,
                               positionOffset=coordTranslationPos)
    flow = rrl.AudioSignalFlow( comp )
    msgIn = flow.parameterReceivePort( 'messageInput' )
    msgOut = flow.parameterSendPort( 'positionOutput' )

    for bi in range(numOrientations):
        posMsg = """{ "x": %f, "y": %f, "z": %f,\
            "orientation": { "yaw": %f, "pitch": %f, "roll": %f } }"""\
            % (tuple(positions[bi,:].tolist())
               +tuple(np.rad2deg(orientationsYPR[bi,:]).tolist()))
        msgIn.enqueue( pml.StringParameter( posMsg ))
        flow.process()
        if msgOut.changed():
            lp = msgOut.data()
            posRet[bi,:] = lp.position
            orRet[bi,:] = lp.orientationQuaternion.data
            msgOut.resetChanged()
        else:
            print( "Did not receive a listener position in iteration %i" % bi )

    orRet = homogeniseQuaternions( orRet )

    maxPosErr = np.max( np.linalg.norm( positionsRef - posRet, axis=-1 ) )
    # Tolerance needs some tuning, since there's apparently some numerical error in the translation.
    assert maxPosErr < 5e-6, "Difference between and output position."
    # Orientation should be unchanged
    maxOrErr = np.max( np.linalg.norm( orientationsQuat - orRet, axis=-1 ) )
    assert maxOrErr < 1e-6, "Difference between and output position."


def test_parseListenerPositionTransformation():

    sptr=pytest.importorskip("scipy.spatial.transform",
      reason="Package scipy.spatial.transform not installed.")

    bs = 64
    fs = 48000
    cc = visr.SignalFlowContext( bs, fs )

    coordTranslation = np.asarray( [0.5, -0.1, 0.3])
    coordTranslationPos = rbbl.Position3D(coordTranslation)

    rotationYPRdeg = np.asarray([30, -45, 70])
    rotationYPR = np.deg2rad(rotationYPRdeg)
    # rotationQuat = pml.ypr2Quaternion(rotationYPR)
    rotationRef = sptr.Rotation.from_euler('ZYX', rotationYPR )

    numOrientations = 1000
    positions = np.array(10*(np.random.random_sample( (numOrientations, 3) )-0.5),
                        dtype=np.float32)
    positionsRef = rotationRef.apply(positions) + coordTranslation[np.newaxis,:]
    orientationsYPR = np.hstack((2*np.pi*(np.random.random_sample((numOrientations,3))-0.5),
                                 np.zeros((numOrientations,0))))

    orientationsQuat = np.asarray([rbbl.Quaternion.fromYPR(o).data
                                  for o in orientationsYPR] )
    orientationsQuat = homogeniseQuaternions( orientationsQuat )

    # Compute the reference orientation (after transformation in the decoder)
    # We try to compute it as differently as possible to the internal
    # transformation.
    # orRefYPR = np.asarray([(Rotation.from_euler('ZYX', ypr )*rotationRef).as_euler('ZYX')
    #             for ypr in orientationsYPR])
    orRefYPR = np.asarray([(rotationRef*sptr.Rotation.from_euler('ZYX', ypr )).as_euler('ZYX')
                for ypr in orientationsYPR])

    orRefQuat = np.asarray([rbbl.Quaternion.fromYPR(ypr).data
      for ypr in orRefYPR] )
    # Quaternions with opposite signs denote equal rotations. To resolve this ambiguity,
    # adapt the sign such that the w component is positive.

    orRefQuat = homogeniseQuaternions( orRefQuat )

    posRet = np.full( positions.shape, np.inf, positions.dtype )
    orRet = np.full( orientationsQuat.shape, np.inf, positions.dtype )

    comp = rcl.PositionDecoder(context=cc, name="decoder", parent=None,
                               positionOffset=coordTranslationPos,
                               orientation=rotationYPR )
    flow = rrl.AudioSignalFlow( comp )
    msgIn = flow.parameterReceivePort( 'messageInput' )
    msgOut = flow.parameterSendPort( 'positionOutput' )

    for bi in range(numOrientations):
        posMsg = """{ "x": %f, "y": %f, "z": %f,\
            "orientation": { "yaw": %f, "pitch": %f, "roll": %f } }"""\
            % (tuple(positions[bi,:].tolist())
               +tuple(np.rad2deg(orientationsYPR[bi,:]).tolist()))
        msgIn.enqueue( pml.StringParameter( posMsg ))
        flow.process()
        if msgOut.changed():
            lp = msgOut.data()
            posRet[bi,:] = lp.position
            orRet[bi,:] = lp.orientationQuaternion.data
            msgOut.resetChanged()
        else:
            print( "Did not receive a listener position in iteration %i" % bi )

    orRet = homogeniseQuaternions( orRet )

    # For debugging only
    # orRetYPR= np.asarray([ pml.yprFromQuaternion(pml.ListenerPosition.OrientationQuaternion( *quat.tolist() ))
    #                       for quat in orRet ] )

    maxPosErr = np.max( np.linalg.norm( positionsRef - posRet, axis=-1 ) )
    # Tolerance needs some tuning, since there's apparently some numerical error in the translation.
    assert maxPosErr < 5e-6, "Difference between reference and output position."
    maxOrErr = np.max( np.linalg.norm( orRefQuat - orRet, axis=-1 ) )
    assert maxOrErr < 5e-6, "Difference between reference and output orientation."


# Enable this file to run as a script (in addition to being used as a pytest unit test.)
if __name__ == "__main__":
    test_parseListenerPositionNoOrientation()
    test_parseListenerPositionOrientationYPR()
    test_parseListenerPositionTranslation()
    test_parseListenerPositionTransformation()
