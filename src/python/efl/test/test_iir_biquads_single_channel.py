
import pytest

import efl

import numpy as np
from scipy import signal

from uniform_sequence import uniformSequence, uniformBasicVector, uniformScalar, typedVector

def test_biquad_iir_filter_single_channel(plot=False):
    sigLen = 1024
    t = np.arange( sigLen )
    
    inputSignal = np.zeros( sigLen, dtype = np.float32 )
    outputSignal = np.ones( sigLen, dtype = np.float32 )
    coeffs = np.zeros( 6, dtype = np.float32 )
    
    #inputSignal[0] = 1
    
    # inputSignal[...] = np.asarray( 2*(np.random.rand( sigLen ) - 0.5), dtype=np.float32 
    
    ft = 880
    fs = 48e3
    
    inputSignal = 0.8*np.sin( 2*np.pi*ft/fs*t)
         
    # Coeffs: [b0, b1, b2, a1, a2 ]
    # scipy SOS coeffs
    coeffsSOS = np.asarray( [[1, 0.5, -0.75, 1.0, 0, -0.25 ], [1, 0.75, -0.75, 1.0, -0.5, 0.25 ]], dtype=np.float32 )

    coeffs = coeffsSOS[...,[0,1,2,4,5]] # / coeffsSOS[...,3]
    
    numSections = coeffs.shape[0]
    
    coeffsFlat = np.reshape( coeffs, (5*numSections), order='c' )

    state = np.zeros( (numSections * 2), dtype = np.float32 )
    
    outputRef = signal.sosfilt( coeffsSOS, inputSignal )
    
    numBlocks = 4
    assert sigLen % numBlocks == 0
    bs = sigLen // numBlocks
    
    
    for bIdx in range(numBlocks):
        efl.iirFilterBiquadsSingleChannel(inputSignal[bIdx*bs:(bIdx+1)*bs],
                                          outputSignal[bIdx*bs:(bIdx+1)*bs],
                                          states = state, 
                                          coeffs = coeffsFlat,
                                          numElements = bs,
                                          numSections = numSections,
                                          coeffStride = 5,
                                          stateStride = 2,
                                          alignment = 0 )
        
    if plot:
        import matplotlib.pyplot as plt
    
        plt.figure()
        plt.plot( t, outputRef, 'bo-' )
        plt.plot( t, outputSignal, 'mx-' )
        plt.show()
        
        plt.figure()
        plt.plot( t, outputSignal - outputRef, 'mx-' )
        plt.show()
    
if __name__ == "__main__":
    test_biquad_iir_filter_single_channel(plot=True)
    