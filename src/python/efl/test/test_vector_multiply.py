
import pytest

import efl

import numpy as np

from uniform_sequence import uniformSequence, uniformBasicVector, uniformScalar, typedVector

def test_vectorMultiply():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      a2 = uniformSequence( fullSize, dtype=dtype )
      ref = a1*a2

      res = np.zeros( fullSize, dtype = dtype )

      efl.vectorMultiply( a1, a2, res, size )

      if size > 0:
        assert np.max( np.abs( res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if res.shape[-1] > size:
        assert np.all( np.abs(res[size:]) == 0.0 )

def test_vectorMultiplyInplace():
  for dtype in [np.float32, np.float64, np.complex64, np.complex128]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      a2Res = uniformSequence( fullSize, dtype=dtype )
      ref = a1*a2Res

      a2Orig = a2Res

      efl.vectorMultiplyInplace( a1, a2Res, size )

      if size > 0:
        np.all( np.abs( a2Res[:size] - ref[:size] ) <= np.finfo(dtype).eps * 4.0)
      if a2Res.shape[-1] > size:
        assert np.all( np.all( np.abs( a2Res[size:] - a2Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )

def test_vectorMultiplyConstant():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      c1 = uniformScalar( dtype )
      ref = a1*c1

      res = np.zeros( fullSize, dtype = dtype )

      efl.vectorMultiplyConstant( c1, a1, res, size )

      if size > 0:
        assert np.max( np.abs( res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if res.shape[-1] > size:
        assert np.all( np.abs(res[size:]) == 0.0 )

def test_vectorMultiplyConstantInplace():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1Res = uniformSequence( fullSize, dtype=dtype )
      c1 = uniformScalar( dtype )
      ref = a1Res*c1

      a1Orig = a1Res

      efl.vectorMultiplyConstantInplace( c1, a1Res, size )

      if size > 0:
        assert np.max( np.abs( a1Res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if a1Res.shape[-1] > size:
        assert np.all( np.all( np.abs( a1Res[size:] - a1Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )

# Test the same functions, but use the BasicVector class.
# This enables setting the alignment.


def test_basicVectorMultiply():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        a2 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        ref = np.array(a1)*np.array(a2)

        res = typedVector(dtype = dtype)( fullSize, alignment=alignment )

        efl.vectorMultiply( a1, a2, res, size, alignment=alignment )
        resData = np.array( res, copy=False )

        if size > 0:
          if not np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0:
            print("Fail!")
        if resData.shape[-1] > size:
          if not np.all( np.abs(resData[size:]) == 0.0 ):
            print("Fail!")

def test_basicVectorMultiplyInplace():
  for dtype in [np.float32, np.float64, np.complex64, np.complex128]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        a2Res = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        ref = np.array(a1) * np.array(a2Res)

        a2Orig = np.array( a2Res, copy=True )

        efl.vectorMultiplyInplace( a1, a2Res, size, alignment=alignment )
        a2ResData = np.array( a2Res, copy=False )

        if size > 0:
          np.all( np.abs( a2ResData[:size] - ref[:size] ) <= np.finfo(dtype).eps * 4.0)
        if a2ResData.shape[-1] > size:
          assert np.all( np.all( np.abs( a2ResData[size:] - a2Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )

def test_basicVectorMultiplyConstant():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        c1 = uniformScalar( dtype )
        ref = np.array(a1)*c1

        res = typedVector(dtype=dtype)( fullSize, alignment=alignment )
        resData = np.array( res, copy=False )

        efl.vectorMultiplyConstant( c1, a1, res, size, alignment=alignment )

        if size > 0:
          assert np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
        if resData.shape[-1] > size:
          assert np.all( np.abs(resData[size:]) == 0.0 )

def test_basicVectorMultiplyConstantInplace():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1Res = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        c1 = uniformScalar( dtype )
        ref = np.array(a1Res)*c1

        a1Orig = np.array( a1Res, copy=True )

        efl.vectorMultiplyConstantInplace( c1, a1Res, size, alignment=alignment )
        a1ResData = np.array( a1Res, copy=False )

        if size > 0:
          assert np.max( np.abs( a1ResData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
        if a1ResData.shape[-1] > size:
          assert np.all( np.all( np.abs( a1ResData[size:] - a1Orig[size:] ) <= np.finfo(dtype).eps * 4.0) )


if __name__ == "__main__":
    test_vectorMultiply()
    test_vectorMultiplyInplace()
    test_vectorMultiplyConstant()
    test_vectorMultiplyConstantInplace()
    test_basicVectorMultiply()
    test_basicVectorMultiplyInplace()
    test_basicVectorMultiplyConstant()
    test_basicVectorMultiplyConstantInplace()
