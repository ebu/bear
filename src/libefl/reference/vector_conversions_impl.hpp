/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_REFERENCE_VECTOR_CONVERSIONS_IMPL_HPP_INCLUDED
#define VISR_LIBEFL_REFERENCE_VECTOR_CONVERSIONS_IMPL_HPP_INCLUDED

/**
 * @file reference/vector_conversions.hpp
 * Provide templated implementations of the reference vector 
 * conversion functions.
 * This file is is not normally included in user code, but mainly in 
 * .cpp files that provide explicit template instantiations.
 * This in done in reference/vector_conversions.cpp for the data types supported 
 *  by the standard efl library.
 */

#include "vector_conversions.hpp"

#include "../alignment.hpp"

#include <ciso646>
#include <cmath>
#include <type_traits>

namespace visr
{
namespace efl
{

namespace reference
{

namespace // unnamed
{

/**
 * Generic function object template for converting scalars.
 * The general variant uses a cast.
 */
template< typename InputType, typename OutputType, typename Rounding = void >
struct Converter
{
  /**
   * Conversion function
   */
  static OutputType convert( InputType val )
  {
    return static_cast<OutputType>( val );
  }
};

/**
 * Partial template specialisation of the type conversion functor for float->int conversions.
 * This variant uses llrint(), which means the output data type can be as large as a long long.
 */
template< typename InputType, typename OutputType>
struct Converter< InputType, OutputType, typename std::enable_if<std::is_floating_point<InputType>::value and std::is_integral<OutputType>::value >::type >
{
  static OutputType convert( InputType val )
  {
    // TODO: use lrint based on actual output type.
    return static_cast<OutputType>( std::llrint(val) );
  }
};

} // unnamed namespace;

template< typename InputType, typename OutputType >
ErrorCode vectorConvert( InputType const * src,
                         OutputType * dest,
                         std::size_t numElements,
                         std::size_t alignment /*= 0*/ )
{
  return vectorConvertInputOutputStride( src, dest, numElements, 1, 1, alignment );
}

template< typename InputType, typename OutputType >
ErrorCode vectorConvertInputStride( InputType const * src,
                                    OutputType * dest,
                                    std::size_t numElements,
                                    std::size_t inputStride,
                                    std::size_t alignment /*= 0*/ )
{
  return vectorConvertInputOutputStride( src, dest, numElements, inputStride, 1, alignment );
}

template< typename InputType, typename OutputType >
ErrorCode vectorConvertOutputStride( InputType const * src,
                                     OutputType * dest,
                                     std::size_t numElements,
                                     std::size_t outputStride,
                                     std::size_t alignment /*= 0*/ )
{
  return vectorConvertInputOutputStride( src, dest, numElements, 1, outputStride, alignment );
}

template< typename InputType, typename OutputType >
ErrorCode vectorConvertInputOutputStride( InputType const * src,
                                          OutputType * dest,
                                          std::size_t numElements,
                                          std::size_t inputStride,
                                          std::size_t outputStride,
                                          std::size_t alignment /*= 0*/ )
{
  if( not checkAlignment( src, alignment ) ) return alignmentError;
  if( not checkAlignment( dest, alignment ) ) return alignmentError;
  InputType const * inputPtr( src );
  OutputType * outputPtr( dest );
  for( std::size_t runIdx(0); runIdx < numElements; ++runIdx, inputPtr += inputStride, outputPtr += outputStride )
  {
    *outputPtr = Converter< InputType, OutputType >::convert( *inputPtr );
  }
  return noError;
}

} // namespace reference
} // namespace efl
} // namespace visr

#endif // VISR_LIBEFL_REFERENCE_VECTOR_CONVERSIONS_IMPL_HPP_INCLUDED

