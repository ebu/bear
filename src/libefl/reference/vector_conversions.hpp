/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_REFERENCE_VECTOR_CONVERSIONS_HPP_INCLUDED
#define VISR_LIBEFL_REFERENCE_VECTOR_CONVERSIONS_HPP_INCLUDED

#include "../vector_conversions.hpp"

namespace visr
{
namespace efl
{

namespace reference
{

template< typename InputType, typename OutputType >
ErrorCode vectorConvert( InputType const * src,
                         OutputType * dest,
                         std::size_t numElements,
                         std::size_t alignment=0 );

template< typename InputType, typename OutputType >
ErrorCode vectorConvertInputStride( InputType const * src,
                                    OutputType * dest,
                                    std::size_t numElements,
                                    std::size_t inputStride,
                                    std::size_t alignment = 0 );

template< typename InputType, typename OutputType >
ErrorCode vectorConvertOutputStride( InputType const * src,
                                     OutputType * dest,
                                     std::size_t numElements,
                                     std::size_t outputStride,
                                     std::size_t alignment = 0 );

template< typename InputType, typename OutputType >
ErrorCode vectorConvertInputOutputStride( InputType const * src,
                                          OutputType * dest,
                                          std::size_t numElements,
                                          std::size_t inputStride,
                                          std::size_t outputStride,
                                          std::size_t alignment = 0 );

} // namespace efl
} // namespace visr
} // namespace reference

#endif // VISR_LIBEFL_REFERENCE_VECTOR_CONVERSIONS_HPP_INCLUDED
