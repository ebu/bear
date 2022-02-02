
import visr
import panningdsp
import pml
import rrl

import numpy as np


def test_initialiseDefault():
    numObjects = 4
    numLoudspeakers = 8
    alignment = 16

    pmg1 = panningdsp.PanningMatrixParameter(numObjects,
                                             numLoudspeakers,
                                             alignment)

    assert pmg1.numberOfObjects == numObjects
    assert pmg1.numberOfLoudspeakers == numLoudspeakers
    assert pmg1.alignmentElements == alignment

    assert pmg1.numberOfObjects == numObjects
    assert pmg1.numberOfLoudspeakers == numLoudspeakers

    # Default intialisation means zero gains.
    gains =  np.asarray(pmg1.gains)
    assert gains.ndim == 2 and gains.shape == (numObjects, numLoudspeakers)
    assert np.all( gains == 0.0 )

    timeStamps = pmg1.timeStamps
    assert timeStamps.ndim == 1 and timeStamps.shape[0] == numObjects
    assert np.all( timeStamps == panningdsp.timeStampInfinity )

    transitions = pmg1.transitionTimes
    assert transitions.ndim == 1 and transitions.shape[0] == numObjects
    assert np.all( transitions == panningdsp.timeStampInfinity )

def test_initialiseFromMatrix():
    numObjects = 7
    numLoudspeakers = 5
    alignment = 4

    gainsInit = np.random.random_sample( (numObjects, numLoudspeakers) );

    ts = np.asarray( np.random.randint( 0, 100, numObjects ), dtype=np.uint64 )
    ii = np.asarray( np.random.randint( 200, 300, numObjects ), dtype=np.uint64 )

    pmg1 = panningdsp.PanningMatrixParameter(gainsInit, ts, ii, alignment )

    assert pmg1.numberOfObjects == numObjects
    assert pmg1.numberOfLoudspeakers == numLoudspeakers
    assert pmg1.alignmentElements == alignment

    assert pmg1.numberOfObjects == numObjects
    assert pmg1.numberOfLoudspeakers == numLoudspeakers

    # Default intialisation means zero gains.
    gains = np.asarray( pmg1.gains )
    assert np.all( np.abs(gains - gainsInit) < 1e-7 )

    timeStamps = pmg1.timeStamps
    assert timeStamps.ndim == 1 and timeStamps.shape[0] == numObjects
    assert np.all( timeStamps == ts )

    transitions = pmg1.transitionTimes
    assert transitions.ndim == 1 and transitions.shape[0] == numObjects
    assert np.all( transitions == ii )

# Enable to run the unit test as a script.
if __name__ == "__main__":
    test_initialiseDefault()
    test_initialiseFromMatrix()
