# -*- coding: utf-8 -*-
"""
Created on Wed Dec 27 08:02:00 2017

@author: andi
"""

import efl
import rbbl

import numpy as np

def test_interpolating_convolver_instantiation(plot=False):
    numberOfInputs = 2
    numberOfOutputs = 3

    blockLength = 64

    filterLength = 256

    routings = rbbl.FilterRoutingList([rbbl.FilterRouting(0,0,0), rbbl.FilterRouting(1,1,1)])

    numFilters = 64

    filters = np.zeros( (numFilters, filterLength), dtype=np.float32 )
    for idx in range(numFilters):
        filters[idx, 4*idx] = 1.0

    filterMtx = efl.BasicMatrixFloat(filters, alignment = 16)

    interpolationOrder = 3

    ip0 = rbbl.InterpolationParameter( 0, [0,1,2], [1.0, 0.0, 0.0] )

    interpolants = rbbl.InterpolationParameterSet( [ip0] )


    convolver = rbbl.InterpolatingConvolverUniformFloat(
            numberOfInputs=numberOfInputs,
            numberOfOutputs=numberOfOutputs,
            blockLength=blockLength,
            maxFilterLength=filterLength,
            maxRoutingPoints = len(routings),
            maxFilterEntries = numFilters,
            numberOfInterpolants = interpolationOrder,
            transitionSamples = blockLength,
            initialRoutings = routings,
            initialFilters = filterMtx,
            initialInterpolants = interpolants#,
    #        fftImplementation = "ffts"
            )

    numBlocks = 16
    signalLength = blockLength * numBlocks

    inputSignal = np.zeros( (numberOfInputs, signalLength), dtype=np.float32 )
    #inputSignal[0,100] = 1.0
    inputSignal[0,:] = np.sin( 2.0*np.pi*440/48000 * np.arange(signalLength) )


    outputSignal = np.zeros( (numberOfOutputs, signalLength), dtype=np.float32 )

    for blockIdx in range(numBlocks):
        inputBlock = inputSignal[:,blockIdx*blockLength:(blockIdx+1)*blockLength]
        outputBlock = convolver.process( inputBlock )
        outputSignal[:,blockIdx*blockLength:(blockIdx+1)*blockLength] = outputBlock

    t = np.arange(signalLength)

    if(plot):
        import matplotlib.pyplot as plt
        plt.figure()
        plt.plot( t, outputSignal[0,:], 'bo-' )
