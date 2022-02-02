/* Copyright 2019 AudioScenic Ltd - All rights reserved */

#include "filter_functions_impl.hpp"

namespace visr
{
namespace efl
{
namespace reference
{

// Explicit template instantiations
template
ErrorCode iirFilterBiquadsSingleChannel<float>( float const *, float *, float *,
                                         float const *, std::size_t,
                                         std::size_t, std::size_t, std::size_t,
                                         std::size_t );

template                                         
ErrorCode iirFilterBiquadsSingleChannel<double>( double const *, double *, double *,
                                         double const *, std::size_t,
                                         std::size_t, std::size_t, std::size_t,
                                         std::size_t );

} // namespace reference
} // namespace efl
} // namespace visr
