
import pytest

import visr
import rcl
import pml
import rrl

import numpy as np

def test_rampGain(plot=False):
    fs=48000
    bs=64
    interpolationLength=2*bs
    numIns=12
    numOuts= 8

    activeIn=3
    activeOut=1

    ctxt=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    gains=np.zeros((numOuts, numIns), dtype=np.float32)

    comp=rcl.GainMatrix(ctxt, "GainMatrix", None,
      numIns, numOuts, interpolationSteps=interpolationLength,
      initialGains=gains, controlInput=True)

    flow = rrl.AudioSignalFlow( comp )
    gainInput = flow.parameterReceivePort("gainInput")

    numBlocks=16
    signalLength=numBlocks*bs
    updatePeriodBlocks=3
    inSig=np.asarray(2.0*np.random.rand( numIns, signalLength )-1,
                     dtype=np.float32)
    inSig[activeIn,:]=0.75*np.sin( 2*np.pi*880/fs*np.arange(signalLength))
    outSig = np.zeros( (numOuts, signalLength), dtype=np.float32)

    numUpdates = numBlocks // updatePeriodBlocks + 2
    gainParams = np.sin( np.arange(numUpdates) )

    for bIdx in range(numBlocks):
        if bIdx % updatePeriodBlocks == 0:
            gainInput.data()[activeOut, activeIn]=gainParams[bIdx//updatePeriodBlocks+1]
        outSig[:,bIdx*bs:(bIdx+1)*bs] = flow.process(inSig[:,bIdx*bs:(bIdx+1)*bs])

    print( np.max(np.abs(outSig), axis=1))

    if plot:
        import matplotlib.pyplot as plt

        fig, ax = plt.subplots(1,1)
        ax.plot( outSig[activeOut,:], 'b.-')

if __name__ == "__main__":
    test_rampGain(plot=True)
