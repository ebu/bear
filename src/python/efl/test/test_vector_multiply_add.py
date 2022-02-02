
import pytest

import efl

import numpy as np

from uniform_sequence import uniformSequence, uniformBasicVector, uniformScalar, typedVector

def test_vectorMultiplyAdd():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      a2 = uniformSequence( fullSize, dtype=dtype )
      addend = uniformSequence( fullSize, dtype=dtype )
      ref = a1*a2+addend

      res = np.zeros( fullSize, dtype = dtype )

      efl.vectorMultiplyAdd( a1, a2, addend, res, size )

      if size > 0:
        assert np.max( np.abs( res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if res.shape[-1] > size:
        assert np.all( np.abs(res[size:]) == 0.0 )

def test_vectorMultiplyAddInplace():
  for dtype in [np.float32, np.float64, np.complex64, np.complex128]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      a2 = uniformSequence( fullSize, dtype=dtype )
      addendRes = uniformSequence( fullSize, dtype=dtype )
      ref = a1*a2+addendRes

      addendOrig = addendRes

      efl.vectorMultiplyAddInplace( a1, a2, addendRes, size )

      if size > 0:
        assert np.all( np.abs( addendRes[:size] - ref[:size] ) <= np.finfo(dtype).eps * 4.0)
      if addendRes.shape[-1] > size:
        assert np.all( np.all( np.abs( addendRes[size:] - addendOrig[size:] ) <= np.finfo(dtype).eps * 4.0) )

def test_vectorMultiplyAddConstant():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      c1 = uniformScalar( dtype )
      addend = uniformSequence( fullSize, dtype=dtype )
      ref = a1*c1+addend

      res = np.zeros( fullSize, dtype = dtype )

      efl.vectorMultiplyConstantAdd( c1, a1, addend, res, size )

      if size > 0:
        assert np.max( np.abs( res[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if res.shape[-1] > size:
        assert np.all( np.abs(res[size:]) == 0.0 )

def test_vectorMultiplyConstantAddInplace():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for size in [0,1,2,3,4,5,7,15,17,31]:
      fullSize = size + 3 # Add padding to check whether the
      # function writes past the end
      a1 = uniformSequence( fullSize, dtype=dtype )
      addendRes = uniformSequence( fullSize, dtype=dtype )
      c1 = uniformScalar( dtype )
      ref = a1*c1+addendRes

      addendOrig = addendRes

      efl.vectorMultiplyConstantAddInplace( c1, a1, addendRes, size )

      if size > 0:
        assert np.max( np.abs( addendRes[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
      if addendRes.shape[-1] > size:
        assert np.all( np.all( np.abs( addendRes[size:] - addendOrig[size:] ) <= np.finfo(dtype).eps * 4.0) )

# Test the same functions, but use the BasicVector class.
# This enables setting the alignment.

def test_basicVectorMultiplyAdd():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
      for alignment in [0,1,2,4,8,16]:
        for size in [0,1,2,3,4,5,7,15,17,31]:
          fullSize = size + 3 # Add padding to check whether the
          # function writes past the end
          a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
          a2 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
          addend = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
          ref = np.array(a1)*np.array(a2)+np.array(addend)

          vecType = typedVector( dtype )
          res = vecType( fullSize, alignment=alignment )

          efl.vectorMultiplyAdd( a1, a2, addend, res, size, alignment=alignment )
          resData = np.array( res, copy=False )

          if size > 0:
            assert np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
          if resData.shape[-1] > size:
            assert np.all( np.abs(resData[size:]) == 0.0 )

def test_basicVectorMultiplyAddInplace():
  for dtype in [np.float32, np.float64, np.complex64, np.complex128]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        a2 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        addendRes = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment )
        ref = np.array(a1)*np.array( a2 ) + np.array( addendRes )

        addendOrig = np.array( addendRes, copy=True )

        efl.vectorMultiplyAddInplace( a1, a2, addendRes, size, alignment=alignment )
        resData = np.array( addendRes, copy=False )

        if size > 0:
          assert np.all( np.abs( resData[:size] - ref[:size] ) <= np.finfo(dtype).eps * 4.0)
        if resData.shape[-1] > size:
          assert np.all( np.all( np.abs( resData[size:] - addendOrig[size:] ) <= np.finfo(dtype).eps * 4.0) )

def test_basicVectorMultiplyAddConstant():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment  )
        c1 = uniformScalar( dtype )
        addend = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment  )
        ref = np.array( a1 ) * np.array( c1 ) + np.array( addend )

        res = typedVector( dtype )( fullSize, alignment=alignment )
        resData = np.array( res, copy = False )

        efl.vectorMultiplyConstantAdd( c1, a1, addend, res, size, alignment=alignment )

        if size > 0:
          assert np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
        if resData.shape[-1] > size:
          assert np.all( np.abs(resData[size:]) == 0.0 )

def test_basicVectorMultiplyConstantAddInplace():
  for dtype in [np.float32, np.double, np.complex64, np.complex128 ]:
    for alignment in [0,1,2,4,8,16]:
      for size in [0,1,2,3,4,5,7,15,17,31]:
        fullSize = size + 3 # Add padding to check whether the
        # function writes past the end
        a1 = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment  )
        addendRes = uniformBasicVector( fullSize, dtype=dtype, alignment=alignment  )
        c1 = uniformScalar( dtype )
        ref = np.array(a1)*np.array(c1)+np.array(addendRes)

        addendOrig = np.array(addendRes, copy=True)

        efl.vectorMultiplyConstantAddInplace( c1, a1, addendRes, size, alignment=alignment )
        resData = np.array( addendRes, copy=False )

        if size > 0:
          assert np.max( np.abs( resData[:size] - ref[:size] )) <= np.finfo(dtype).eps * 4.0
        if resData.shape[-1] > size:
          assert np.all( np.all( np.abs( resData[size:] - addendOrig[size:] ) <= np.finfo(dtype).eps * 4.0) )

# Run the functions (if not called within the pytest environment)
if __name__ == "__main__":
    test_vectorMultiplyAdd()
    test_vectorMultiplyAddInplace()
    test_vectorMultiplyAddConstant()
    test_vectorMultiplyConstantAddInplace()
    test_basicVectorMultiplyAdd()
    test_basicVectorMultiplyAddInplace()
    test_basicVectorMultiplyAddConstant()
    test_basicVectorMultiplyConstantAddInplace()
