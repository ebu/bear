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

// Doxygen fails to find the corresponding declarations, therefore we 
// exclude the definitions here.
/// @cond NEVER

template<>
ErrorCode vectorMultiply<float, Feature::VISR_SIMD_FEATURE>(
    float const * const factor1,
    float const * const factor2,
    float * const result,
    std::size_t numElements,
    std::size_t alignment /*= 0*/ )
{
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;

  float const * pf1 = factor1;
  float const * pf2 = factor2;
  float * pRes = result;

  std::size_t countN = numElements;
#ifdef __AVX__
  if( alignment >= 8 )
  {
    while( countN >= 8 )
    {
      __m256 a = _mm256_load_ps( pf1 );
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

template<>
ErrorCode vectorMultiply<double, Feature::VISR_SIMD_FEATURE>(
    double const * const factor1,
    double const * const factor2,
    double * const result,
    std::size_t numElements,
    std::size_t alignment /*= 0*/ )
{
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;

  double const * pf1 = factor1;
  double const * pf2 = factor2;
  double * pRes = result;

  std::size_t countN = numElements;
#ifdef __AVX__
  if( alignment >= 4 )
  {
    while( countN >= 4 )
    {
      __m256d a = _mm256_load_pd( pf1 );
      pf1 += 4;
      __m256d b = _mm256_load_pd( pf2 );
      pf2 += 4;
      a = _mm256_mul_pd( a, b );
      countN -= 4;
      _mm256_store_pd( pRes, a );
      pRes += 4;
    }
  }
  else
  {
    while( countN >= 4 )
    {
      __m256d a = _mm256_loadu_pd( pf1 );
      pf1 += 4;
      __m256d b = _mm256_loadu_pd( pf2 );
      pf2 += 4;
      a = _mm256_mul_pd( a, b );
      countN -= 4;
      _mm256_storeu_pd( pRes, a );
      pRes += 4;
    }
  }
#endif // __AVX__
  while( countN > 0 )
  {
    __m128d a = _mm_load_sd( pf1 );
    ++pf1;
    __m128d b = _mm_load_sd( pf2 );
    ++pf2;
    a = _mm_mul_sd( a, b );
    --countN;
    _mm_store_sd( pRes, a );
    ++pRes;
  }
  return noError;
}

template<>
ErrorCode vectorMultiply<std::complex<float>, Feature::VISR_SIMD_FEATURE>(
    std::complex<float> const * const factor1,
    std::complex<float> const * const factor2,
    std::complex<float> * const result,
    std::size_t numElements,
    std::size_t alignment )
{
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;

  float const * pf1 = reinterpret_cast<float const *>(factor1);
  float const * pf2 = reinterpret_cast<float const *>(factor2);
  float * pRes = reinterpret_cast<float *>(result);

  std::size_t countN = numElements;
#ifdef __AVX__
  if( (alignment >= 4) )
  {
    while( countN >= 4 )
    {
      __m256 a = _mm256_load_ps( pf1 );
      __m256 b = _mm256_load_ps( pf2 );
      __m256 aHigh = _mm256_permute_ps( a, 0xA0 );
      a = _mm256_permute_ps( a, 0xF5 /*0b11110101*/ );
      __m256 partRes1 = _mm256_mul_ps( b, aHigh );
      b = _mm256_permute_ps( b, 0xB1 /*0b10110001*/ );
      __m256 partRes2 = _mm256_mul_ps( b, a );
      __m256 res = _mm256_addsub_ps( partRes1, partRes2 );
      _mm256_store_ps( pRes, res );

      countN -= 4;
      pf1 += 8;
      pf2 += 8;
      pRes += 8;
    }
  }
  else
  {
    while( countN >= 4 ) // Same as above, but unaligned memory accesses.
    {
      __m256 a = _mm256_loadu_ps( pf1 );
      __m256 b = _mm256_loadu_ps( pf2 );
      __m256 aHigh = _mm256_permute_ps( a, 0xA0 );
      a = _mm256_permute_ps( a, 0xF5 /*0b11110101*/ );
      __m256 partRes1 = _mm256_mul_ps( b, aHigh );
      b = _mm256_permute_ps( b, 0xB1 /*0b10110001*/ );
      __m256 partRes2 = _mm256_mul_ps( b, a );
      __m256 res = _mm256_addsub_ps( partRes1, partRes2 );
      _mm256_storeu_ps( pRes, res );
      countN -= 4;
      pf1 += 8;
      pf2 += 8;
      pRes += 8;
    }
  }
  if( countN == 0 ) // Shorthand exit if #elements is a multiple of 4.
  {
    return noError;
  }
  // Note: _mm_permute_ps requires AVX, change code to use SSE-only instructions.
  // TODO: Distinguish between aligned and unaligned cases.
  if( (countN >= 2) and( alignment >= 2 ) ) // Otherwise fall through to the scalar case.
  {
    while( countN >= 2 )
    {
      __m128 a = _mm_loadu_ps( pf1 );
      __m128 b = _mm_loadu_ps( pf2 );
      __m128 aHigh = _mm_permute_ps( a, 0xA0 );
      a = _mm_permute_ps( a, 0xF5 /*0b11110101*/ );
      __m128 partRes1 = _mm_mul_ps( b, aHigh );
      b = _mm_permute_ps( b, 0xB1 /*0b10110001*/ );
      __m128 partRes2 = _mm_mul_ps( b, a );
      __m128 res = _mm_addsub_ps( partRes1, partRes2 );
      _mm_store_ps( pRes, res );
      countN -= 2;
      pf1 += 4;
      pf2 += 4;
      pRes += 4;
    }
  }
#endif // __AVX__
  // Extra check needed, or is the while loop sufficient?
  if( countN > 0 ) // Scalar code for remaining samples
  {
    while( countN > 0 ) // no alignment or remaining samples
    {
      __m128 cReal = _mm_set_ps1( pf1[0] );
      __m128 cImag = _mm_set_ps1( pf1[1] );
      __m64 const * xL = reinterpret_cast<__m64 const*>(pf2);
      __m64 * yL = reinterpret_cast<__m64*>(pRes);

      __m128 ab = _mm_loadl_pi( cReal, xL ); // cReal is a dummy, we don't use the upper half of the register.
      __m128 acbc = _mm_mul_ps( cReal, ab );
      __m128 ba = _mm_shuffle_ps( ab, ab, 0xB1 );
      __m128 bdad = _mm_mul_ps( cImag, ba );
      __m128 res = _mm_addsub_ps( acbc, bdad );
      _mm_storel_pi( yL, res );

      pf1 += 2;
      pf2 += 2;
      pRes += 2;
      --countN;
    }
  }
  return noError;
}

/// @endcond NEVER

} // namespace intel_x86_64
} // namespace efl
} // namespace visr
