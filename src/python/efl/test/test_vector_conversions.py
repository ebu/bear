
import pytest
from itertools import product

import efl

import numpy as np

from uniform_sequence import uniformSequence, typedVector

allTypes = [np.float32, np.float64, np.int8, np.int16, np.int32, np.int64]
allTypeCombinations = list( product( allTypes, allTypes ) )

allFloatTypes = [np.float32, np.float64]
allFloatTypeCombinations = list( product( allFloatTypes, allFloatTypes ) )


def vectorConversionReference(data, outputType):
    """ Python reference implementation for efl::vectorConvert() function """
    if np.issubdtype( data.dtype, np.floating) and \
      np.issubdtype( outputType, np.integer):
        # Note: rint() apparently rounds halfway integers away from 0, which is 
        # the same as C++ std::round()
        return np.asarray( np.rint( data ), dtype=outputType )
    else:
        return np.asarray( data, dtype=outputType )


@pytest.mark.parametrize("inputType, outputType", allTypeCombinations )
def test_vectorConvertNdArray( inputType, outputType ):
    bs = 16384
    
    inScaling = float(np.iinfo(inputType).max) if np.issubdtype(inputType, np.integer) else 1.0
    outScaling = float(np.iinfo(outputType).max) if np.issubdtype(outputType, np.integer) else 1.0
    
    scaling = min( inScaling, outScaling ) if (inScaling > 1 and outScaling > 1 )\
      else max(inScaling, outScaling)
    
    inSig = uniformSequence(bs, dtype=inputType, scaling=scaling)    
    outRef = vectorConversionReference( inSig, outputType=outputType )
    outSig = np.ones( bs, dtype=outputType )
    efl.vectorConvert(inSig, outSig, bs)
    if np.issubdtype(outputType, np.integer):
        mask = outSig != outRef
        if np.any( mask ):
            print( "Different results for input values %s" % str( inSig[mask]) )
        assert np.all( outSig == outRef)
    else:
        assert np.max(np.abs(outSig-outRef)) <= np.finfo( outputType ).eps

@pytest.mark.parametrize("inputType, outputType", allFloatTypeCombinations )
def test_vectorConvertBasicVector( inputType, outputType ):
    bs = 128
    alignment=4
    inScaling = float(np.iinfo(inputType).max) if np.issubdtype(inputType, np.integer) else 1.0
    outScaling = float(np.iinfo(outputType).max) if np.issubdtype(outputType, np.integer) else 1.0
    
    scaling = min( inScaling, outScaling ) if (inScaling > 1 and outScaling > 1 )\
      else max(inScaling, outScaling)
    
    inSig = uniformSequence(bs, dtype=inputType, scaling=scaling)
    inVec = typedVector( dtype=inputType )( inSig, alignment )    
    outRef = vectorConversionReference( inSig, outputType=outputType )
    outSig = np.ones( bs, dtype=outputType )
    outVec = typedVector( dtype=outputType )( outSig, alignment )
    efl.vectorConvert(inVec, outVec, bs, alignment)
    outArr = np.asarray( outVec )
    if np.issubdtype(outputType, np.integer):
        assert np.all( outArr == outRef)
    else:
        assert np.max(np.abs(outArr-outRef)) <= np.finfo( outputType ).eps


# When not run from pytest, but as a script, 
if __name__ == "__main__":
    for inputType, outputType in allTypeCombinations:
        test_vectorConvertNdArray(inputType=inputType, outputType=outputType)

    for inputType, outputType in allFloatTypeCombinations:
        test_vectorConvertBasicVector(inputType=inputType, outputType=outputType)
