
import visr
import rcl
import pml
import rrl
import rbbl

import numpy as np

def test_startupSingleChannel(plot=False):
    fs=48000
    bs=64
    interpolationLength=2*bs
    filterLength = 1024
    numInterpolants = 3
    numIns=1
    numOuts= 1

    activeIn=0
    activeOut=0

    numBlocks = 32
    signalLength=numBlocks*bs
    inSig=np.zeros((numIns,signalLength), dtype=np.float32)
    inSig[activeIn,::50]=1.0
    outSig = np.full( (numOuts, signalLength), np.NaN, dtype=np.float32)

    filters = np.zeros( (numInterpolants,filterLength), dtype=np.float32 )
    filters[0, 10] = 1.0
    filters[1, 20] = 1.0
    filters[2, 30] = 1.0

    interpWeights = np.asarray([0.5, -0.3, 0.2 ])
    refFilter = np.einsum( 'j,jk->k', interpWeights, filters )

    interpParameter = rbbl.InterpolationParameter( id=0, indices=[0,1,2],
                                                  weights=interpWeights )
    interpParameterSet = rbbl.InterpolationParameterSet(
        [ interpParameter ] )

    routings = rbbl.FilterRoutingList([rbbl.FilterRouting(0, 0, 0, 1.0)])

    refOutput= np.convolve(refFilter, inSig[activeOut,:],
                            mode='full' )[:signalLength]

    ctxt=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    comp=rcl.InterpolatingFirFilterMatrix(ctxt, "Convolver", None,
      numberOfInputs=numIns,
      numberOfOutputs=numOuts,
      filterLength=filterLength,
      maxRoutings=1,
      maxFilters=3,
      numberOfInterpolants=3,
      transitionSamples=interpolationLength,
      interpolants=interpParameterSet,
      routings=routings,
      filters=filters)

    flow = rrl.AudioSignalFlow( comp )

    for bIdx in range(numBlocks):
        outSig[:,bIdx*bs:(bIdx+1)*bs] = flow.process(inSig[:,bIdx*bs:(bIdx+1)*bs])

    if plot:
        import matplotlib.pyplot as plt
        fig, ax = plt.subplots(1,1)
        ax.plot(refOutput, 'bo-', label='Reference')
        ax.plot(outSig[activeOut,:], 'm.-', label='Rendered')
        ax.legend()

    assert np.all(np.abs(outSig[activeOut,:] - refOutput )
                  <= 5*np.finfo(np.float32).eps)

# Enable to run the unit test as a script.
if __name__ == "__main__":
    test_startupSingleChannel(plot=True)
