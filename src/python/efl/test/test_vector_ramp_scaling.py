
import pytest

import efl

import numpy as np
from itertools import product

from uniform_sequence import typedVector

def reference(input, ramp, out, baseGain, rampGain, numElements, accumulate):
    """ Reference implementation for vectorRampScaling. """
    scale = rampGain * ramp + baseGain
    if accumulate:
        out += scale * input
    else:
        out = scale * input
    return out

def scalarConstant( val, dtype ):
    """
    Translate a scalar value a complex number if dtype is complex.
    """
    if  np.issubdtype( dtype, np.complexfloating ):
        return complex( val )
    else:
        return val

@pytest.mark.parametrize("dtype", [np.float32,np.float64,np.complex64,np.complex128])
def test_vectorRampScalingFullUpNdArray( dtype ):

    bs = 1024
    ramp = np.linspace(0.0, 1.0, bs, endpoint = False, dtype=dtype)
    fs = 48e3
    fSig = 3210;
    inSig = np.asarray(np.sin( 2*np.pi*fSig/fs*np.arange(bs) ), dtype=dtype )
    outSig = np.ones(bs, dtype=dtype)
    accumulate = False
    baseGain=scalarConstant(0.0, dtype=dtype)
    rampGain=scalarConstant(1.0, dtype=dtype)
    outRef = reference(inSig, ramp, outSig, baseGain, rampGain, bs,
                        accumulate)
    efl.vectorRampScaling(inSig, ramp, outSig, baseGain, rampGain,
                          bs, accumulate, 0 )
    assert np.max(np.abs(outSig-outRef)) <= np.finfo( dtype ).eps

@pytest.mark.parametrize("dtype", [np.float32,np.float64,np.complex64,np.complex128])
def test_vectorRampScalingHalfDownNdArray( dtype ):
    bs = 100
    ramp = np.linspace(0.0, 1.0, bs, endpoint = False, dtype=dtype)
    fs = 48e3
    fSig = 4783;
    inSig = np.asarray(np.sin( 2*np.pi*fSig/fs*np.arange(bs) ), dtype=dtype )
    outSig = np.ones(bs, dtype=dtype)
    accumulate = False
    baseGain=scalarConstant(1.0, dtype=dtype)
    rampGain=scalarConstant(-0.5, dtype=dtype)
    outRef = reference(inSig, ramp, outSig, baseGain, rampGain, bs,
                        accumulate)
    efl.vectorRampScaling(inSig, ramp, outSig, baseGain, rampGain,
                          bs, accumulate, 0 )
    assert np.max(np.abs(outSig-outRef)) <= np.finfo( dtype ).eps


@pytest.mark.parametrize("dtype", [np.float32,np.float64,np.complex64,np.complex128])
def test_vectorRampScalingAccumulateNdArray( dtype ):
    bs = 65
    ramp = np.linspace(0.0, 1.0, bs, endpoint = False, dtype=dtype)
    fs = 48e3
    fSigBase = 8765;
    fSig = 3210;
    inSig = np.asarray(np.sin( 2*np.pi*fSig/fs*np.arange(bs) ), dtype=dtype )
    outSigBase = np.asarray(np.cos( 2*np.pi*fSigBase/fs*np.arange(bs) ), dtype=dtype )
    outSig = np.array( outSigBase, copy=True )
    accumulate = True
    baseGain=scalarConstant(0.0, dtype=dtype)
    rampGain=scalarConstant(1.0, dtype=dtype)
    outRef = reference(inSig, ramp, outSigBase, baseGain, rampGain, bs,
                        accumulate)
    efl.vectorRampScaling(inSig, ramp, outSig, baseGain, rampGain,
                          bs, accumulate, 0 )
    assert np.max(np.abs(outSig-outRef)) <= np.finfo( dtype ).eps

@pytest.mark.parametrize("dtype", [np.float32,np.float64,np.complex64,np.complex128])
def test_vectorRampScalingFullUpBasicVector( dtype ):
    VecType = typedVector( dtype )

    bs = 1024
    ramp = VecType( np.linspace(0.0, 1.0, bs, endpoint = False, dtype=dtype))
    fs = 48e3
    fSig = 3210;
    inSig = VecType(np.asarray(np.sin( 2*np.pi*fSig/fs*np.arange(bs) ), dtype=dtype ))
    outSig = VecType(np.ones(bs, dtype=dtype))
    accumulate = False
    baseGain=scalarConstant(0.0, dtype=dtype)
    rampGain=scalarConstant(1.0, dtype=dtype)
    outRef = reference(np.asarray(inSig), np.asarray(ramp), np.asarray(outSig),
                       baseGain, rampGain, bs,
                       accumulate)
    efl.vectorRampScaling(inSig, ramp,
                          outSig, baseGain, rampGain,
                          bs, accumulate, 0 )
    outSigArr = np.asarray( outSig ) # get an ndarray view
    assert np.max(np.abs(outSigArr-outRef)) <= np.finfo( dtype ).eps

@pytest.mark.parametrize("dtype", [np.float32,np.float64,np.complex64,np.complex128])
def test_vectorRampScalingAccumulateBasicVector( dtype ):
    VecType = typedVector( dtype )

    bs = 1024
    ramp = VecType( np.linspace(0.0, 1.0, bs, endpoint = False, dtype=dtype))
    fs = 48e3
    fSigBase = 5678;
    fSig = 3210;
    inSig = VecType(np.asarray(np.sin( 2*np.pi*fSig/fs*np.arange(bs) ), dtype=dtype ))
    outSigBase = np.asarray(np.cos( 2*np.pi*fSigBase/fs*np.arange(bs) ), dtype=dtype )
    outSig = VecType(outSigBase)
    accumulate = True
    baseGain=scalarConstant(0.0, dtype=dtype)
    rampGain=scalarConstant(1.0, dtype=dtype)
    outRef = reference(np.asarray(inSig), np.asarray(ramp), outSigBase,
                       baseGain, rampGain, bs,
                       accumulate)
    efl.vectorRampScaling(inSig, ramp,
                          outSig, baseGain, rampGain,
                          bs, accumulate, 0 )
    outSigArr = np.asarray( outSig ) # get an ndarray view
    assert np.max(np.abs(outSigArr-outRef)) <= np.finfo( dtype ).eps

blockSizes = [0,1,2,3,5,7,8,11,16,31,32,33,63,127, 1023]
alignments = [0,1,2,4,8,16]
dataTypes = [np.float32,np.float64,np.complex64,np.complex128]

@pytest.mark.parametrize("dtype, bs, alignment",
                         list(product(dataTypes, blockSizes, alignments)))
def test_vectorRampScalingBasicVectorAlignment( dtype, bs, alignment ):
    VecType = typedVector(dtype)

    ramp = VecType(np.linspace(0.0, 1.0, bs, endpoint = False, dtype=dtype),
                   alignment=alignment)
    fs = 48e3
    fSigBase = 5678;
    fSig = 3210;
    inSig = VecType(np.asarray(np.sin( 2*np.pi*fSig/fs*np.arange(bs) ), dtype=dtype),
                    alignment=alignment)
    outSigBase = np.asarray(np.cos( 2*np.pi*fSigBase/fs*np.arange(bs) ), dtype=dtype )
    outSig = VecType(outSigBase, alignment=alignment)
    accumulate = True
    baseGain=scalarConstant(0.0, dtype=dtype)
    rampGain=scalarConstant(1.0, dtype=dtype)
    outRef = reference(np.asarray(inSig), np.asarray(ramp), outSigBase,
                       baseGain, rampGain, bs,
                       accumulate)
    efl.vectorRampScaling(inSig, ramp,
                          outSig, baseGain, rampGain,
                          bs, accumulate, 0 )
    outSigArr = np.asarray( outSig ) # get an ndarray view
    assert np.all(np.abs(outSigArr-outRef) <= np.finfo( dtype ).eps)


# Enables the tests to be run as a script (in addition to using pytest)
if __name__ == "__main__":
    for dtype in [np.float32, np.float64, np.complex64, np.complex128]:
        test_vectorRampScalingFullUpNdArray(dtype=dtype)
        test_vectorRampScalingHalfDownNdArray(dtype=dtype)
        test_vectorRampScalingAccumulateNdArray(dtype=dtype)
        test_vectorRampScalingFullUpBasicVector(dtype=dtype)
        test_vectorRampScalingAccumulateBasicVector(dtype=dtype)
    for combi in product(dataTypes, blockSizes, alignments):
        test_vectorRampScalingBasicVectorAlignment( *combi )
