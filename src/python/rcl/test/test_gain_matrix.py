
import visr
import rcl
import pml
import rrl

import numpy as np

def gainRampReference(inputSignal, gains, blockSize, interpolationSteps):
    sigLen=inputSignal.shape[-1]
    assert sigLen%blockSize==0
    numBlocks=sigLen//blockSize
    assert gains.shape[-1]==numBlocks+1
    outputSignal=np.zeros(inputSignal.shape, dtype=inputSignal.dtype)
    # The ramp starts at 1/interpolationsteps and ends at 1.
    ramp = np.flip(np.linspace(1.0, 0.0, interpolationSteps, endpoint=False,
                           dtype=inputSignal.dtype))
    if interpolationSteps>=blockSize:
        ramp=ramp[:blockSize]
    else:
        ramp = np.concatenate( (ramp, np.ones((blockSize-interpolationSteps))))
    startGain=gains[...,0]
    endGain=ramp[-1] # Start value for the next iteration
    for bIdx in range(numBlocks):
        slope=startGain+ramp*(gains[...,bIdx+1]-startGain)
        outputSignal[...,bIdx*blockSize:(bIdx+1)*blockSize]=\
            slope*inputSignal[...,bIdx*blockSize:(bIdx+1)*blockSize]
        startGain=startGain+endGain*(gains[...,bIdx+1]-startGain)
    return outputSignal

def test_rampGain():    
    fs=48000
    bs=64
    interpolationLength=5*bs
    numIns=1
    numOuts= 1

    activeIn=0
    activeOut=0

    numUpdates=200
    updatePeriodBlocks=1
    numBlocks=numUpdates*updatePeriodBlocks
    signalLength=numBlocks*bs
    inSig=np.asarray(2.0*np.random.rand( numIns, signalLength )-1,
                     dtype=np.float32)
    # inSig[activeIn,:]=0.75*np.sin( 2*np.pi*880/fs*np.arange(signalLength))
    inSig[activeIn,:]=1.0
    outSig = np.zeros( (numOuts, signalLength), dtype=np.float32)

    #gainParams = np.sin( np.arange(numUpdates+1) )
    gainParams=np.zeros( numUpdates+1, dtype=np.float32 )
    gainParams[1:10]=1.0

    refOutput=gainRampReference(inSig[activeIn,:],
                                gainParams,
                                bs*updatePeriodBlocks, interpolationLength,
                                )

    ctxt=visr.SignalFlowContext(period=bs, samplingFrequency=fs)


    initialGains=np.zeros((numOuts, numIns), dtype=np.float32)
    initialGains[activeOut, activeIn] = gainParams[0]

    comp=rcl.GainMatrix(ctxt, "GainMatrix", None,
      numIns, numOuts, interpolationSteps=interpolationLength,
      initialGains=initialGains, controlInput=True)

    flow = rrl.AudioSignalFlow( comp )
    gainInput = flow.parameterReceivePort("gainInput")

    for bIdx in range(numBlocks):
        if bIdx % updatePeriodBlocks == 0:
            gainInput.data()[activeOut, activeIn]=gainParams[bIdx//updatePeriodBlocks+1]
        outSig[:,bIdx*bs:(bIdx+1)*bs] = flow.process(inSig[:,bIdx*bs:(bIdx+1)*bs])

    # fig, ax = plt.subplots(1,1)
    # ax.plot( inSig[activeIn,:], 'g-' )
    # ax.plot( refOutput, 'bo-' )
    # ax.plot( outSig[activeOut,:], 'm.-')

    # fig2, ax2 = plt.subplots(1,1)
    # ax2.plot( outSig[activeOut,:]-refOutput, 'r.-')
    # fig2.suptitle('Error (output-reference)')

    # fig3, ax3 = plt.subplots(1,1)
    # ax3.plot( np.diff(refOutput), 'bo-', label='ref' )
    # ax3.plot( np.diff(outSig[activeOut,:]), 'm.-', label='output')
    # fig3.suptitle( 'Finite differences' )
    # ax3.legend()
    # plt.show(block=True)

    assert np.all(np.abs(outSig[activeOut,:] - refOutput )
                  <= 5*np.finfo(np.float32).eps)

# Enable to run the unit test as a script.
if __name__ == "__main__":
    test_rampGain()
