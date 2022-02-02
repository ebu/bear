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
ErrorCode vectorMultiplyAdd( float const * const factor1,
			     float const * const factor2,
			     float const * const addend,
			     float * const result,
			     std::size_t numElements,
			     std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( addend, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * f1Ptr = const_cast<float const *>(factor1);
  float const * f2Ptr = const_cast<float const *>(factor2);
  float const * addendPtr = const_cast<float *>(addend);
  float * resultPtr = const_cast<float *>(result);

  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t f1 = vld1q_f32( f1Ptr );
      f1Ptr += 4;
      float32x4_t f2 = vld1q_f32( f2Ptr );
      f2Ptr += 4;
      float32x4_t acc = vld1q_f32( addendPtr );
      addendPtr += 4;
      acc = vmlaq_f32( acc, f1, f2 );
      vst1q_f32( resultPtr, acc );
      resultPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t f1;
    float32x2_t f2;
    float32x2_t acc;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld1_f32( f1Ptr );
      f1Ptr += 2;
      f2 = vld1_f32( f2Ptr );
      f2Ptr += 2;
      acc  = vld1_f32( addendPtr );
      addendPtr += 2;
      acc =  vmla_f32( acc, f1, f2 );
      vst1_f32( resultPtr, acc );
      resultPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      f1 = vld1_lane_f32( f1Ptr, f1, 0 );
      ++f1Ptr;
      f2 = vld1_lane_f32( f2Ptr, f2, 0 );
      ++f2Ptr;
      acc = vld1_lane_f32( addendPtr, acc, 0 );
      ++addendPtr;
      acc = vmla_f32( acc, f1, f2 );
      vst1_lane_f32( resultPtr, acc, 0 );
      ++resultPtr;
    }
  }
  return noError;
}

template<>
ErrorCode vectorMultiplyAdd( std::complex<float> const * const factor1,
			     std::complex<float> const * const factor2,
			     std::complex<float> const * const addend,
			     std::complex<float> * const result,
			     std::size_t numElements,
			     std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( addend, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * f1Ptr = reinterpret_cast<float const *>(factor1);
  float const * f2Ptr = reinterpret_cast<float const *>(factor2);
  float const * addendPtr = reinterpret_cast<float const *>(addend);
  float * resultPtr = reinterpret_cast<float *>(result);

  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t f1 = vld2q_f32( f1Ptr );
    f1Ptr += 8;
    float32x4x2_t f2 = vld2q_f32( f2Ptr );
    f2Ptr += 8;
    float32x4x2_t res = vld2q_f32( addendPtr );
    addendPtr += 8;
    res.val[0] = vmlaq_f32( res.val[0], f1.val[0], f2.val[0] );
    res.val[1] = vmlaq_f32( res.val[1], f1.val[0], f2.val[1] );
    res.val[0] = vmlsq_f32( res.val[0], f1.val[1], f2.val[1] );
    res.val[1] = vmlaq_f32( res.val[1], f1.val[1], f2.val[0] );
    vst2q_f32( resultPtr, res );
    resultPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t f1;
    float32x2x2_t f2;
    float32x2x2_t res;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld2_f32( f1Ptr );
      f1Ptr += 4;
      f2 = vld2_f32( f2Ptr );
      f2Ptr += 4;
      res = vld2_f32( addendPtr );
      addendPtr += 4;
      res.val[0] = vmla_f32( res.val[0], f1.val[0], f2.val[0] );
      res.val[1] = vmla_f32( res.val[1], f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_f32( resultPtr, res );
      resultPtr += 4;
    }
    if( numElements > 0 )
    {
      f1 = vld2_lane_f32( f1Ptr, f1, 0 );
      f1Ptr += 2;
      f2 = vld2_lane_f32( f2Ptr, f2, 0 );
      f2Ptr += 2;
      res = vld2_lane_f32( addendPtr, res, 0 );
      addendPtr += 2;
      res.val[0] = vmla_f32( res.val[0], f1.val[0], f2.val[0] );
      res.val[1] = vmla_f32( res.val[1], f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_lane_f32( resultPtr, res, 0 );
      resultPtr += 2;
    }
  }
  return noError;
}
  
template<>
ErrorCode vectorMultiplyAddInplace( float const * const factor1,
				    float const * const factor2,
				    float * const accumulator,
				    std::size_t numElements,
				    std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( accumulator, alignment ) ) return alignmentError;
#endif
  float const * f1Ptr = static_cast<float const *>(factor1);
  float const * f2Ptr = static_cast<float const *>(factor2);
  float * accPtr = static_cast<float *>(accumulator);

  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t f1 = vld1q_f32( f1Ptr );
      f1Ptr += 4;
      float32x4_t f2 = vld1q_f32( f2Ptr );
      f2Ptr += 4;
      float32x4_t acc = vld1q_f32( accPtr );
      acc = vmlaq_f32( acc, f1, f2 );
      vst1q_f32( accPtr, acc );
      accPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t f1;
    float32x2_t f2;
    float32x2_t acc;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld1_f32( f1Ptr );
      f1Ptr += 2;
      f2 = vld1_f32( f2Ptr );
      f2Ptr += 2;
      acc  = vld1_f32( accPtr );
      acc =  vmla_f32( acc, f1, f2 );
      vst1_f32( accPtr, acc );
      accPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      f1 = vld1_lane_f32( f1Ptr, f1, 0 );
      ++f1Ptr;
      f2 = vld1_lane_f32( f2Ptr, f2, 0 );
      ++f2Ptr;
      acc = vld1_lane_f32( accPtr, acc, 0 );
      acc = vmla_f32( acc, f1, f2 );
      vst1_lane_f32( accPtr, acc, 0 );
      ++accPtr;
    }
  }
  return noError;
}

template<>
ErrorCode vectorMultiplyAddInplace( std::complex<float> const * const factor1,
				    std::complex<float> const * const factor2,
				    std::complex<float> * const accumulator,
				    std::size_t numElements,
				    std::size_t alignment )
{
  if( not checkAlignment( factor1, alignment ) ) return alignmentError;
  if( not checkAlignment( factor2, alignment ) ) return alignmentError;
  if( not checkAlignment( accumulator, alignment ) ) return alignmentError;

  float const * f1Ptr = reinterpret_cast<float const *>(factor1);
  float const * f2Ptr = reinterpret_cast<float const *>(factor2);
  float * accPtr = reinterpret_cast<float *>(accumulator);
  
  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t f1 = vld2q_f32( f1Ptr );
    f1Ptr += 8;
    float32x4x2_t f2 = vld2q_f32( f2Ptr );
    f2Ptr += 8;
    float32x4x2_t res = vld2q_f32( accPtr );
    res.val[0] = vmlaq_f32( res.val[0], f1.val[0], f2.val[0] );
    res.val[1] = vmlaq_f32( res.val[1], f1.val[0], f2.val[1] );
    res.val[0] = vmlsq_f32( res.val[0], f1.val[1], f2.val[1] );
    res.val[1] = vmlaq_f32( res.val[1], f1.val[1], f2.val[0] );
    vst2q_f32( accPtr, res );
    accPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t f1;
    float32x2x2_t f2;
    float32x2x2_t res;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld2_f32( f1Ptr );
      f1Ptr += 4;
      f2 = vld2_f32( f2Ptr );
      f2Ptr += 4;
      res = vld2_f32( accPtr );
      res.val[0] = vmla_f32( res.val[0], f1.val[0], f2.val[0] );
      res.val[1] = vmla_f32( res.val[1], f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_f32( accPtr, res );
      accPtr += 4;
    }
    if( numElements > 0 )
    {
      f1 = vld2_lane_f32( f1Ptr, f1, 0 );
      f1Ptr += 2;
      f2 = vld2_lane_f32( f2Ptr, f2, 0 );
      f2Ptr += 2;
      res = vld2_lane_f32( accPtr, res, 0 );
      res.val[0] = vmla_f32( res.val[0], f1.val[0], f2.val[0] );
      res.val[1] = vmla_f32( res.val[1], f1.val[0], f2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], f2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], f2.val[0] );
      vst2_lane_f32( accPtr, res, 0 );
      accPtr += 2;
    }
  }

  return noError;
}

template<>
ErrorCode vectorMultiplyConstantAdd( float constFactor,
				     float const * const factor,
				     float const * const addend,
				     float * const result,
				     std::size_t numElements,
				     std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor, alignment ) ) return alignmentError;
  if( not checkAlignment( addend, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * factorPtr = static_cast<float const *>(factor);
  float const * addendPtr = static_cast<float const *>(addend);
  float * resultPtr = static_cast<float *>(result);

  float32x4_t cf4 = { constFactor, constFactor, constFactor, constFactor };
  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t f1 = vld1q_f32( factorPtr );
      factorPtr += 4;
      float32x4_t acc = vld1q_f32( addendPtr );
      addendPtr += 4;
      acc = vmlaq_f32( acc, f1, cf4 );
      vst1q_f32( resultPtr, acc );
      resultPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t f1;
    float32x2_t cf2 = {constFactor, constFactor};
    float32x2_t acc;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld1_f32( factorPtr );
      factorPtr += 2;
      acc  = vld1_f32( addendPtr );
      addendPtr += 2;
      acc =  vmla_f32( acc, f1, cf2 );
      vst1_f32( resultPtr, acc );
      resultPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      f1 = vld1_lane_f32( factorPtr, f1, 0 );
      ++factorPtr;
      acc = vld1_lane_f32( addendPtr, acc, 0 );
      ++addendPtr;
      acc = vmla_f32( acc, f1, cf2 );
      vst1_lane_f32( resultPtr, acc, 0 );
      ++resultPtr;
    }
  }
  return noError;
}
  
template<>
ErrorCode vectorMultiplyConstantAdd( std::complex<float> constFactor,
				     std::complex<float> const * const factor,
				     std::complex<float> const * const addend,
				     std::complex<float> * const result,
				     std::size_t numElements,
				     std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor, alignment ) ) return alignmentError;
  if( not checkAlignment( addend, alignment ) ) return alignmentError;
  if( not checkAlignment( result, alignment ) ) return alignmentError;
#endif
  float const * factorPtr = reinterpret_cast<float const *>(factor);
  float const * addendPtr = reinterpret_cast<float const *>(addend);
  float * resultPtr = reinterpret_cast<float *>(result);

  float const rV = constFactor.real();
  float const iV = constFactor.imag();
  float32x4x2_t cf4 = {{{rV,rV,rV,rV},{iV,iV,iV,iV}}};

  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t factor = vld2q_f32( factorPtr );
    factorPtr += 8;
    float32x4x2_t res = vld2q_f32( addendPtr );
    addendPtr += 8;
    res.val[0] = vmlaq_f32( res.val[0], factor.val[0], cf4.val[0] );
    res.val[1] = vmlaq_f32( res.val[1], factor.val[0], cf4.val[1] );
    res.val[0] = vmlsq_f32( res.val[0], factor.val[1], cf4.val[1] );
    res.val[1] = vmlaq_f32( res.val[1], factor.val[1], cf4.val[0] );
    vst2q_f32( resultPtr, res );
    resultPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t cf2 = {{{rV,rV},{iV,iV}}};
    float32x2x2_t f1;
    float32x2x2_t res;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld2_f32( factorPtr );
      factorPtr += 4;
      res = vld2_f32( addendPtr );
      addendPtr += 4;
      res.val[0] = vmla_f32( res.val[0], f1.val[0], cf2.val[0] );
      res.val[1] = vmla_f32( res.val[1], f1.val[0], cf2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], cf2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], cf2.val[0] );
      vst2_f32( resultPtr, res );
      resultPtr += 4;
    }
    if( numElements > 0 )
    {
      f1 = vld2_lane_f32( factorPtr, f1, 0 );
      factorPtr += 2;
      res = vld2_lane_f32( addendPtr, res, 0 );
      addendPtr += 2;
      res.val[0] = vmla_f32( res.val[0], f1.val[0], cf2.val[0] );
      res.val[1] = vmla_f32( res.val[1], f1.val[0], cf2.val[1] );
      res.val[0] = vmls_f32( res.val[0], f1.val[1], cf2.val[1] );
      res.val[1] = vmla_f32( res.val[1], f1.val[1], cf2.val[0] );
      vst2_lane_f32( resultPtr, res, 0 );
      resultPtr += 2;
    }
  }
  return noError;
}
  
template<>
ErrorCode vectorMultiplyConstantAddInplace( float constFactor,
					    float const * const factor,
					    float * const accumulator,
					    std::size_t numElements,
					    std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor, alignment ) ) return alignmentError;
  if( not checkAlignment( accumulator, alignment ) ) return alignmentError;
#endif
  float const * factorPtr = static_cast<float const *>(factor);
  float * accPtr = static_cast<float *>(accumulator);

  float32x4_t cf4 = {constFactor, constFactor, constFactor, constFactor };
  if( numElements >= 4 )
  {
    for(; numElements >= 4; numElements -= 4 )
    {
      float32x4_t f1 = vld1q_f32( factorPtr );
      factorPtr += 4;
      float32x4_t acc = vld1q_f32( accPtr );
      acc = vmlaq_f32( acc, f1, cf4 );
      vst1q_f32( accPtr, acc );
      accPtr += 4;
    }
  }
  if( numElements > 0 )
  {
    // Calculate the remaining elements.
    float32x2_t f1;
    float32x2_t cf2  = {constFactor, constFactor};
    float32x2_t acc;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld1_f32( factorPtr );
      factorPtr += 2;
      acc  = vld1_f32( accPtr );
      acc =  vmla_f32( acc, f1, cf2 );
      vst1_f32( accPtr, acc );
      accPtr += 2;
    }
    if( numElements > 0 )
    {
      --numElements;
      f1 = vld1_lane_f32( factorPtr, f1, 0 );
      ++factorPtr;
      acc = vld1_lane_f32( accPtr, acc, 0 );
      acc = vmla_f32( acc, f1, cf2 );
      vst1_lane_f32( accPtr, acc, 0 );
      ++accPtr;
    }
  }
  return ErrorCode::noError;
}

template<>
ErrorCode vectorMultiplyConstantAddInplace( std::complex<float> constFactor,
					    std::complex<float> const * const factor,
					    std::complex<float> * const accumulator,
					    std::size_t numElements,
					    std::size_t alignment /*= 0*/ )
{
#ifndef NDEBUG
  if( not checkAlignment( factor, alignment ) ) return alignmentError;
  if( not checkAlignment( accumulator, alignment ) ) return alignmentError;
#endif
  float const * factorPtr = reinterpret_cast<float const *>(factor);
  float * accPtr = reinterpret_cast<float *>(accumulator);

  float const rV = constFactor.real();
  float const iV = constFactor.imag();
  float32x4x2_t cf4 = {{{rV,rV,rV,rV},{iV,iV,iV,iV}}};

  for(; numElements >=4; numElements -= 4 )
  {
    float32x4x2_t f1 = vld2q_f32( factorPtr );
    factorPtr += 8;
    float32x4x2_t acc = vld2q_f32( accPtr );
    acc.val[0] = vmlaq_f32( acc.val[0], f1.val[0], cf4.val[0] );
    acc.val[1] = vmlaq_f32( acc.val[1], f1.val[0], cf4.val[1] );
    acc.val[0] = vmlsq_f32( acc.val[0], f1.val[1], cf4.val[1] );
    acc.val[1] = vmlaq_f32( acc.val[1], f1.val[1], cf4.val[0] );
    vst2q_f32( accPtr, acc );
    accPtr += 8;
  }
  if( numElements > 0 )
  {
    float32x2x2_t cf2 = {{{rV,rV},{iV,iV}}};
    float32x2x2_t f1;
    float32x2x2_t acc;
    if( numElements >= 2 )
    {
      numElements -= 2;
      f1 = vld2_f32( factorPtr );
      factorPtr += 4;
      acc = vld2_f32( accPtr );
      acc.val[0] = vmla_f32( acc.val[0], f1.val[0], cf2.val[0] );
      acc.val[1] = vmla_f32( acc.val[1], f1.val[0], cf2.val[1] );
      acc.val[0] = vmls_f32( acc.val[0], f1.val[1], cf2.val[1] );
      acc.val[1] = vmla_f32( acc.val[1], f1.val[1], cf2.val[0] );
      vst2_f32( accPtr, acc );
      accPtr += 4;
    }
    if( numElements > 0 )
    {
      f1 = vld2_lane_f32( factorPtr, f1, 0 );
      factorPtr += 2;
      acc = vld2_lane_f32( accPtr, acc, 0 );
      acc.val[0] = vmla_f32( acc.val[0], f1.val[0], cf2.val[0] );
      acc.val[1] = vmla_f32( acc.val[1], f1.val[0], cf2.val[1] );
      acc.val[0] = vmls_f32( acc.val[0], f1.val[1], cf2.val[1] );
      acc.val[1] = vmla_f32( acc.val[1], f1.val[1], cf2.val[0] );
      vst2_lane_f32( accPtr, acc, 0 );
      accPtr += 2;
    }
  }
  return noError;
}

} // namespace armv7l_neon_32bit
} // namespace efl
} // namespace visr
