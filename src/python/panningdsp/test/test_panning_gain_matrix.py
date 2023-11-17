
import visr
import panningdsp
import rrl

from operator import itemgetter
import numpy as np

def referenceGainScalar( sigLen, initialGain, transitions ):
    """
    Internal function to compute the gain trajectory for an arbitra
    """
    out = np.full(sigLen, initialGain, dtype=np.float32)
    currGain = initialGain
    for trans in transitions:
        start = trans["s"]
        duration = trans["d"]
        gain = trans["g"]
        if start >= sigLen:
            break
        gainRamp = np.flip(np.linspace(gain, currGain,
                                       duration, endpoint=False))
        remLen = sigLen - start
        if remLen >= duration:
            out[start:start+duration] = gainRamp
            out[start+duration:] = gain
        else:
            out[-remLen:] = gainRamp[:remLen]
        currGain=gain

    return out

def plotPanningScalar( inSig, outSig, refSig, title, fig=None):
    fig, axs = plt.subplots(3,1)
    axs[0].plot( inSig, 'g-', label='input' )
    axs[0].plot( refSig, 'bo-', label='output' )
    axs[0].plot( outSig, 'm.-', label='reference' )

    axs[1].plot( outSig - refSig, 'r.-')
    axs[1].set_title('Error (output-reference)')

    axs[2].plot( np.diff(refSig), 'bo-', label='reference' )
    axs[2].plot( np.diff(outSig), 'm.-', label='output')
    axs[2].set_title( 'Finite differences' )
    axs[2].legend()
    fig.suptitle( title )

def scalarPanning( signal, transitions=[], bs=64, fs=48000, initialGain=0.0 ):
    """
    Internal implementation function for applying a panning gain to a scalar
    signal with an arbitrary sequence of transition messages.
    """
    numIns=1
    numOuts=1

    activeIn=0
    activeOut=0

    inSig = np.asarray( np.atleast_2d( signal), dtype=np.float32)
    signalLength=inSig.shape[-1]
    numBlocks=signalLength // bs
    assert numBlocks*bs == signalLength
    outSig = np.zeros( (numOuts, signalLength), dtype=np.float32)

    gains = np.zeros((numOuts, numIns), dtype=np.float32)
    gains[activeOut, activeIn] = initialGain

    transitions.sort( key=itemgetter( 'i' ))

    refGain = referenceGainScalar( signalLength, initialGain, transitions )

    # Works only for scalar input
    refOutput = refGain * inSig

    ctxt=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    comp=panningdsp.PanningGainMatrix(ctxt, "GainMatrix", None,
      numberOfObjects=numIns, numberOfLoudspeakers=numOuts,
      initialGains=gains )

    flow = rrl.AudioSignalFlow( comp )
    gainInput = flow.parameterReceivePort("gainInput")

    for bIdx in range(numBlocks):
        param = gainInput.data()
        for t in [ p for p in transitions if p['i'] == bIdx]:
            param.timeStamps = [t['s']]
            param.transitionTimes = [t['d']]
            np.asarray(param.gains)[activeOut,activeIn] = t['g']
            gainInput.swapBuffers()
        outSig[:,bIdx*bs:(bIdx+1)*bs] = flow.process(inSig[:,bIdx*bs:(bIdx+1)*bs])

    return outSig[activeOut,:], refOutput[activeOut,:]


def test_constantGain( plot=False ):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.sin( 2*np.pi*880/fs*np.arange(signalLength))
    out, ref = scalarPanning(inSig, transitions=[], bs=bs, fs=fs,
                             initialGain=0.35)

    if plot:
        plotPanningScalar(inSig, out, ref, title="Constant gain")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)

def test_rampGain(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    endGain = 0.725
    transitionTime = 17
    transitionStartTime = 137
    parameterSendBlock = 1
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Ramp gain")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_rampGainCrossBlockBoundary(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    endGain = 0.725
    transitionTime = 139
    transitionStartTime = 137
    parameterSendBlock = 1
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Ramp gain crossing block boundary")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_rampGainStartAtBlockBoundary(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    endGain = 0.725
    transitionTime = 2*bs
    transitionStartTime = 137
    parameterSendBlock = 1
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Ramp starting at block boundary")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_rampGainEndAtBlockBoundary(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    endGain = 0.725
    transitionTime = 34
    transitionStartTime = 2*bs - transitionTime
    parameterSendBlock = 1
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Ramp endig at block boundary")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_multipleTransitionsSeparate(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    transitions = [ dict(i=0, s=13, d=75, g=1.0),
                   dict(i=2, s=143, d=75, g=0.5)]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Two non-overlappping transactions")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_multipleTransitionsSeparateSend2ndBefore1stFinished(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    transitions = [ dict(i=0, s=13, d=75, g=1.0),
                   dict(i=2, s=143, d=75, g=0.5)]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Two non-overlappping transactions, second is sent before first is finished.")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_multipleTransitionsAdjoining(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    start1 = 88
    duration1 = 75
    start2 = start1+duration1
    duration2 = 135
    transitions = [ dict(i=0, s=start1, d=duration1, g=1.0),
                   dict(i=2, s=start2, d=duration2, g=0.5)]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Two adjoining transactions")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_multipleTransitionsOverlapping(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    start1 = 88
    duration1 = 75
    start2 = start1+duration1+12
    duration2 = 135
    transitions = [ dict(i=0, s=start1, d=duration1, g=1.0),
                   dict(i=2, s=start2, d=duration2, g=0.5)]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Two overlappping transactions")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_jumpGain(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.7
    endGain = 0.4
    transitionTime = 0
    transitionStartTime = 137
    parameterSendBlock = 1
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Jump gain")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)

def test_rampGainImmediately(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    endGain = 0.725
    transitionTime = 17
    parameterSendBlock = 1
    transitionStartTime = parameterSendBlock * bs
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Ramp gain starting at current clock boundary.")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)

def test_rampGainImmediatelyStartTime0(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.2
    endGain = 0.725
    transitionTime = 17
    parameterSendBlock = 0
    transitionStartTime = parameterSendBlock * bs
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Ramp gain starting at t=0")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)


def test_jumpGainImmediately(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.7
    endGain = 0.4
    transitionTime = 0
    parameterSendBlock = 1
    transitionStartTime = parameterSendBlock * bs
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Jump gain at current time")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)

def test_jumpGainImmediatelyStartTime0(plot=False):
    fs=48000
    bs=64
    numBlocks=8
    signalLength=numBlocks*bs
    inSig=0.75*np.ones(signalLength)

    startGain = 0.7
    endGain = 0.4
    transitionTime = 0
    parameterSendBlock = 0
    transitionStartTime = parameterSendBlock * bs
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime, g=endGain) ]

    out, ref = scalarPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                             initialGain=startGain )
    if plot:
        plotPanningScalar(inSig, out, ref, title="Jump gain at t=0")

    assert np.all(np.abs(out - ref ) <= 5*np.finfo(np.float32).eps)

    # %% Multiple-input / multiple-output tests


def referenceGainMimo(sigLen, initialGains, transitions):
    numOuts, numIns= initialGains.shape

    # Note: this can yield rather large matrices
    refGains = np.repeat(initialGains[...,np.newaxis], sigLen, axis=-1)

    # We assume that the transitions are sorted.
    for trans in transitions:
        objIdx = trans["o"]
        start = trans["s"]
        duration = trans["d"]
        gains = trans["g"]
        if start >= sigLen:
            break

        startGains = refGains[objIdx,:,start]
        scaleRamp= np.flip(np.linspace(1.0, 0.0, duration, endpoint=False))
        gainRamp = startGains[:,np.newaxis]\
            + (gains-startGains)[:,np.newaxis]*scaleRamp[np.newaxis,:]

        remLen = sigLen - start
        if remLen >= duration:
            refGains[objIdx,:,start:start+duration] = gainRamp
            refGains[objIdx,:,start+duration:] = gains[...,np.newaxis]
        else:
            refGains[objIdx,:,-remLen:] = gainRamp[objIdx,:,:remLen]
    return refGains

def mimoPanning(signal, initialGains, transitions=[], bs=64, fs=48000):
    """
    Internal implementation function for applying a panning gain to a
    multichannel signal signal with an arbitrary sequence of transition messages.
    """

    initialGains=np.asarray(initialGains, dtype=np.float32)
    numObj, numLsp= initialGains.shape

    inSig = np.asarray( np.atleast_2d(signal), dtype=np.float32)
    assert inSig.shape[0] == numObj
    signalLength=inSig.shape[-1]
    numBlocks=signalLength // bs
    assert numBlocks*bs == signalLength
    outSig = np.zeros( (numLsp, signalLength), dtype=np.float32)

    transitions.sort( key=itemgetter( 'i' ))

    refGain = referenceGainMimo( signalLength, initialGains, transitions )

    refOutput = np.einsum( 'jik,jk->ik', refGain, inSig )

    ctxt=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    comp=panningdsp.PanningGainMatrix(ctxt, "GainMatrix", None,
      numberOfObjects=numObj, numberOfLoudspeakers=numLsp,
      initialGains=initialGains )

    flow = rrl.AudioSignalFlow( comp )
    gainInput = flow.parameterReceivePort("gainInput")

    for bIdx in range(numBlocks):
        param = gainInput.data()
        for t in [ p for p in transitions if p['i'] == bIdx]:
            objIdx=t['o']
            timeStamps = [panningdsp.timeStampInfinity]*numObj
            transitionTimes =[panningdsp.timeStampInfinity]*numObj
            timeStamps[objIdx] = t['s']
            transitionTimes[objIdx] = t['d']
            param.timeStamps = timeStamps
            param.transitionTimes = transitionTimes
            np.asarray(param.gains)[...] = np.NaN
            newGains = t['g']
            np.asarray(param.gains)[objIdx,:] = newGains
            gainInput.swapBuffers()
        outSig[:,bIdx*bs:(bIdx+1)*bs] = flow.process(inSig[:,bIdx*bs:(bIdx+1)*bs])

    return outSig, refOutput


def test_mimoStatic(plot=False):
    fs=48000
    bs=64
    numBlocks=8

    numObj=3
    numLsp=5

    sigLen=numBlocks*bs

    inSig=2*(np.random.random_sample((numObj, sigLen))-0.5)
    # inSig=np.zeros((numObj, sigLen), dtype=np.float32)
    # inSig[1,:] = 0.3

    initGains = np.random.random_sample((numObj, numLsp))
    # initGains=np.zeros((numObj, numLsp), dtype=np.float32)
    # initGains[0,0] = 0.5
    # initGains[1,3] = 0.2

    transitions = []

    out, ref = mimoPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                         initialGains=initGains )
    if plot:
        plotPanningScalar( inSig[0,:], out[0,:], ref[0,:], title="static MIMO")

    assert np.max(np.abs(out-ref)) < 1e-5

def test_mimoRampGainMultipleBlocks(plot=False):
    fs=48000
    bs=64
    numBlocks=8

    numObj=3
    numLsp=5

    sigLen=numBlocks*bs

    objIdx = 1

    inSig=2*(np.random.random_sample((numObj, sigLen))-0.5)
    # inSig=np.zeros((numObj, sigLen), dtype=np.float32)
    # inSig[objIdx,:] = 1

    initGains = np.random.random_sample((numObj, numLsp))
    # initGains=np.zeros((numObj, numLsp), dtype=np.float32)
    # initGains[0,0] = 0.5
    # initGains[1,3] = 0.2

    initGains[objIdx,:] = [0.5, 0.7, 0.1, 0.3, 1.0 ]

    newGains = np.array([0.7, 0.1, 0.3, 1.0, 0.2 ], dtype=np.float32 )
    # newGains = np.array([0, 0, 0.5, 0, 0 ], dtype=np.float32 )

    parameterSendBlock = 1
    transitionStartTime = 135
    transitionTime = 153
    assert parameterSendBlock*bs <= transitionStartTime # Causality rules!
    # newGainMtx = np.full( initGains.shape, np.NaN, dtype=np.float32)
    # newGainMtx[objIdx] = newGains
    transitions = [ dict(i=parameterSendBlock,
                         s=transitionStartTime, d=transitionTime,
                         o=objIdx, g=newGains )
                  ]

    out, ref = mimoPanning(inSig, transitions=transitions, bs=bs, fs=fs,
                         initialGains=initGains )
    if plot:
        plotPanningScalar( inSig[objIdx,:], out[2,:], ref[2,:], title="MIMO ramp gain.")

    assert np.max(np.abs(out-ref)) < 1e-5


# Enable to run the unit test as a script.
if __name__ == "__main__":
    plotData = True # Set to show plots
    if plotData:
        import matplotlib.pyplot as plt

    test_constantGain(plot=plotData)
    test_rampGain(plot=plotData)
    test_rampGainCrossBlockBoundary(plot=plotData)
    test_rampGainStartAtBlockBoundary(plot=plotData)
    test_rampGainEndAtBlockBoundary(plot=plotData)
    test_multipleTransitionsSeparate(plot=plotData)
    test_multipleTransitionsSeparateSend2ndBefore1stFinished(plot=plotData)
    test_multipleTransitionsAdjoining(plot=plotData)
    test_multipleTransitionsOverlapping(plot=plotData)
    test_jumpGain(plot=plotData)

    test_rampGainImmediately(plot=plotData)
    test_rampGainImmediatelyStartTime0(plot=plotData)
    test_jumpGainImmediately(plot=plotData)
    test_jumpGainImmediatelyStartTime0(plot=plotData)

    test_mimoStatic(plot=plotData)
    test_mimoRampGainMultipleBlocks(plot=plotData)