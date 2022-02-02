/* Copyright 2019 AudioScenic Ltd - All rights reserved */

#include "filter_functions.hpp"

#include "function_wrapper.hpp"

#include "reference/filter_functions.hpp"

namespace visr
{
namespace efl
{

/**
 * Convenience macro to define the function pointers (explicit template instantiations)
 * for a given vector function and a data type.
 */
#define EFL_FUNCTION_WRAPPER_INSTANTIATION( Wrapper, DataType, referenceFunction )\
template<> VISR_EFL_LIBRARY_SYMBOL decltype(Wrapper< DataType >::sPtr) Wrapper< DataType >::sPtr{ &referenceFunction< DataType > };

EFL_FUNCTION_WRAPPER_INSTANTIATION( IirFilterBiquadsSingleChannel, float, \
				    reference::iirFilterBiquadsSingleChannel )
EFL_FUNCTION_WRAPPER_INSTANTIATION( IirFilterBiquadsSingleChannel, double, \
				    reference::iirFilterBiquadsSingleChannel )

} // namespace efl
} // namespace visr
