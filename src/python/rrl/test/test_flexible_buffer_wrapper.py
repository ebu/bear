
import numpy as np

import visr
import pml
import rrl


class CompositeFeedthrough( visr.CompositeComponent ):
    def __init__(self, context, name, parent, numChannels):
        super().__init__(context, name, parent)
        self._input = visr.AudioInputFloat("in", self, numChannels)
        self._output = visr.AudioOutputFloat("out", self, numChannels)
        self.audioConnection(self._input, self._output)

def test_compositeFeedthrough(plot=False):
    fs=48000
    period=64
    numChannels=2

    numBlocks=64

    # fSig = np.random.random( numChannels ) * 1200

    fSig = np.array( [440, 880] )

    # blockSizes = np.random.randint( 1, 4*period, numBlocks )
    blockSizes = np.full( (numBlocks), 100, dtype=np.uint32 )

    sigLen=int(np.sum(blockSizes))

    t = np.arange( sigLen) / fs
    inSig=np.asarray(np.cos( fSig[:,np.newaxis] *2*np.pi * t[np.newaxis,:] ),
                     dtype=np.float32)
    # inSig[0,:] = np.arange( sigLen )
    outSig = np.full( (numChannels, sigLen), np.NaN, dtype=np.float32)

    context=visr.SignalFlowContext(period=period, samplingFrequency=fs)

    comp=CompositeFeedthrough(context, "CC", None, numChannels)
    flow = rrl.AudioSignalFlow( comp )
    bufferWrapper = rrl.FlexibleBufferWrapper( flow )

    currIdx = 0
    for bs in blockSizes:
        endIdx = currIdx + bs
        inChunk = inSig[:,currIdx:endIdx]
        outChunk = bufferWrapper.process( inChunk )
        assert outChunk.shape[1] == bs
        outSig[:,currIdx:endIdx] = outChunk
        currIdx = endIdx
    assert currIdx == sigLen

    if plot:
        import matplotlib.pyplot as plt
        plt.figure(1)
        plt.plot( inSig[0,:], 'bo-' )
        plt.plot( outSig[0,period:], 'm.-' )

    assert np.all(np.abs(outSig[:,period:] - inSig[:,:-period] )
                  <= 5*np.finfo(np.float32).eps)


# Enable to run the unit test as a script.
if __name__ == "__main__":
    test_compositeFeedthrough(plot=True)
