/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_VECTOR_CONVERSIONS_HPP_INCLUDED
#define VISR_LIBEFL_VECTOR_CONVERSIONS_HPP_INCLUDED

#include "error_codes.hpp"
#include "function_wrapper.hpp"

#include <cstddef>
#include <cstdint>

namespace visr
{
namespace efl
{

/**
 */
//@{

/**
 * List of supported floating point data types.
 * Conversion functions are provided for each combination of floating-point datatypes (including when the * input type equals the output type), and each combination of floating-point and integer data types.
 * @note The type list is in a format usable by boost::preprocessor, which is used to automatically
 * generated the function dispatchers and reference implementations.
 */
#define VISR_EFL_CONVERSION_FUNCTION_FLOAT_TYPES (float)(double)

/**
 * List of supported integer data types.
 * Conversion functions are provided for each combination of integer data types (including when the
 * source type equals the integer type), and each combination of floating-point and integer data types.
 * @note The type list is in a format usable by boost::preprocessor, which is used to automatically
 * generated the function dispatchers and reference implementations.
 */
#define VISR_EFL_CONVERSION_FUNCTION_INTEGER_TYPES (int8_t)(int16_t)(int32_t)(int64_t)

#define VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES (float)(double)(int8_t)(int16_t)(int32_t)(int64_t)

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE_TWO_TYPES( VectorConvertWrapper, T1, T2, ErrorCode, T1 const *, T2 *, std::size_t, std::size_t);

/**
 * Convert a contiguous vector of elements into a contiguous vector.
 * @tparam InputType source data type
 * @tparam OutputType destination data type.
 * @param src The source vector.
 * @param dest The destination vector, must hold at least \p numElements
 * @param numElements The number of elements to be converted.
 * @param alignment The minimum alignment of the input and output vector,
 * measured in number of elements.
 */
template< typename InputType, typename OutputType >
ErrorCode vectorConvert( InputType const * src,
                         OutputType * dest,
                         std::size_t numElements,
                         std::size_t alignment = 0 )
{
  return VectorConvertWrapper< InputType, OutputType >::call( src, dest, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE_TWO_TYPES( VectorConvertInputStrideWrapper, T1, T2, ErrorCode, T1 const *, T2 *, std::size_t, std::size_t, std::size_t);

/**
 * Convert a strided sequence of elements into a contiguous vector.
 * @tparam InputType source data type
 * @tparam OutputType destination data type.
 * @param src The source vector.
 * @param dest The destination vector, must hold at least \p numElements
 * @param numElements The number of elements to be converted.
 * @param inputStride Distance between the input elements (in number of elements)
 * @param alignment The minimum alignment of the input and output vector,
 * measured in number of elements.
 */
template< typename InputType, typename OutputType >
ErrorCode vectorConvertInputStride( InputType const * src,
                                    OutputType * dest,
                                    std::size_t numElements,
                                    std::size_t inputStride,
                                    std::size_t alignment = 0 )
{
  return VectorConvertInputStrideWrapper< InputType, OutputType >::call(
     src, dest, numElements, inputStride, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE_TWO_TYPES( VectorConvertOutputStrideWrapper, T1, T2, ErrorCode, T1 const *, T2 *, std::size_t, std::size_t, std::size_t);

/**
 * Convert contiguous vector into a strided sequence of elements.
 * @tparam InputType source data type
 * @tparam OutputType destination data type.
 * @param src The source vector.
 * @param dest The destination vector, must hold at least \p numElements
 * @param numElements The number of elements to be converted.
 * @param outputStride Distance between the output elements (in number of elements)
 * @param alignment The minimum alignment of the input and output vector,
 * measured in number of elements.
 */
template< typename InputType, typename OutputType >
ErrorCode vectorConvertOutputStride( InputType const * src,
                                     OutputType * dest,
                                     std::size_t numElements,
                                     std::size_t outputStride,
                                     std::size_t alignment = 0 )
{
  return VectorConvertOutputStrideWrapper< InputType, OutputType >::call(
     src, dest, numElements, outputStride, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE_TWO_TYPES( VectorConvertInputOutputStrideWrapper, T1, T2, ErrorCode, T1 const *, T2 *, std::size_t, std::size_t, std::size_t, std::size_t );

/**
 * Convert strided sequence into a strided sequence of elements.
 * @tparam InputType source data type
 * @tparam OutputType destination data type.
 * @param src The source vector.
 * @param dest The destination vector, must hold at least \p numElements
 * @param numElements The number of elements to be converted.
 * @param inputStride Distance between the input elements (in number of elements)
 * @param outputStride Distance between the output elements (in number of elements)
 * @param alignment The minimum alignment of the input and output vector,
 * measured in number of elements.
 */
template< typename InputType, typename OutputType >
ErrorCode vectorConvertInputOutputStride( InputType const * src,
                                          OutputType * dest,
                                          std::size_t numElements,
                                          std::size_t inputStride,
                                          std::size_t outputStride,
                                          std::size_t alignment = 0 )
{
  return VectorConvertInputOutputStrideWrapper< InputType, OutputType >::call(
     src, dest, numElements, inputStride, outputStride, alignment);
}
//@}

} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_VECTOR_CONVERSIONS_HPP_INCLUDED
