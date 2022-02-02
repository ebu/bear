
import pytest

import efl

import numpy as np

from uniform_sequence import uniformSequence, uniformBasicVector, uniformScalar, typedVector

def test_vectorAdd():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      a2 = uniformSequence( fullSize, dtype=dtype )
      ref = a1+a2

      res = np.zeros( fullSize, dtype = dtype )

      efl.vectorAdd( a1, a2, res, size )

      if size > 0:
        assert np.max( np.abs( res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if res.shape[-1] > size:
        assert np.all( np.abs(res[size:]) == 0.0 )

def test_vectorAddInplace():
  for dtype in [np.float32, np.float64, np.complex64, np.complex128]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      a2Res = uniformSequence( fullSize, dtype=dtype )
      ref = a1+a2Res

      a2Orig = a2Res

      efl.vectorAddInplace( a1, a2Res, size )

      if size > 0:
        assert np.all( np.abs( a2Res[:size] - ref[:size] ) <= np.finfo(dtype).eps * 4.0)
      if a2Res.shape[-1] > size:
        assert np.all( np.all( np.abs( a2Res[size:] - a2Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )

def test_vectorAddConstant():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      c1 = uniformScalar( dtype )
      ref = a1+c1

      res = np.zeros( fullSize, dtype = dtype )

      efl.vectorAddConstant( c1, a1, res, size )

      if size > 0:
        assert np.max( np.abs( res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if res.shape[-1] > size:
        assert np.all( np.abs(res[size:]) == 0.0 )

def test_vectorAddConstantInplace():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1Res = uniformSequence( fullSize, dtype=dtype )
      c1 = uniformScalar( dtype )
      ref = a1Res+c1

      a1Orig = a1Res

      efl.vectorAddConstantInplace( c1, a1Res, size )

      if size > 0:
        assert np.max( np.abs( a1Res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if a1Res.shape[-1] > size:
        assert np.all( np.all( np.abs( a1Res[size:] - a1Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )

# Test the same functions, but use the BasicVector class.
# This enables setting the alignment.

def test_basicVectorAdd():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        a2 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        ref = np.array(a1)+np.array(a2)

        res = typedVector(dtype)(fullSize, alignment=alignment)

        efl.vectorAdd( a1, a2, res, size, alignment=alignment )
        resData = np.array(res, copy=False )

        if size > 0:
          assert np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
        if resData.shape[-1] > size:
          assert np.all( np.abs(resData[size:]) == 0.0 )

def test_basicVectorAddInplace():
  for dtype in [np.float32, np.float64, np.complex64, np.complex128]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        a2Res = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        ref = np.array(a1)+np.array(a2Res)

        a2Orig = np.array(a2Res, copy=True)

        efl.vectorAddInplace( a1, a2Res, size, alignment=alignment )
        resData = np.array( a2Res, copy=False )

        if size > 0:
          assert np.all( np.abs( resData[:size] - ref[:size] ) <= np.finfo(dtype).eps * 4.0)
        if resData.shape[-1] > size:
          assert np.all( np.all( np.abs( resData[size:] - a2Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )

def test_basicVectorAddConstant():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        c1 = uniformScalar( dtype )
        ref = np.array(a1)+c1

        res = typedVector(dtype)( fullSize, alignment=alignment )

        efl.vectorAddConstant( c1, a1, res, size, alignment=alignment )
        resData = np.array( res, copy=False )

        if size > 0:
          assert np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
        if resData.shape[-1] > size:
          assert np.all( np.abs(resData[size:]) == 0.0 )

def test_basicVectorAddConstantInplace():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1Res = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        c1 = uniformScalar( dtype )
        ref = np.array(a1Res)+c1

        a1Orig = np.array(a1Res, copy=True)

        efl.vectorAddConstantInplace( c1, a1Res, size, alignment=alignment )
        resData = np.array( a1Res, copy=False )

        if size > 0:
          assert np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
        if resData.shape[-1] > size:
          assert np.all( np.all( np.abs( resData[size:] - a1Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )


if __name__ == "__main__":
    test_vectorAdd()
    test_vectorAddInplace()
    test_vectorAddConstant()
    test_vectorAddConstantInplace()
    test_basicVectorAdd()
    test_basicVectorAddInplace()
    test_basicVectorAddConstant()
    test_basicVectorAddConstantInplace()
