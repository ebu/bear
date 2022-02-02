#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Apr 29 08:04:07 2019

@author: andi
"""

import numpy as np

import efl

def uniformSequence( n, dtype=np.float32, scaling=1.0 ):
  """
  Return a vector of uniformly distributed elements (real or complex-valued)
  """
  if np.issubdtype( dtype, np.complexfloating ):
    return np.asarray( uniformSequence( n, dtype=np.float64)
                       + 1j*uniformSequence( n, dtype=np.float64),
                       dtype=dtype )
  else:      
    return np.asarray(2.0*scaling*(np.random.rand( n )-0.5),dtype=dtype)

def typedVector( dtype ):
  """
  Translate a numpy data type into the corresponding efl BasicVector class type.
  """
  if dtype == np.float32:
      return efl.BasicVectorFloat
  if dtype == np.float64:
      return efl.BasicVectorDouble
  if dtype == np.complex64:
      return efl.BasicVectorComplexFloat
  if dtype == np.complex128:
      return efl.BasicVectorComplexDouble
  raise ValueError( "No efl.BasicVector for this numeric data type." )
    
def uniformBasicVector(  n, dtype=np.float32, scaling=1.0, alignment=0 ):
    vecType = typedVector( dtype )
    val = uniformSequence( n, dtype )
    ret = vecType( val, alignment=alignment )
    return ret
  
def uniformScalar( dtype = np.float32 ):
  """
  Return a random scalar (real or complex )
  """
  isComplex = np.issubdtype( dtype, np.complexfloating )
  if isComplex:
    return complex( 2*np.random.rand()-1.0, 2*np.random.rand()-1.0 )
  else:
    return 2*np.random.rand()-1.0
