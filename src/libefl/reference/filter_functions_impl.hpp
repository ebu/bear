/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_REFERENCE_FILTER_FUNCTIONS_IMPL_HPP_INCLUDED
#define VISR_LIBEFL_REFERENCE_FILTER_FUNCTIONS_IMPL_HPP_INCLUDED

#include "vector_functions.hpp"

#include "../alignment.hpp"
#include "../error_codes.hpp"

#include <cstddef>

namespace visr
{
namespace efl
{
namespace reference
{
namespace // unnamed
{

template< typename T >
void iirBiquadInternal( T const * input,
  T * output,
  T * states,
  T const * coeffs,
  std::size_t numElements )
{
  // Implement transposed direct form II
  // v0 = b0 * x[n] + v1
  // y[n] = v0
  // v1 = -a1*y[n] + b1 * x[n] + v2
  // v2 = -a2*y[n] + b2 * x[n]

  T v1 = states[0];
  T v2 = states[1];
  for( std::size_t idx{0}; idx < numElements; ++idx )
  {
    T const x{ *input++ };
    T const y{ coeffs[0] * x + v1 };
    v1 = v2 + coeffs[1] * x - coeffs[3] * y;
    v2 = coeffs[2] * x - coeffs[4] * y;
    *output++ = y;
  }
  states[0] = v1;
  states[1] = v2;
}

template< typename T >
void iirBiquadInternalInplace( T * inout,
  T * states,
  T const * coeffs,
  std::size_t numElements  )
{
  // Implement transposed direct form II
  // v0 = b0 * x[n] + v1
  // y[n] = v0
  // v1 = -a1*y[n] + b1 * x[n] + v2
  // v2 = -a2*y[n] + b2 * x[n]

  T v1 = states[0];
  T v2 = states[1];
  for( std::size_t idx{0}; idx < numElements; ++idx )
  {
    T const x{ *inout };
    T const y{ coeffs[0] * x + v1 };
    v1 = v2 + coeffs[1] * x - coeffs[3] * y;
    v2 = coeffs[2] * x - coeffs[4] * y;
    *inout++ = y;
  }
  states[0] = v1;
  states[1] = v2;
}


} // unnamed namespace
 
template< typename T >
ErrorCode iirFilterBiquadsSingleChannel( T const * input,
                                         T * output,
                                         T * states,
                                         T const * coeffs,
                                         std::size_t numElements,
                                         std::size_t numSections,
                                         std::size_t stateStride,
                                         std::size_t coeffStride,
                                         std::size_t alignment = 0)
{
  if( not checkAlignment( input, alignment ) ) return alignmentError;
  if( not checkAlignment( output, alignment ) ) return alignmentError;

  if( numSections == 0 )
  {
    return vectorCopy( input, output, numElements, alignment );
  }
  iirBiquadInternal( input, output, states, coeffs, numElements );
  for( std::size_t secIdx{ 1 }; secIdx < numSections; ++secIdx )
  {
    T * currState{ states + secIdx * stateStride };
    T const * currCoeffs{ coeffs + secIdx * coeffStride };
    iirBiquadInternalInplace( output, currState, currCoeffs, numElements );
  }
  return noError;
}
  
} // namespace reference
} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_REFERENCE_FILTER_FUNCTIONS_IMPL_HPP_INCLUDED
