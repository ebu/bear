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
ErrorCode vectorAdd( float const * const addend1,
		     float const * const addend2,
		     float* const result,
		     std::size_t numElements,
		     std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( addend1, alignment ) ) return alignmentError;
  if( not checkAlignment( addend2, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  // Need extra pointers for iterating because the arguments are const.
  float const * a1Ptr = addend1;
  float const * a2Ptr = addend2;
  float * resPtr = result;

  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t a1 = vld1q_f32( a1Ptr );
      a1Ptr += 4;
      float32x4_t a2 = vld1q_f32( a2Ptr );
      a2Ptr += 4;
      float32x4_t res = vaddq_f32( a1, a2 );
      vst1q_f32( resPtr, res );
      resPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t a1;
    float32x2_t a2;
    if( numElements >= 2 )
    {
      numElements -= 2;
      a1 = vld1_f32( a1Ptr );
      a1Ptr += 2;
      a2 = vld1_f32( a2Ptr );
      a2Ptr += 2;
      float32x2_t res = vadd_f32( a1, a2 );
      vst1_f32( resPtr, res );
      resPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      a1 = vld1_lane_f32( a1Ptr, a1, 0 );
      ++a1Ptr;
      a2 = vld1_lane_f32( a2Ptr, a2, 0 );
      ++a2Ptr;
      float32x2_t res = vadd_f32( a1, a2 );
      vst1_lane_f32( resPtr, res, 0 );
      ++resPtr;
    }
  }
  return noError;
}

template<>
ErrorCode vectorAdd( std::complex<float> const * const addend1,
		     std::complex<float> const * const addend2,
		     std::complex<float> * const result,
		     std::size_t numElements,
		     std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( addend1, alignment ) ) return alignmentError;
  if( not checkAlignment( addend2, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * a1Ptr = reinterpret_cast<float const *>(addend1);
  float const * a2Ptr = reinterpret_cast<float const *>(addend2);
  float * resPtr = reinterpret_cast<float *>(result);

  for(; numElements >=2; numElements -= 2 )
  {
    float32x4_t a1 = vld1q_f32( a1Ptr );
    a1Ptr += 4;
    float32x4_t a2 = vld1q_f32( a2Ptr );
    a2Ptr += 4;
    float32x4_t res = vaddq_f32( a1, a2 );
    vst1q_f32( resPtr, res );
    resPtr += 4;
  }
  if( numElements > 0 )
  {
    --numElements;
    float32x2_t a1 = vld1_f32( a1Ptr );
    a1Ptr += 2;
    float32x2_t a2 = vld1_f32( a2Ptr );
    a2Ptr += 2;
    float32x2_t res = vadd_f32( a1, a2 );
    vst1_f32( resPtr, res );
    resPtr += 2;
  }
  return noError;
}

template<>
ErrorCode vectorAddInplace( float const * const addend1,
                            float * const addend2Result,
                            std::size_t numElements,
                            std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( addend1, alignment ) ) return alignmentError;
  if( not checkAlignment( addend2Result, alignment ) ) return alignmentError;
#endif
  // Need extra pointers for iterating because the arguments are const.
  float const * a1Ptr = addend1;
  float * a2ResPtr = addend2Result;
  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t a1 = vld1q_f32( a1Ptr );
      a1Ptr += 4;
      float32x4_t a2Res = vld1q_f32( a2ResPtr );
      a2Res = vaddq_f32( a1, a2Res );
      vst1q_f32( a2ResPtr, a2Res );
      a2ResPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t a1;
    float32x2_t a2Res;
    if( numElements >= 2 )
    {
      numElements -= 2;
      a1 = vld1_f32( a1Ptr );
      a1Ptr += 2;
      a2Res = vld1_f32( a2ResPtr );
      a2Res = vadd_f32( a1, a2Res );
      vst1_f32( a2ResPtr, a2Res );
      a2ResPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      a1 = vld1_lane_f32( a1Ptr, a1, 0 );
      ++a1Ptr;
      a2Res = vld1_lane_f32( a2ResPtr, a2Res, 0 );
      a2Res = vadd_f32( a1, a2Res );
      vst1_lane_f32( a2ResPtr, a2Res, 0 );
      ++a2ResPtr;
    }
  }
  return noError;
  
}

template<>
ErrorCode vectorAddInplace( std::complex<float> const * const addend1,
                            std::complex<float> * const addend2Result,
                            std::size_t numElements,
                            std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( addend1, alignment ) ) return alignmentError;
  if( not checkAlignment( addend2Result, alignment ) ) return alignmentError;
#endif
  float const * a1Ptr = reinterpret_cast<float const *>(addend1);
  float * a2resPtr = reinterpret_cast<float *>(addend2Result);

  for(; numElements >=2; numElements -= 2 )
  {
    float32x4_t a1 = vld1q_f32( a1Ptr );
    a1Ptr += 4;
    float32x4_t a2 = vld1q_f32( a2resPtr );
    float32x4_t res = vaddq_f32( a1, a2 );
    vst1q_f32( a2resPtr, res );
    a2resPtr += 4;
  }
  if( numElements > 0 )
  {
    float32x2_t a1 = vld1_f32( a1Ptr );
    a1Ptr += 2;
    float32x2_t a2 = vld1_f32( a2resPtr );
    float32x2_t res = vadd_f32( a1, a2 );
    vst1_f32( a2resPtr, res );
    a2resPtr += 2;
  }
  return noError;
}

template<>
ErrorCode vectorAddConstant( float constantValue,
                             float const * const addend,
                             float * const result,
                             std::size_t numElements,
                             std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( addend, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * addendPtr = static_cast<float const *>(addend);
  float * resPtr = static_cast<float *>(result);

  float32x4_t cf = {constantValue, constantValue, constantValue, constantValue };
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4_t addend = vld1q_f32( addendPtr );
    addendPtr += 4;
    float32x4_t res = vaddq_f32( addend, cf );
    vst1q_f32( resPtr, res );
    resPtr += 4;
  }
  if( numElements > 0 )
  {
    float32x2_t cf = {constantValue, constantValue};
    float32x2_t addend;
    if( numElements >= 2 )
    {
      numElements -= 2;
      addend = vld1_f32( addendPtr );
      addendPtr += 2;
      float32x2_t res = vadd_f32( cf, addend );
      vst1_f32( resPtr, res );
      resPtr += 2;
    }
    if( numElements > 0 )
    {
      addend = vld1_lane_f32( addendPtr, addend, 0 );
      addendPtr += 1;
      float32x2_t res = vadd_f32( cf, addend );
      vst1_lane_f32( resPtr, res, 0 );
      resPtr += 1;
    }
  }
  return noError;
}

template<>
ErrorCode vectorAddConstant( std::complex<float> constantValue,
                             std::complex<float> const * const addend,
                             std::complex<float> * const result,
                             std::size_t numElements,
                             std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( addend, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * addendPtr = reinterpret_cast<float const *>(addend);
  float * resultPtr = reinterpret_cast<float *>(result);
  float const rV = constantValue.real();
  float const iV = constantValue.imag();
  float32x4_t cf4 = {rV,iV,rV,iV};
  for(; numElements >=2; numElements -= 2 )
  {
    float32x4_t a1 = vld1q_f32( addendPtr );
    addendPtr += 4;
    float32x4_t res = vaddq_f32( a1, cf4 );
    vst1q_f32( resultPtr, res );
    resultPtr += 4;
  }
  if( numElements > 0 )
  {
    --numElements;
    float32x2_t a1 = vld1_f32( addendPtr );
    addendPtr += 2;
    float32x2_t cf2 = {rV,iV};
    float32x2_t res = vadd_f32( a1, cf2 );
    vst1_f32( resultPtr, res );
    resultPtr += 2;
  }
  return noError;
}

template<>
ErrorCode vectorAddConstantInplace(float constantValue,
				   float * const addendResult,
				   std::size_t numElements,
				   std::size_t alignment /*= 0*/)
{
#ifndef NDEBUG
  if( not checkAlignment( addendResult, alignment ) ) return alignmentError;
#endif
  float * addendResultPtr = static_cast<float *>(addendResult);

  float32x4_t cf4 = {constantValue, constantValue, constantValue, constantValue };
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4_t addend = vld1q_f32( addendResultPtr );
    float32x4_t res = vaddq_f32( addend, cf4 );
    vst1q_f32( addendResultPtr, res );
    addendResultPtr += 4;
  }
  if( numElements > 0 )
  {
    float32x2_t cf2 = {constantValue, constantValue};
    float32x2_t add;
    if( numElements >= 2 )
    {
      numElements -= 2;
      add = vld1_f32( addendResultPtr );
      float32x2_t res = vadd_f32( cf2, add );
      vst1_f32( addendResultPtr, res );
      addendResultPtr += 2;
    }
    if( numElements > 0 )
    {
      add = vld1_lane_f32( addendResultPtr, add, 0 );
      float32x2_t res = vadd_f32( cf2, add );
      vst1_lane_f32( addendResultPtr, res, 0 );
      addendResultPtr += 1;
    }
  }
  return noError;
}

template<>
ErrorCode vectorAddConstantInplace(std::complex<float> constantValue,
				   std::complex<float> * const addendResult,
				   std::size_t numElements,
				   std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( addendResult, alignment ) ) return alignmentError;
#endif
  float * addendResultPtr = reinterpret_cast<float *>(addendResult);
  float const rV = constantValue.real();
  float const iV = constantValue.imag();
  float32x4_t cf4 = {rV,iV,rV,iV};
  for(; numElements >= 2; numElements -= 2 )
  {
    float32x4_t addend = vld1q_f32( addendResultPtr );
    float32x4_t res = vaddq_f32( cf4, addend );
    vst1q_f32( addendResultPtr, res );
    addendResultPtr += 4;
  }
  if( numElements > 0 )
  {
    float32x2_t cf2 = {rV,iV};
    float32x2_t addend = vld1_f32( addendResultPtr );
    float32x2_t res = vadd_f32( cf2, addend );
    vst1_f32( addendResultPtr, res );
    addendResultPtr += 2;
  }
  return noError;
}

} // namespace armv7l_neon_32bit
} // namespace efl
} // namespace visr
