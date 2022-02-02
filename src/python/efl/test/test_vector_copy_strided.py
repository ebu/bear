
import pytest
import numpy as np
from itertools import product

import efl

from uniform_sequence import uniformSequence, typedVector

dataTypes = [np.float32, np.float64, np.complex64, np.complex128]
strides = [1,2,3,4,5,8,15,31]
numEl = [1,2,3,5,8,9,15,16,33]

allCombinations = list(product(dataTypes, strides, strides, numEl))

@pytest.mark.parametrize("dtype, srcStride, destStride, numElements", allCombinations )
def test_vectorCopyStrided( dtype, srcStride, destStride, numElements):
    src = uniformSequence( numElements*srcStride, dtype=dtype)
    dest = np.full( numElements*destStride, fill_value=np.NaN, dtype=dtype)

    efl.vectorCopyStrided(src,dest, srcStride, destStride, numElements, 0)
    
    assert np.all(src[::srcStride][:numElements] == dest[::destStride][:numElements])

@pytest.mark.parametrize("dtype, srcStride, destStride, numElements", allCombinations )
def test_vectorCopyStridedBasicVector( dtype, srcStride, destStride, numElements):
    alignment=8 # TODO: Check diferent alignments, too?
    src = uniformSequence( numElements*srcStride, dtype=dtype)
    srcVec = typedVector( dtype=dtype )( src, alignment )  
    
    dest = np.full( numElements*destStride, fill_value=np.NaN, dtype=dtype)
    destVec = typedVector( dtype=dtype )( dest, alignment )  

    efl.vectorCopyStrided(srcVec,destVec, srcStride, destStride, numElements, alignment)

    destView = np.asarray(destVec)
    
    assert np.all(src[::srcStride][:numElements] == destView[::destStride][:numElements])


if __name__ == "__main__":
    for dtype, srcStride, destStride, numElements in allCombinations:
        test_vectorCopyStrided(dtype, srcStride, destStride, numElements)
        test_vectorCopyStridedBasicVector(dtype, srcStride, destStride, numElements)
