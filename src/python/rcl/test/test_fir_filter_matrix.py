# Copyright Andreas Franck 2019 - All rights reserved.
# Copyright (c) 2019 Institute of Sound and Vibration Research, University of Southampton and VISR contributors --- All rights reserved.

import numpy as np

import efl
import visr
import rcl
import rbbl
import rrl

def test_FirFilterMatrixInit():

    assert efl.initialiseLibrary(), "Initialisation of vector function library failed."

    bs = 64
    fs = 48000

    numBlocks = 8

    sigLen = bs * numBlocks

    cc = visr.SignalFlowContext( bs, fs )

    numInputs = 4
    numOutputs = 3
    firLength = 128
    numFilters = 2

    filterMtx = np.zeros( (numFilters, firLength ), dtype=np.float32 )

    filterMtx[0,:10] = np.linspace( 1,0, 10, endpoint=False )

    routings = rbbl.FilterRoutingList( [
        rbbl.FilterRouting(inputIndex=0,outputIndex=0, filterIndex=0),
        rbbl.FilterRouting(inputIndex=1,outputIndex=1, filterIndex=1)
        ] )

    print( "Hello FIR!" )

    comp = rcl.FirFilterMatrix( cc, "FIR", None,
                                numberOfInputs = numInputs,
                                numberOfOutputs = numOutputs,
                                routings = routings,
                                filterLength = firLength,
                                maxFilters = filterMtx.shape[0],
                                maxRoutings = numInputs * numOutputs,
                                filters = filterMtx
                                )

    flow = rrl.AudioSignalFlow( comp )

    inputSig = np.zeros( (numInputs, sigLen), dtype=np.float32 )
    inputSig[:,0] = -0.5

    outputSig = np.zeros( (numOutputs, sigLen), dtype=np.float32 )

    for bi in range(numBlocks):
        outputSig[ :, bi*bs:(bi+1)*bs] = flow.process( inputSig[ :, bi*bs:(bi+1)*bs] )

    ref = np.convolve( inputSig[0,:], filterMtx[0,:], mode ='full' )[:sigLen]

    maxErr = np.max( np.abs(outputSig[0,:] - ref ) )
    assert( maxErr < 1e-6 )

if __name__ == "__main__":
    test_FirFilterMatrixInit()
