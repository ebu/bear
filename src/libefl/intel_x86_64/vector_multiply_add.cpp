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
ErrorCode vectorMultiplyAddInplace<float, Feature::VISR_SIMD_FEATURE>( float const * const factor1,
				    float const * const factor2,
				    float * const accumulator,
				    std::size_t numElements,
				    std::size_t alignment /*= 0*/ )
{
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( accumulator, alignment ) ) return alignmentError;

  float const * pf1 = factor1;
  float const * pf2 = factor2;
  float * y = accumulator;
  std::size_t cnt = numElements;

#ifdef __AVX__
  if( alignment >= 8 )
  {
    while( cnt >= 8 )
    {
      __m256 a = _mm256_load_ps( pf1 );
      pf1 += 8;
      __m256 b = _mm256_load_ps( pf2 );
      pf2 += 8;
      __m256 acc = _mm256_load_ps( y );
      cnt -= 8;
#ifdef __AVX2__
      acc = _mm256_fmadd_ps( a, b, acc );
#else
      __m256 res = _mm256_mul_ps( a, b );
      acc = _mm256_add_ps( res, acc );
#endif
      _mm256_store_ps( y, acc );
      y += 8;
    }
  }
  else // Same as above, but unaligned.
  {
    while( cnt >= 8 )
    {
      __m256 a = _mm256_loadu_ps( pf1 );
      pf1 += 8;
      __m256 b = _mm256_loadu_ps( pf2 );
      pf2 += 8;
      __m256 acc = _mm256_loadu_ps( y );
      cnt -= 8;
#ifdef __AVX2__
      acc = _mm256_fmadd_ps( a, b, acc );
#else
      __m256 res = _mm256_mul_ps( a, b );
      acc = _mm256_add_ps( res, acc );
#endif
      _mm256_storeu_ps( y, acc );
      y += 8;
    }
  }
  if( cnt == 0 ) // Shortcut if #numElements is a multiple of 8  
  {
    return noError;
  }
#endif // __AVX__
  while( cnt >= 4 )
  {
    __m128 a = _mm_loadu_ps( pf1 );
    pf1 += 4;
    __m128 b = _mm_loadu_ps( pf2 );
    pf2 += 4;
    __m128 acc = _mm_loadu_ps( y );
    cnt -= 4;
#ifdef __AVX2__
    acc = _mm_fmadd_ps( a, b, acc );
#else
    __m128 mulRes = _mm_mul_ps( a, b );
    acc = _mm_add_ps( mulRes, acc );
#endif
    _mm_storeu_ps( y, acc );
    y += 4;
  }
  while( cnt > 0 )
  {
    __m128 a = _mm_load_ss( pf1 );
    ++pf1;
    __m128 b = _mm_load_ss( pf2 );
    ++pf2;
    __m128 acc = _mm_load_ss( y );
    --cnt;
#ifdef __AVX2__
    acc = _mm_fmadd_ss( a, b, acc );
#else
    __m128 mulRes = _mm_mul_ss( a, b );
    acc = _mm_add_ss( mulRes, acc );
#endif
    _mm_store_ss( y, acc );
    ++y;
  }
  return noError;
}

template<>
ErrorCode vectorMultiplyAddInplace<double, Feature::VISR_SIMD_FEATURE>( double const * const factor1,
				    double const * const factor2,
				    double * const accumulator,
				    std::size_t numElements,
				    std::size_t alignment /*= 0*/ )
{
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( accumulator, alignment ) ) return alignmentError;

  double const * pf1 = factor1;
  double const * pf2 = factor2;
  double * y = accumulator;
  std::size_t cnt = numElements;

#ifdef __AVX__
  if( alignment >= 4 )
  {
    while( cnt >= 4 )
    {
      __m256d a = _mm256_load_pd( pf1 );
      pf1 += 4;
      __m256d b = _mm256_load_pd( pf2 );
      pf2 += 4;
      __m256d acc = _mm256_load_pd( y );
      cnt -= 4;
#ifdef __AVX2__
      acc = _mm256_fmadd_pd( a, b, acc );
#else
      __m256d mulRes = _mm256_mul_pd( a, b );
      acc = _mm256_add_pd( mulRes, acc );
#endif
      _mm256_store_pd( y, acc );
      y += 4;
    }
  }
  else // Same as above, but unaligned.
  {
    while( cnt >= 4 )
    {
      __m256d a = _mm256_loadu_pd( pf1 );
      pf1 += 4;
      __m256d b = _mm256_loadu_pd( pf2 );
      pf2 += 4;
      __m256d acc = _mm256_loadu_pd( y );
      cnt -= 4;
#ifdef __AVX2__
      acc = _mm256_fmadd_pd( a, b, acc );
#else
      __m256d mulRes = _mm256_mul_pd( a, b );
      acc = _mm256_add_pd( mulRes, acc );
#endif
      _mm256_storeu_pd( y, acc );
      y += 4;
    }
  }
  if( cnt == 0 ) // Shortcut if #numElements is a multiple of 8  
  {
    return noError;
  }
#endif // __AVX__
  while( cnt >= 2 ) // Happens at most once
  {
    // Do not distinguish between aligned and unaligned accesses.
    __m128d a = _mm_loadu_pd( pf1 );
    pf1 += 2;
    __m128d b = _mm_loadu_pd( pf2 );
    pf2 += 2;
    __m128d acc = _mm_loadu_pd( y );
    cnt -= 2;
#ifdef __AVX2__
    acc = _mm_fmadd_pd( a, b, acc );
#else
    __m128d mulRes = _mm_mul_pd( a, b );
    acc = _mm_add_pd( mulRes, acc );
#endif
    _mm_storeu_pd( y, acc );
    y += 2;
  }
  while( cnt > 0 )
  {
    __m128d a = _mm_load_sd( pf1 );
    ++pf1;
    __m128d b = _mm_load_sd( pf2 );
    ++pf2;
    __m128d acc = _mm_load_sd( y );
    --cnt;
#ifdef __AVX2__
    acc = _mm_fmadd_sd( a, b, acc );
#else
    __m128d mulRes = _mm_mul_sd( a, b );
    acc = _mm_add_sd( mulRes, acc );
#endif    
    _mm_store_sd( y, acc );
    ++y;
  }
  return noError;
}

// Temporarily suppress Doxygen warning until the feature-based dispatching is implemented properly.
/// @cond NEVER
template<>
ErrorCode vectorMultiplyAddInplace<std::complex<float>, Feature::VISR_SIMD_FEATURE>( std::complex<float> const * const factor1,
                                    std::complex<float> const * const factor2,
                                    std::complex<float> * const accumulator,
                                    std::size_t numElements,
                                    std::size_t alignment )
{
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( accumulator, alignment ) ) return alignmentError;

  float const * pf1 = reinterpret_cast<float const *>(factor1);
  float const * pf2 = reinterpret_cast<float const *>(factor2);
  float * pAcc = reinterpret_cast<float *>(accumulator);

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
      __m256 acc = _mm256_load_ps( pAcc );
      __m256 res = _mm256_addsub_ps( partRes1, partRes2 );
      __m256 finalRes = _mm256_add_ps( res, acc );
      _mm256_store_ps( pAcc, finalRes );

      countN -= 4;
      pf1 += 8;
      pf2 += 8;
      pAcc += 8;
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
      __m256 acc = _mm256_loadu_ps( pAcc );
      __m256 res = _mm256_addsub_ps( partRes1, partRes2 );
      __m256 finalRes = _mm256_add_ps( res, acc );
      _mm256_storeu_ps( pAcc, finalRes );
      countN -= 4;
      pf1 += 8;
      pf2 += 8;
      pAcc += 8;
    }
  }
  if( countN == 0 ) // Shorthand exit if #elements is a multiple of 4.
  {
    return noError;
  }
  // Note: _mm_permute_ps requires AVX.
  // TODO: Implement unaligned case.
  if( (countN >= 2) and( alignment >= 2 ) ) // Otherwise fall through to the scalar case.
  {
    while( countN >= 2 )
    {
      __m128 a = _mm_load_ps( pf1 );
      __m128 b = _mm_load_ps( pf2 );
      __m128 aHigh = _mm_permute_ps( a, 0xA0 );
      a = _mm_permute_ps( a, 0xF5 /*0b11110101*/ );
      __m128 partRes1 = _mm_mul_ps( b, aHigh );
      b = _mm_permute_ps( b, 0xB1 /*0b10110001*/ );
      __m128 acc = _mm_load_ps( pAcc );
      __m128 partRes2 = _mm_mul_ps( b, a );
      __m128 res = _mm_addsub_ps( partRes1, partRes2 );
      __m128 finalRes = _mm_add_ps( acc, res );
      _mm_store_ps( pAcc, finalRes );
      countN -= 2;
      pf1 += 4;
      pf2 += 4;
      pAcc += 4;
    }
  }
#endif // __AVX__
  if( countN > 0 ) // Scalar code for remaining samples
  {
    while( countN > 0 ) // no alignment or remaining samples
    {
      __m128 cReal = _mm_set_ps1( pf1[0] );
      __m128 cImag = _mm_set_ps1( pf1[1] );
      __m64 const * xL = reinterpret_cast<__m64 const*>(pf2);
      __m64 * yL = reinterpret_cast<__m64*>(pAcc);

      __m128 ab = _mm_loadl_pi( cReal, xL ); // cReal is a dummy, we don't use the upper half of the register.
      __m128 acbc = _mm_mul_ps( cReal, ab );
      __m128 ba = _mm_shuffle_ps( ab, ab, 0xB1 );
      __m128 acc = _mm_loadl_pi( cReal, yL );
      __m128 bdad = _mm_mul_ps( cImag, ba );
      __m128 res = _mm_addsub_ps( acbc, bdad );
      __m128 finalRes = _mm_add_ps( res, acc );
      _mm_storel_pi( yL, finalRes );

      --countN;
      pf1 += 2;
      pf2 += 2;
      pAcc += 2;
    }
  }
  return noError;
}

template<>
ErrorCode vectorMultiplyConstantAddInplace<float, Feature::VISR_SIMD_FEATURE>( float constFactor,
					    float const * const factor,
					    float * const accumulator,
					    std::size_t numElements,
					    std::size_t alignment /*= 0*/ )
{
  float const * x = factor;
  float * y = accumulator;
  std::size_t cnt = numElements;
#ifdef __AVX__
  if( cnt >= 8 )
  {
    __m256 c = _mm256_broadcast_ss( &constFactor );
    if( alignment >= 8 )
    {
      while( cnt >= 8 )
      {
        __m256 a = _mm256_load_ps( x );
        x += 8;
        __m256 acc = _mm256_load_ps( y );
        cnt -= 8;
#ifdef __AVX2__
        acc = _mm256_fmadd_ps( a, c, acc ); // Note: this requires FMA
#else
        __m256 mulRes = _mm256_mul_ps( a, c );
        acc = _mm256_add_ps( mulRes, acc );
#endif
        _mm256_store_ps( y, acc );
        y += 8;
      }
    }
    else // Same, but with unaligned load/stores
    {
      while( cnt >= 8 )
      {
        __m256 a = _mm256_loadu_ps( x );
        x += 8;
        __m256 acc = _mm256_loadu_ps( y );
        cnt -= 8;
#ifdef __AVX2__
        acc = _mm256_fmadd_ps( a, c, acc ); // Note: this requires FMA
#else
        __m256 mulRes = _mm256_mul_ps( a, c );
        acc = _mm256_add_ps( mulRes, acc );
#endif
        _mm256_storeu_ps( y, acc );
        y += 8;
      }
    }
  }
  if( cnt == 0 ) // Shorthand exit if #numElements is a multiple of 8.
  {
    return noError;
  }
#endif // __AVX__
  // If not returned by now, this is used throughout the rest of the function. 
  __m128 c = _mm_set_ps1( constFactor );

  while( cnt >= 4 ) // This can happen at most once.
  {
    // Don't distinguish between aligned and unaligned accesses here.
    __m128 a = _mm_loadu_ps( x );
    x += 4;
    __m128 acc = _mm_loadu_ps( y );
    cnt -= 4;
#ifdef __AVX2__
        acc = _mm_fmadd_ps( a, c, acc ); // Note: this requires FMA
#else
        __m128 mulRes = _mm_mul_ps( a, c );
        acc = _mm_add_ps( mulRes, acc );
#endif
    _mm_store_ps( y, acc );
    y += 4;
  }
  while( cnt > 0 ) // Remaining scalar elements
  {
    __m128 a = _mm_load_ss( x );
    ++x;
    __m128 acc = _mm_load_ss( y );
    --cnt;
#ifdef __AVX2__
    acc = _mm_fmadd_ss( a, c, acc ); // Note: this requires FMA
#else
    __m128 mulRes = _mm_mul_ss( a, c );
    acc = _mm_add_ss( mulRes, acc );
#endif
    _mm_store_ss( y, acc );
    ++y;
  }
  return ErrorCode::noError;
}

template<>
ErrorCode vectorMultiplyConstantAddInplace<std::complex<float>, Feature::VISR_SIMD_FEATURE>( std::complex<float> constFactor,
					    std::complex<float> const * const factor,
					    std::complex<float> * const accumulator,
					    std::size_t numElements,
					    std::size_t alignment /*= 0*/ )
{
  float const * x = reinterpret_cast<float const *>(factor);
  float * y = reinterpret_cast<float *>(accumulator);
  std::size_t cnt = numElements;
#ifdef __AVX__
  if( cnt >= 4)
  {
    float const * cPtr = reinterpret_cast<float const *>(&constFactor);
    __m256 cReal = _mm256_broadcast_ss( cPtr );
    __m256 cImag = _mm256_broadcast_ss( cPtr+1 );
    if( alignment >= 4 )
    {
      while( cnt >= 4 )
      {
        __m256 ab = _mm256_load_ps( x );
        __m256 acbc = _mm256_mul_ps( cReal, ab );
        __m256 ba = _mm256_shuffle_ps( ab, ab, 0xB1 );
        __m256 acc = _mm256_load_ps( y );
        __m256 bdad = _mm256_mul_ps( cImag, ba );
        __m256 res = _mm256_addsub_ps( acbc, bdad );
        __m256 fullRes = _mm256_add_ps( res, acc );
        _mm256_store_ps( y, fullRes );
        x += 8;
        y += 8;
        cnt -= 4;
      }
    }
    else // Same, but with unaligned load/stores
    {
      while( cnt >= 4 )
      {
        __m256 ab = _mm256_loadu_ps( x );
        __m256 acbc = _mm256_mul_ps( cReal, ab );
        __m256 ba = _mm256_shuffle_ps( ab, ab, 0xB1 );
        __m256 acc = _mm256_loadu_ps( y );
        __m256 bdad = _mm256_mul_ps( cImag, ba );
        __m256 res = _mm256_addsub_ps( acbc, bdad );
        __m256 fullRes = _mm256_add_ps( res, acc );
        _mm256_storeu_ps( y, fullRes );
        x += 8;
        y += 8;
        cnt -= 4;
      }
    }
  }
  if( cnt == 0 ) // Shorthand exit if #numElements is a multiple of 4.
  {
    return noError;
  }
#endif // __AVX__
  if( (cnt >= 2) and( alignment >= 2 ) ) // Otherwise fall through to the scalar case.
  {
    __m128 cReal = _mm_set_ps1( constFactor.real() );
    __m128 cImag = _mm_set_ps1( constFactor.imag() );
    while( cnt >= 2 )
    {
      __m128 ab = _mm_load_ps( x );
      __m128 acbc = _mm_mul_ps( cReal, ab );
      __m128 ba = _mm_shuffle_ps( ab, ab, 0xB1 );
      __m128 acc = _mm_load_ps( y );
      __m128 bdad = _mm_mul_ps( cImag, ba );
      __m128 res = _mm_addsub_ps( acbc, bdad );
      res = _mm_add_ps( res, acc );
      _mm_store_ps( y, res );
      x += 4;
      y += 4;
      cnt -= 2;
    }
  }
  if( cnt > 0)
  {
    __m128 cReal = _mm_set_ps1( constFactor.real() );
    __m128 cImag = _mm_set_ps1( constFactor.imag() );
    while( cnt > 0 ) // no alignment or remaining samples
    {
      __m64 const * xL = reinterpret_cast<__m64 const*>(x);
      __m64 * yL = reinterpret_cast<__m64*>(y);

      __m128 ab = _mm_loadl_pi( cReal, xL ); // cReal is a dummy, we don't use the upper half of the register.
      __m128 acbc = _mm_mul_ps( cReal, ab );
      __m128 ba = _mm_shuffle_ps( ab, ab, 0xB1 );
      __m128 acc = _mm_loadl_pi( cReal, yL );
      __m128 bdad = _mm_mul_ps( cImag, ba );
      __m128 res = _mm_addsub_ps( acbc, bdad );
      res = _mm_add_ps( res, acc );
      _mm_storel_pi( yL, res );
      x += 2;
      y += 2;
      --cnt;
    }
  }
  return ErrorCode::noError;
}

/// @endcond NEVER

} // namespace intel_x86_64
} // namespace efl
} // namespace visr
