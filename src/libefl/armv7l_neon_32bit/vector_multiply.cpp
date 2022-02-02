/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "vector_functions.hpp"

#include "../alignment.hpp"

// Intrinsics
#include <arm_neon.h>

namespace visr
{
namespace efl
{
namespace armv7l_neon_32bit
{

template<>
ErrorCode vectorMultiply( float const * const factor1,
			  float const * const factor2,
			  float * const result,
			  std::size_t numElements,
			  std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  // Need extra pointers for iterating because the arguments are const.
  float const * f1Ptr = factor1;
  float const * f2Ptr = factor2;
  float * resPtr = result;

  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t f1 = vld1q_f32( f1Ptr );
      f1Ptr += 4;
      float32x4_t f2 = vld1q_f32( f2Ptr );
      f2Ptr += 4;
      float32x4_t res = vmulq_f32( f1, f2 );
      vst1q_f32( resPtr, res );
      resPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t f1;
    float32x2_t f2;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld1_f32( f1Ptr );
      f1Ptr += 2;
      f2 = vld1_f32( f2Ptr );
      f2Ptr += 2;
      float32x2_t res = vmul_f32( f1, f2 );
      vst1_f32( resPtr, res );
      resPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      f1 = vld1_lane_f32( f1Ptr, f1, 0 );
      ++f1Ptr;
      f2 = vld1_lane_f32( f2Ptr, f2, 0 );
      ++f2Ptr;
      float32x2_t res = vmul_f32( f1, f2 );
      vst1_lane_f32( resPtr, res, 0 );
      ++resPtr;
    }
  }
  return noError;
}

template<>
ErrorCode vectorMultiply( std::complex<float> const * const factor1,
			  std::complex<float> const * const factor2,
			  std::complex<float> * const result,
			  std::size_t numElements,
			  std::size_t alignment )
{
#ifndef NDEBUG
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * f1Ptr = reinterpret_cast<float const *>(factor1);
  float const * f2Ptr = reinterpret_cast<float const *>(factor2);
  float * resPtr = reinterpret_cast<float *>(result);
#if 0 // Apparently the complex instructions are available only in ARM8 (64 bit?)
  for(; numElements >=2; numElements -= 2 )
  {
    float32x4_t f1 = vld1q_f32( f1Ptr );
    f1Ptr += 4;
    float32x4_t f2 = vld1q_f32( f2Ptr );
    f2Ptr += 4;
    float32x4_t res = vcmlaq_f32( f1, f2 );
    vst1q_f32( resPtr, res );
    resPtr += 4;
  }
  if( numElements > 0 )
  {
    float32x2_t f1 = vld1_f32( f1Ptr );
    float32x2_t f2 = vld1_f32( f2Ptr );
    float32x2_t res = vcmla_f32( f1, f2 );
    vst1_f32( resPtr, res );
  }
#else
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t f1 = vld2q_f32( f1Ptr );
    f1Ptr += 8;
    float32x4x2_t f2 = vld2q_f32( f2Ptr );
    f2Ptr += 8;
    float32x4x2_t res;
    res.val[0] = vmulq_f32( f1.val[0], f2.val[0] );
    res.val[1] = vmulq_f32( f1.val[0], f2.val[1] );
    res.val[0] = vmlsq_f32( res.val[0], f1.val[1], f2.val[1] );
    res.val[1] = vmlaq_f32( res.val[1], f1.val[1], f2.val[0] );
    vst2q_f32( resPtr, res );
    resPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t f1;
    float32x2x2_t f2;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld2_f32( f1Ptr );
      f1Ptr += 4;
      f2 = vld2_f32( f2Ptr );
      f2Ptr += 4;
      float32x2x2_t res;
      res.val[0] = vmul_f32( f1.val[0], f2.val[0] );
      res.val[1] = vmul_f32( f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_f32( resPtr, res );
      resPtr += 4;
    }
    if( numElements > 0 )
    {
      f1 = vld2_lane_f32( f1Ptr, f1, 0 );
      f1Ptr += 2;
      f2 = vld2_lane_f32( f2Ptr, f2, 0 );
      f2Ptr += 2;
      float32x2x2_t res;
      res.val[0] = vmul_f32( f1.val[0], f2.val[0] );
      res.val[1] = vmul_f32( f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_lane_f32( resPtr, res, 0 );
      resPtr += 2;
    }
  }
#endif
  return noError;
}

template<>
ErrorCode vectorMultiplyInplace( float const * const factor1,
				 float * const factor2Result,
				 std::size_t numElements,
				 std::size_t alignment )
{
#ifndef NDEBUG
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2Result, alignment ) ) return alignmentError;
#endif
  // Need extra pointers for iterating because the arguments are const.
  float const * f1Ptr = factor1;
  float * f2ResPtr = factor2Result;
  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t f1 = vld1q_f32( f1Ptr );
      f1Ptr += 4;
      float32x4_t f2Res = vld1q_f32( f2ResPtr );
      f2Res = vmulq_f32( f1, f2Res );
      vst1q_f32( f2ResPtr, f2Res );
      f2ResPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t f1;
    float32x2_t f2Res;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld1_f32( f1Ptr );
      f1Ptr += 2;
      f2Res = vld1_f32( f2ResPtr );
      f2Res = vmul_f32( f1, f2Res );
      vst1_f32( f2ResPtr, f2Res );
      f2ResPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      f1 = vld1_lane_f32( f1Ptr, f1, 0 );
      ++f1Ptr;
      f2Res = vld1_lane_f32( f2ResPtr, f2Res, 0 );
      f2Res = vmul_f32( f1, f2Res );
      vst1_lane_f32( f2ResPtr, f2Res, 0 );
      ++f2ResPtr;
    }
  }
  return noError;
  
}

template<>
ErrorCode vectorMultiplyInplace( std::complex<float> const * const factor1,
				 std::complex<float> * const factor2Result,
				 std::size_t numElements,
				 std::size_t alignment )
{
#ifndef NDEBUG
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2Result, alignment ) ) return alignmentError;
#endif
  float const * f1Ptr = reinterpret_cast<float const *>(factor1);
  float * f2resPtr = reinterpret_cast<float *>(factor2Result);

  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t f1 = vld2q_f32( f1Ptr );
    f1Ptr += 8;
    float32x4x2_t f2 = vld2q_f32( f2resPtr );
    float32x4x2_t res;
    res.val[0] = vmulq_f32( f1.val[0], f2.val[0] );
    res.val[1] = vmulq_f32( f1.val[0], f2.val[1] );
    res.val[0] = vmlsq_f32( res.val[0], f1.val[1], f2.val[1] );
    res.val[1] = vmlaq_f32( res.val[1], f1.val[1], f2.val[0] );
    vst2q_f32( f2resPtr, res );
    f2resPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t f1;
    float32x2x2_t f2;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld2_f32( f1Ptr );
      f1Ptr += 4;
      f2 = vld2_f32( f2resPtr );
      float32x2x2_t res;
      res.val[0] = vmul_f32( f1.val[0], f2.val[0] );
      res.val[1] = vmul_f32( f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_f32( f2resPtr, res );
      f2resPtr += 4;
    }
    if( numElements > 0 )
    {
      f1 = vld2_lane_f32( f1Ptr, f1, 0 );
      f1Ptr += 2;
      f2 = vld2_lane_f32( f2resPtr, f2, 0 );
      float32x2x2_t res;
      res.val[0] = vmul_f32( f1.val[0], f2.val[0] );
      res.val[1] = vmul_f32( f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_lane_f32( f2resPtr, res, 0 );
      f2resPtr += 2;
    }
  }
  return noError;
}

template<>
ErrorCode vectorMultiplyConstant( float constantValue,
                                  float const * const factor,
                                  float * const result,
                                  std::size_t numElements,
                                  std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * factorPtr = static_cast<float const *>(factor);
  float * resPtr = static_cast<float *>(result);

  float32x4_t cf = {constantValue, constantValue, constantValue, constantValue };
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4_t factor = vld1q_f32( factorPtr );
    factorPtr += 4;
    float32x4_t res = vmulq_f32( factor, cf );
    vst1q_f32( resPtr, res );
    resPtr += 4;
  }
  if( numElements > 0 )
  {
    float32x2_t cf = {constantValue, constantValue};
    float32x2_t factor;
    if( numElements >= 2 )
    {
      numElements -= 2;
      factor = vld1_f32( factorPtr );
      factorPtr += 2;
      float32x2_t res = vmul_f32( cf, factor );
      vst1_f32( resPtr, res );
      resPtr += 2;
    }
    if( numElements > 0 )
    {
      factor = vld1_lane_f32( factorPtr, factor, 0 );
      factorPtr += 1;
      float32x2_t res = vmul_f32( cf, factor );
      vst1_lane_f32( resPtr, res, 0 );
      resPtr += 1;
    }
  }
  return noError;
}


template<>
ErrorCode vectorMultiplyConstant( std::complex<float> constantValue,
                                  std::complex<float> const * const factor,
                                  std::complex<float> * const result,
                                  std::size_t numElements,
                                  std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * factorPtr = reinterpret_cast<float const *>(factor);
  float * resultPtr = reinterpret_cast<float *>(result);
  float const rV = constantValue.real();
  float const iV = constantValue.imag();
  float32x4x2_t cf4 = {{{rV,rV,rV,rV},{iV,iV,iV,iV}}};
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t f1 = vld2q_f32( factorPtr );
    factorPtr += 8;
    float32x4x2_t res;
    res.val[0] = vmulq_f32( f1.val[0], cf4.val[0] );
    res.val[1] = vmulq_f32( f1.val[0], cf4.val[1] );
    res.val[0] = vmlsq_f32( res.val[0], f1.val[1], cf4.val[1] );
    res.val[1] = vmlaq_f32( res.val[1], f1.val[1], cf4.val[0] );
    vst2q_f32( resultPtr, res );
    resultPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t f1;
    float32x2x2_t cf2 = {{{rV,rV},{iV,iV}}};
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld2_f32( factorPtr );
      factorPtr += 4;
      float32x2x2_t res;
      res.val[0] = vmul_f32( f1.val[0], cf2.val[0] );
      res.val[1] = vmul_f32( f1.val[0], cf2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], cf2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], cf2.val[0] );
      vst2_f32( resultPtr, res );
      resultPtr += 4;
    }
    if( numElements > 0 )
    {
      f1 = vld2_lane_f32( factorPtr, f1, 0 );
      factorPtr += 2;
      float32x2x2_t res;
      res.val[0] = vmul_f32( f1.val[0], cf2.val[0] );
      res.val[1] = vmul_f32( f1.val[0], cf2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], cf2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], cf2.val[0] );
      vst2_lane_f32( resultPtr, res, 0 );
      resultPtr += 2;
    }
  }

  return noError;
}

template<>
ErrorCode vectorMultiplyConstantInplace( float constantValue,
                                         float * const factorResult,
                                         std::size_t numElements,
                                         std::size_t alignment /*= 0*/)
{
#ifndef NDEBUG
  if( not checkAlignment( factorResult, alignment ) ) return alignmentError;
#endif
  float * factorResultPtr = static_cast<float *>(factorResult);

  float32x4_t cf = {constantValue, constantValue, constantValue, constantValue };
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4_t facRes = vld1q_f32( factorResultPtr );
    facRes = vmulq_f32( facRes, cf );
    vst1q_f32( factorResultPtr, facRes );
    factorResultPtr += 4;
  }
  if( numElements > 0 )
  {
    float32x2_t cf = {constantValue, constantValue};
    float32x2_t facRes;
    if( numElements >= 2 )
    {
      numElements -= 2;
      facRes = vld1_f32( factorResultPtr );
      facRes = vmul_f32( cf, facRes );
      vst1_f32( factorResultPtr, facRes );
      factorResultPtr += 2;
    }
    if( numElements > 0 )
    {
      facRes = vld1_lane_f32( factorResultPtr, facRes, 0 );
      facRes = vmul_f32( cf, facRes );
      vst1_lane_f32( factorResultPtr, facRes, 0 );
      factorResultPtr += 1;
    }
  }
  return noError;
}

template<>
ErrorCode vectorMultiplyConstantInplace( std::complex<float> constantValue,
                                         std::complex<float> * const factorResult,
                                         std::size_t numElements,
                                         std::size_t alignment /*= 0*/)
{
#ifndef NDEBUG
  if( not checkAlignment( factorResult, alignment ) ) return alignmentError;
#endif
  float * factorResultPtr = reinterpret_cast<float *>(factorResult);
  float const rV = constantValue.real();
  float const iV = constantValue.imag();
  float32x4x2_t cf4 = {{{rV,rV,rV,rV},{iV,iV,iV,iV}}};
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t factor = vld2q_f32( factorResultPtr );
    float32x4x2_t res;
    res.val[0] = vmulq_f32( factor.val[0], cf4.val[0] );
    res.val[1] = vmulq_f32( factor.val[0], cf4.val[1] );
    res.val[0] = vmlsq_f32( res.val[0], factor.val[1], cf4.val[1] );
    res.val[1] = vmlaq_f32( res.val[1], factor.val[1], cf4.val[0] );
    vst2q_f32( factorResultPtr, res );
    factorResultPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t factor;
    float32x2x2_t cf2 = {{{rV,rV},{iV,iV}}};
    if( numElements >= 2 )
    {
      numElements -= 2;
      factor = vld2_f32( factorResultPtr );
      float32x2x2_t res;
      res.val[0] = vmul_f32( factor.val[0], cf2.val[0] );
      res.val[1] = vmul_f32( factor.val[0], cf2.val[1] );
      res.val[0] = vmls_f32( res.val[0], factor.val[1], cf2.val[1] );
      res.val[1] = vmla_f32( res.val[1], factor.val[1], cf2.val[0] );
      vst2_f32( factorResultPtr, res );
      factorResultPtr += 4;
    }
    if( numElements > 0 )
    {
      factor = vld2_lane_f32( factorResultPtr, factor, 0 );
      float32x2x2_t res;
      res.val[0] = vmul_f32( factor.val[0], cf2.val[0] );
      res.val[1] = vmul_f32( factor.val[0], cf2.val[1] );
      res.val[0] = vmls_f32( res.val[0], factor.val[1], cf2.val[1] );
      res.val[1] = vmla_f32( res.val[1], factor.val[1], cf2.val[0] );
      vst2_lane_f32( factorResultPtr, res, 0 );
      factorResultPtr += 2;
    }
  }
  return noError;
}
  
} // namespace armv7l_neon_32bit
} // namespace efl
} // namespace visr
