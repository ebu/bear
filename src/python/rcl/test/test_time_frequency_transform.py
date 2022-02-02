
import visr
import rcl
import pml
import rrl

import numpy as np
from scipy import signal

def test_singleChannel(plot=False):
    fs=48000
    bs=256
    numSignals=3


    # fSig=np.array([440, 1871, 3234])
    # ampSig=np.array([ 0.3,0.125, 0.0375])
    fSig=np.array([ 1275, 0, 0])
    ampSig=np.array([1.0, 0, 0])

    numBlocks=128
    signalLength=numBlocks*bs

    t = 1/fs * np.arange(signalLength)

    inputSignal = np.asarray(0.2*(np.random.random_sample( (numSignals, signalLength))-0.5)
                             + ampSig[:,np.newaxis] * np.sin( 2*np.pi*fSig[:,np.newaxis]*t[np.newaxis,:]),
                             dtype=np.float32)

    dftLen=512
    hopSize=256
    window = signal.hann(dftLen, sym=False) # 'sym=False' needed for COLA condition.

    numOutBlocks=int(np.floor(signalLength/hopSize))
    numFdBins=int(np.ceil( (dftLen+1)/2 ))

    assert bs % hopSize == 0, "Period size must be an integer multiple of the hop size"
    dftBlocksPerPeriod=bs // hopSize

    outputData=np.full( (numSignals, numFdBins, numOutBlocks), complex(np.NaN), dtype=np.complex64)

    if not signal.check_COLA(window, dftLen, dftLen-hopSize):
        print( "Used window/overlap combination does not satisfy the COLA condition.")

    fBins, tBlock, refOutput = signal.stft(inputSignal, window=window, nperseg=dftLen,
                                           noverlap=dftLen-hopSize,
                                           boundary='zeros', return_onesided=True)

    cc = visr.SignalFlowContext(bs, fs)

    comp = rcl.TimeFrequencyTransform(cc, "TFT", None,
                                      numberOfChannels=numSignals,
                                      dftSize=dftLen, hopSize=hopSize,
                                      window=window,
                                      fftImplementation='kissfft',
                                      normalisation=rcl.TimeFrequencyTransform.Normalisation.One )

    flow=rrl.AudioSignalFlow(comp)
    tfOut = flow.parameterSendPort("out")

    for bIdx in range(numBlocks):
        flow.process( inputSignal[:,bs*bIdx:bs*(bIdx+1)])
        tfBlock=tfOut.data()

        assert tfBlock.numberOfDftBins == numFdBins
        assert tfBlock.numberOfChannels == numSignals
        assert tfBlock.numberOfFrames == dftBlocksPerPeriod

        outputData[:,:, bIdx*dftBlocksPerPeriod:(bIdx+1)*dftBlocksPerPeriod]=\
            np.asarray(tfBlock).transpose( (1,2,0) )

    sigIdx = 0
    blockIdx = 12
    # %%
    # Manual zero-padding
    if blockIdx * hopSize < dftLen:
        bl = np.concatenate( (np.zeros( dftLen-hopSize, np.float32),
                              inputSignal[sigIdx,:(blockIdx*hopSize)]) )
    else:
        endIdx = (blockIdx+1)*hopSize
        bl = inputSignal[sigIdx,endIdx-dftLen:endIdx]
    Bl = np.fft.rfft( bl * window )

    if(plot):
        import matplotlib.pyplot as plt
        plt.figure()
        plt.plot(fBins, np.abs(Bl), 'rs-', label='Ref')
        plt.plot(fBins, np.abs(outputData[sigIdx,:,blockIdx]), 'm.-', label='VISR TFT')
        plt.legend()

    assert np.linalg.norm(Bl - outputData[sigIdx,:,blockIdx]) < 5e-5


# Enable to run the unit test as a script.
if __name__ == "__main__":
    test_singleChannel(plot=True)
