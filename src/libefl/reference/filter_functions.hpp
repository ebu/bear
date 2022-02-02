/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_REFERENCE_FILTER_FUNCTIONS_HPP_INCLUDED
#define VISR_LIBEFL_REFERENCE_FILTER_FUNCTIONS_HPP_INCLUDED

#include "../error_codes.hpp"

#include <cstddef>

namespace visr
{
namespace efl
{
namespace reference
{

template< typename T >
ErrorCode iirFilterBiquadsSingleChannel( T const * input,
                                         T * output,
                                         T * states,
                                         T const * coeffs,
                                         std::size_t numElements,
                                         std::size_t numSections,
                                         std::size_t stateStride,
                                         std::size_t coeffStride,
                                         std::size_t alignment = 0);
  
} // namespace reference
} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_REFERENCE_FILTER_FUNCTIONS_HPP_INCLUDED
