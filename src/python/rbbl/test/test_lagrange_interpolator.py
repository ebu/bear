# Copyright (c) 2014-2020 Institute of Sound and Vibration Research,
# University of Southampton and VISR contributors --- All rights reserved.
# Copyright (c) 2014-2020 Andreas Franck a.franck@soton.ac.uk --- All rights reserved.

import pytest

import rbbl

import numpy as np

@pytest.mark.parametrize("dType", [np.float32, np.float64])
def test_lagrangeOrder3_sineDelayRamp(dType):
    numSamples=1024

    sigLen = 4096

    tBase = np.arange(sigLen)

    startDelay = 1.0
    endDelay = 4 + startDelay
    startGain=0.9
    endGain=0.5

    # TODO: Ensure that input offset is large sun is large enough.
    inputOffs=1050

    output = np.full( numSamples, np.inf, dtype=dType )


    # # Use linear ramp to check that the interpolator is working correctly
    # # close to DC.
    # input = np.arange( sigLen, dtype=dType )
    # outputRef = tRef

    f1=4891
    fs=48000
    input = 0.9*np.sin( 2*np.pi*f1/fs * tBase, dtype=dType )


    if dType == np.float32:
        cls = rbbl.LagrangeInterpolatorOrder3Float
    elif dType == np.float64:
        cls = rbbl.LagrangeInterpolatorOrder3Double
    interp = cls(numSamples, alignmentElements=0)
    methodDelay=interp.methodDelay()

    interp.interpolate(input[inputOffs:], output,
                                    numSamples, startDelay, endDelay,
                                    startGain, endGain)

    tDelayRef = np.flip( np.linspace(endDelay,startDelay,numSamples,endpoint=False) )
    tRef = float(inputOffs) - tDelayRef\
        - np.linspace( numSamples-1, 0, numSamples ) - methodDelay
    outputRef = 0.9*np.sin( 2*np.pi*f1/fs * tRef, dtype=dType )\
        * np.flip( np.linspace(endGain, startGain, numSamples, endpoint=False))

    # The error (and thus the tolerance) depends strongly on the signal
    # frequency and the interpolation algorithm
    maxTol = 0.01*max( startGain, endGain )

    assert np.max( np.abs( output-outputRef )) < maxTol

# Enables the tests to be run as a script (in addition to using pytest)
if __name__ == "__main__":
    for dtype in [np.float32, np.float64 ]:
        test_lagrangeOrder3_sineDelayRamp(dtype)
