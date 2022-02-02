/* Copyright (c) 2014-2019 Institute of Sound and Vibration Research,
   University of Southampton and VISR contributors --- All rights reserved. */
/* @author Andreas Franck andreas.franck@audioscenic.com */

#include "vector_functions.hpp"

#include "../alignment.hpp"

#include <immintrin.h>

#include <ciso646>

namespace visr
{
namespace efl
{
namespace intel_x86_64
{

template<>
ErrorCode
vectorRampScaling< float, Feature::VISR_SIMD_FEATURE >(
  float const * input,
  float const * ramp,
  float * output,
  float baseGain,
  float rampGain,
  std::size_t numberOfElements,
  bool accumulate /*= false*/,
  std::size_t alignmentElements /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( input, alignmentElements ) ) return alignmentError;
  if( not checkAlignment( ramp, alignmentElements ) ) return alignmentError;
  if( not checkAlignment( output, alignmentElements) ) return alignmentError;
#endif

  std::size_t count = numberOfElements;

  // TODO: Support AVX512 as soon as we have a machine to test it.

#ifdef __AVX__
  __m256 base = _mm256_broadcast_ss( &baseGain  );
  __m256 gain = _mm256_broadcast_ss( &rampGain  );
  if( accumulate )
  {
    if( alignmentElements >= 8 )
    {
      while( count >= 8 )
      {
        count -= 8;
        __m256 rampPart = _mm256_load_ps( ramp );
        __m256 inPart = _mm256_load_ps( input );
        ramp += 8;
        input += 8;
        __m256 outPart = _mm256_load_ps( output );
#ifdef __AVX2__
        __m256 scale = _mm256_fmadd_ps( rampPart, gain, base );
        __m256 res =  _mm256_fmadd_ps( scale, inPart, outPart );
#else
        __m256 scale = _mm256_add_ps( _mm256_mul_ps( rampPart, gain ), base );
        __m256 res = _mm256_add_ps( _mm256_mul_ps( scale, inPart ), outPart );
#endif
        _mm256_store_ps( output, res );
        output += 8;
      }
    }
    else // alignment != 8
    {
      while( count >= 8 )
      {
        count -= 8;
        __m256 rampPart = _mm256_loadu_ps( ramp );
        __m256 inPart = _mm256_loadu_ps( input );
        ramp += 8;
        input += 8;
        __m256 outPart = _mm256_loadu_ps( output );
#ifdef __AVX2__
        __m256 scale = _mm256_fmadd_ps( rampPart, gain, base );
        __m256 res =  _mm256_fmadd_ps( scale, inPart, outPart );
#else
        __m256 scale = _mm256_add_ps( _mm256_mul_ps( rampPart, gain ), base );
        __m256 res = _mm256_add_ps( _mm256_mul_ps( scale, inPart ), outPart );
#endif
        _mm256_storeu_ps( output, res );
        output += 8;
      }
    }
  }
  else // accumulate == false
  {
    if( alignmentElements >= 8 )
    {
      while( count >= 8 )
      {
        count -= 8;
        __m256 rampPart = _mm256_load_ps( ramp );
        __m256 inPart = _mm256_load_ps( input );
        ramp += 8;
        input += 8;
#ifdef __AVX2__
        __m256 scale = _mm256_fmadd_ps( rampPart, gain, base );
#else
        __m256 scale = _mm256_add_ps( _mm256_mul_ps( rampPart, gain ), base );
#endif
        __m256 res =  _mm256_mul_ps( scale, inPart );
        _mm256_store_ps( output, res );
        output += 8;
      }
    }
    else // alignment != 8
    {
      while( count >= 8 )
      {
        count -= 8;
        __m256 rampPart = _mm256_loadu_ps( ramp );
        __m256 inPart = _mm256_loadu_ps( input );
        ramp += 8;
        input += 8;
#ifdef __AVX2__
        __m256 scale = _mm256_fmadd_ps( rampPart, gain, base );
#else
        __m256 scale = _mm256_add_ps( _mm256_mul_ps( rampPart, gain ), base );
#endif
        __m256 res =  _mm256_mul_ps( scale, inPart );
        _mm256_storeu_ps( output, res );
        output += 8;
      }
    }
  }
#endif
  {
#ifdef __AVX__
    __m128 base = _mm_broadcast_ss( &baseGain  );
    __m128 gain = _mm_broadcast_ss( &rampGain  );
#else
    __m128 baseScalar = _mm_load_ss( &baseGain  );
    __m128 gainScalar = _mm_load_ss( &rampGain  );
    __m128 base = _mm_shuffle_ps( baseScalar, baseScalar, 0x00 );
    __m128 gain = _mm_shuffle_ps( gainScalar, gainScalar, 0x00 );
#endif
    if( accumulate )
    {
      if( alignmentElements >= 4 )
      {
        while( count >= 4 )
        {
          count -= 4;
          __m128 rampPart = _mm_load_ps( ramp );
          __m128 inPart = _mm_load_ps( input );
          __m128 outPart = _mm_load_ps( output );
          ramp += 4;
          input += 4;
#ifdef __AVX2__
          __m128 scale = _mm_fmadd_ps( rampPart, gain, base );
          __m128 res =  _mm_fmadd_ps( scale, inPart, outPart );
#else
          __m128 scale = _mm_add_ps( _mm_mul_ps( rampPart, gain ), base );
          __m128 res = _mm_add_ps( _mm_mul_ps( scale, inPart ), outPart );
#endif
          _mm_store_ps( output, res );
          output += 4;
        }
      }
      else // alignment not >= 4
      {
        while( count >= 4 )
        {
          count -= 4;
          __m128 rampPart = _mm_loadu_ps( ramp );
          __m128 inPart = _mm_loadu_ps( input );
          __m128 outPart = _mm_loadu_ps( output );
          ramp += 4;
          input += 4;
#ifdef __AVX2__
          __m128 scale = _mm_fmadd_ps( rampPart, gain, base );
          __m128 res =  _mm_fmadd_ps( scale, inPart, outPart );
#else
          __m128 scale = _mm_add_ps( _mm_mul_ps( rampPart, gain ), base );
          __m128 res = _mm_add_ps( _mm_mul_ps( scale, inPart ), outPart );
#endif
          _mm_storeu_ps( output, res );
          output += 4;
        }
      }
    }
    else // accumulate == false
    {
      if( alignmentElements >= 4 )
      {
        while( count >= 4 )
        {
          count -= 4;
          __m128 rampPart = _mm_load_ps( ramp );
          __m128 inPart = _mm_load_ps( input );
          ramp += 4;
          input += 4;
#ifdef __AVX2__
          __m128 scale = _mm_fmadd_ps( rampPart, gain, base );
#else
          __m128 scale = _mm_add_ps( _mm_mul_ps( rampPart, gain ), base );
#endif
          __m128 res =  _mm_mul_ps( scale, inPart );
          _mm_store_ps( output, res );
          output += 4;
        }
      }
      else // alignment < 4
      {
        while( count >= 4 )
        {
          count -= 4;
          __m128 rampPart = _mm_loadu_ps( ramp );
          __m128 inPart = _mm_loadu_ps( input );
          ramp += 4;
          input += 4;
#ifdef __AVX2__
          __m128 scale = _mm_fmadd_ps( rampPart, gain, base );
#else
          __m128 scale = _mm_add_ps( _mm_mul_ps( rampPart, gain ), base );
#endif
          __m128 res =  _mm_mul_ps( scale, inPart );
          _mm_storeu_ps( output, res );
          output += 4;
        }
      }
    }

    // Scalar processing of remaining elements
    if( accumulate )
    {
      while( count > 0 )
      {
        --count;
        __m128 rampPart = _mm_load_ss( ramp );
        __m128 inPart = _mm_load_ss( input );
        ++ramp;
        ++input;
        __m128 outPart = _mm_load_ss( output );
        __m128 scale = _mm_add_ss( _mm_mul_ps( rampPart, gain ), base );
        __m128 res =  _mm_add_ss(_mm_mul_ps( scale, inPart ), outPart );
        _mm_store_ss( output, res );
        ++output;
      }
    }
    else
    {
      while( count > 0 )
      {
        --count;
        __m128 rampPart = _mm_load_ss( ramp );
        __m128 inPart = _mm_load_ss( input );
        ++ramp;
        ++input;
        __m128 scale = _mm_add_ss( _mm_mul_ps( rampPart, gain ), base );
        __m128 res =  _mm_mul_ps( scale, inPart );
        _mm_store_ss( output, res );
        ++output;
      }
    }
  }
  return efl::noError;
}

#if 0
  float const * pf1 = factor1;
  float const * pf2 = factor2;
  float * pRes = result;




  std::size_t countN = numElements;
#ifdef __AVX__
  if( alignment >= 8 )
  {
    while( countN >= 8 )
    {
      __m128 a = _mm256_load_ps( pf1 );
      pf1 += 8;
      __m256 b = _mm256_load_ps( pf2 );
      pf2 += 8;
      a = _mm256_mul_ps( a, b );
      countN -= 8;
      _mm256_store_ps( pRes, a );
      pRes += 8;
    }
  }
  else
  {
    while( countN >= 8 )
    {
      __m256 a = _mm256_loadu_ps( pf1 );
      pf1 += 8;
      __m256 b = _mm256_loadu_ps( pf2 );
      pf2 += 8;
      a = _mm256_mul_ps( a, b );
      countN -= 8;
      _mm256_storeu_ps( pRes, a );
      pRes += 8;
    }
  }
#endif
// TODO: implement standard SSE/SSE2 with/without alignment
  while( countN > 0 )
  {
    __m128 a = _mm_load_ss( pf1 );
    ++pf1;
    __m128 b = _mm_load_ss( pf2 );
    ++pf2;
    a = _mm_mul_ss( a, b );
    --countN;
    _mm_store_ss( pRes, a );
    ++pRes;
  }
  return noError;
}

#endif

} // namespace intel_x86_64
} // namespace efl
} // namespace visr
