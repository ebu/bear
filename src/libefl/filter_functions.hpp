/* Copyright AudioScenic Ltd 2019 - All rights reserved */
/* Copyright Andreas Franck 2019 - All rights reserved */

#ifndef VISR_LIBEFL_FILTER_FUNCTIONS_HPP_INCLUDED
#define VISR_LIBEFL_FILTER_FUNCTIONS_HPP_INCLUDED

#include "error_codes.hpp"

#include "export_symbols.hpp"

#include "function_wrapper.hpp"

#include <cstddef>
#include <complex>

namespace visr
{
namespace efl
{

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( IirFilterBiquadsSingleChannel, T, ErrorCode, T const *, T *, T*, T const *, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t );
  
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
  return IirFilterBiquadsSingleChannel< T >::call( input, output,
    states, coeffs,
    numElements, numSections,
    stateStride, coeffStride,
    alignment );
}

} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_FILTER_FUNCTIONS_HPP_INCLUDED
