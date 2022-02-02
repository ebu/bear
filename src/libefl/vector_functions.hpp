/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_VECTOR_FUNCTIONS_HPP_INCLUDED
#define VISR_LIBEFL_VECTOR_FUNCTIONS_HPP_INCLUDED

#include "error_codes.hpp"

#include "export_symbols.hpp"

#include "function_wrapper.hpp"

#include <cstddef>
#include <complex>

namespace visr
{
namespace efl
{

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorZeroWrapper, T, ErrorCode, T * const, std::size_t, std::size_t);

template <typename T>
ErrorCode vectorZero(T * const dest, std::size_t numElements, std::size_t alignment = 0)
{
  return VectorZeroWrapper<T>::call(dest, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorFillWrapper, T, ErrorCode, T, T * const, std::size_t, std::size_t);

template <typename T>
ErrorCode vectorFill( const T value, T * const dest, std::size_t numElements, std::size_t alignment = 0 )
{
  return VectorFillWrapper<T>::call(value, dest, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorCopyWrapper, T, ErrorCode, T const * const, T * const, std::size_t, std::size_t);

template <typename T>
ErrorCode vectorCopy( T const * const source, T * const dest, std::size_t numElements, std::size_t alignment = 0 )
{
  return VectorCopyWrapper<T>::call(source, dest, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorRampWrapper, T, ErrorCode, T * const, std::size_t, T, T, bool, bool, std::size_t );

/**
 * Fill an array with a ramp defined by its start and end value.
 * Instantiated for element types float and double.
 * @tparam T The sample type
 * @param dest The array to be filled
 * @param numElements The number of element of the ramp.
 * @param startVal The start value of the ramp.
 * @param endVal  The end value of the ramp.
 * @param startInclusive Switch whether the first ramp value is startVal (true) or whether the ramp is constructed
 * that the value before the first would be startVal (false).
 * @param endInclusive Switch whether the last ramp value is endVal (true) or whether the ramp is constructed
 * that the value after the last sample would be endVal (false).
 * @param alignment The aligment of the dest vector, given in numbers of elements.
 */
template <typename T>
ErrorCode vectorRamp( T * const dest, std::size_t numElements, T startVal, T endVal,
                      bool startInclusive, bool endInclusive, std::size_t alignment = 0 )
{
  return VectorRampWrapper<T>::call( dest, numElements, startVal, endVal,
                                     startInclusive, endInclusive, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorAddWrapper, T, ErrorCode, T const * const, T const * const, T * const, std::size_t, std::size_t );

template<typename T>
ErrorCode vectorAdd( T const * const op1,
         T const * const op2,
         T * const result,
         std::size_t numElements,
         std::size_t alignment = 0 )
{
  return VectorAddWrapper<T>::call(op1, op2, result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorAddInplaceWrapper, T, ErrorCode, T const * const, T * const, std::size_t, std::size_t );

template<typename T>
ErrorCode vectorAddInplace( T const * const op1,
                            T * const op2Result,
                            std::size_t numElements,
                            std::size_t alignment = 0)
{
  return VectorAddInplaceWrapper<T>::call(op1, op2Result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorAddConstantWrapper, T, ErrorCode, T, T const * const, T * const, std::size_t, std::size_t );

  template<typename T>
ErrorCode vectorAddConstant( T constantValue,
                             T const * const op,
                             T * const result,
                             std::size_t numElements,
                             std::size_t alignment = 0)
{
  return VectorAddConstantWrapper<T>::call(constantValue, op, result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorAddConstantInplaceWrapper, T, ErrorCode, T, T * const, std::size_t, std::size_t );

template<typename T>
ErrorCode vectorAddConstantInplace(T constantValue,
	T * const opResult,
	std::size_t numElements,
	std::size_t alignment = 0)
{
  return VectorAddConstantInplaceWrapper<T>::call(constantValue, opResult, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorSubtractWrapper, T, ErrorCode, T const * const, T const * const, T * const, std::size_t, std::size_t);

template<typename T>
ErrorCode vectorSubtract( T const * const subtrahend,
                          T const * const minuend,
                          T * const result,
                          std::size_t numElements,
                          std::size_t alignment = 0 )
{
  return VectorSubtractWrapper<T>::call( subtrahend, minuend, result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorSubtractInplaceWrapper, T, ErrorCode, T const * const, T * const, std::size_t, std::size_t);

template<typename T>
ErrorCode vectorSubtractInplace( T const * const minuend,
  T * const subtrahendResult,
  std::size_t numElements,
  std::size_t alignment = 0 )
{
  return VectorSubtractInplaceWrapper<T>::call(minuend, subtrahendResult, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorSubtractConstantWrapper, T, ErrorCode, T, T const * const, T * const, std::size_t, std::size_t);

  template<typename T>
ErrorCode vectorSubtractConstant( T constantMinuend,
  T const * const subtrahend,
  T * const result,
  std::size_t numElements,
  std::size_t alignment = 0 )
{
  return VectorSubtractConstantWrapper<T>::call(constantMinuend, subtrahend, result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorSubtractConstantInplaceWrapper, T, ErrorCode, T, T * const, std::size_t, std::size_t);

template<typename T>
ErrorCode vectorSubtractConstantInplace( T constantMinuend,
  T * const subtrahendResult,
  std::size_t numElements,
  std::size_t alignment = 0 )
{
  return VectorSubtractConstantInplaceWrapper<T>::call(constantMinuend, subtrahendResult, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorMultiplyWrapper, T, ErrorCode, T const * const, T const * const, T * const, std::size_t, std::size_t );

/**
 * Multiply two vectors.
 * @tparam T The element type of the operands.
 * @param factor1 First vector to multiply.
 * @param factor2 Second vector to multiply.
 * @param[out] result The result of the operation.
 * @param numElements The number of elements to be multiplied.
 * @param alignment Assured alignment of all vector arguments (measured in elements).
 */
template<typename T>
ErrorCode vectorMultiply( T const * const factor1,
                          T const * const factor2,
                          T * const result,
                          std::size_t numElements,
                          std::size_t alignment = 0 )
{
  return VectorMultiplyWrapper<T>::call(factor1, factor2, result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorMultiplyInplaceWrapper, T, ErrorCode, T const * const, T * const, std::size_t, std::size_t);

/**
 * Multiply two vectors inplace.
 * @tparam T The element type of the operands.
 * @param factor1 First vector to multiply.
 * @param[in,out] factor2Result Second vector to multiply, holds also the result of the operation.
 * @param numElements The number of elements to be multiplied.
 * @param alignment Assured alignment of all vector arguments (measured in elements).
 */
template<typename T>
ErrorCode vectorMultiplyInplace( T const * const factor1,
                                 T * const factor2Result,
                                 std::size_t numElements,
                                 std::size_t alignment = 0)
{
  return VectorMultiplyInplaceWrapper<T>::call( factor1, factor2Result, numElements, alignment );
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorMultiplyConstantWrapper, T, ErrorCode, T, T const * const, T * const, std::size_t, std::size_t);

/**
 * Multiply a vector with a constant.
 * @tparam T The element type of the operands.
 * @param constantValue The constant scaling factor.
 * @param[in] factor Vector to multiply with.
 * @param[out] result The result of the operation.
 * @param numElements The number of elements to be multiplied.
 * @param alignment Assured alignment of all vector arguments (measured in elements).
 */
template<typename T>
ErrorCode vectorMultiplyConstant( T constantValue,
                                  T const * const factor,
                                  T * const result,
                                  std::size_t numElements,
                                  std::size_t alignment = 0 )
{
  return VectorMultiplyConstantWrapper<T>::call( constantValue, factor, result, numElements, alignment );
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorMultiplyConstantInplaceWrapper, T, ErrorCode, T, T * const, std::size_t, std::size_t);

/**
 * Multiply a vector with a constant inplace.
 * @tparam T The element type of the operands.
 * @param constantValue The constant scaling factor.
 * @param[in,out] factorResult Vector to multiply with, also holds the result.
 * @param numElements The number of elements to be multiplied.
 * @param alignment Assured alignment of all vector arguments (measured in elements).
 */
template<typename T>
ErrorCode vectorMultiplyConstantInplace( T constantValue,
                                         T * const factorResult,
                                         std::size_t numElements,
                                         std::size_t alignment = 0 )
{
  return VectorMultiplyConstantInplaceWrapper<T>::call(constantValue, factorResult, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorMultiplyAddWrapper, T, ErrorCode, T const * const, T const * const, T const * const, T * const, std::size_t, std::size_t);

template<typename T>
ErrorCode vectorMultiplyAdd( T const * const factor1,
  T const * const factor2,
  T const * const addend,
  T * const result,
  std::size_t numElements,
  std::size_t alignment = 0 )
{
  return VectorMultiplyAddWrapper<T>::call( factor1, factor2, addend, result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorMultiplyAddInplaceWrapper, T, ErrorCode, T const * const, T const * const, T * const, std::size_t, std::size_t);

template<typename T>
ErrorCode vectorMultiplyAddInplace( T const * const factor1,
  T const * const factor2,
  T * const accumulator,
  std::size_t numElements,
  std::size_t alignment = 0 )
{
  return VectorMultiplyAddInplaceWrapper<T>::call( factor1, factor2, accumulator, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorMultiplyConstantAddWrapper, T, ErrorCode, T, T const * const, T const * const, T * const, std::size_t, std::size_t);

template<typename T>
ErrorCode vectorMultiplyConstantAdd( T constFactor,
                                     T const * const factor,
                                     T const * const addend,
                                     T * const result,
                                     std::size_t numElements,
                                     std::size_t alignment = 0 )
{
  return VectorMultiplyConstantAddWrapper<T>::call(constFactor, factor, addend, result, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorMultiplyConstantAddInplaceWrapper, T, ErrorCode, T, T const * const, T * const, std::size_t, std::size_t);

template<typename T>
ErrorCode vectorMultiplyConstantAddInplace( T constFactor,
  T const * const factor,
  T * const accumulator,
  std::size_t numElements,
  std::size_t alignment = 0 )
{
  return VectorMultiplyConstantAddInplaceWrapper<T>::call(constFactor, factor, accumulator, numElements, alignment);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorCopyStridedWrapper, T, ErrorCode, T const * const, T * const, std::size_t,
                                           std::size_t, std::size_t, std::size_t );

/**
 * Copy a strided sequence of values, i.e., a memory area where the elements have a constant, but not necessarily
 * unit distance, to another strided sequence
 * @tparam DataType The data type contained in the sequences. Must provide an assignment operator.
 * @param src The base pointer of the source sequence (i.e., location of the first element)
 * @param dest Base pointer of the destination sequence (i.e., location of the first element).
 * @param srcStrideElements Stride (i.e., distance between the elements) of the source sequence given in number of elements (not bytes).
 * @param destStrideElements Stride (i.e., distance between the elements) of the destination sequence given in number of elements (not bytes).
 * @param numberOfElements The number of elements to be copied.
 * @param alignmentElements Minimum alignment of the source and destination sequences (in number of elements)
 * @note Consider removing the \p alignmentElements parameter because it is scarcely useful unliess both sequences have stride 1.
 */
template<typename DataType>
ErrorCode vectorCopyStrided( DataType const * const src, DataType * const dest, std::size_t srcStrideElements,
  std::size_t destStrideElements, std::size_t numberOfElements, std::size_t alignmentElements )
{
  return VectorCopyStridedWrapper<DataType>::call(src, dest, srcStrideElements, destStrideElements, numberOfElements, alignmentElements);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( VectorFillStridedWrapper, T, ErrorCode, T, T * const, std::size_t, std::size_t, std::size_t );

/**
 * Fill a strided sequence of values, i.e., a memory area where the elements have a constant, but not necessarily
 * unit distance, with a constant value
 * @tparam DataType The data type contained in the sequences. Must provide an assignemnt operator.
 * @param val The constant value to be assigned to the strided locations.
 * @param dest The base pointer of the strided sequence.
 * @param destStrideElements Distance between consecutive addressed elements in the target sequence.
 * @param numberOfElements Number of values to be assigned.
 * @param alignmentElements The alignment of the target sequence, i.e., of the base pointer.
 * @note Consider removing the alignmentElements parameter, because it it is scarcely useful for non-unit strides.
*/
template<typename DataType>
efl::ErrorCode vectorFillStrided( DataType val,
                                  DataType * const dest,
                                  std::size_t destStrideElements,
                                  std::size_t numberOfElements,
                                  std::size_t alignmentElements )
{
  return VectorFillStridedWrapper<DataType>::call(val, dest, destStrideElements, numberOfElements, alignmentElements);
}

VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorRampScalingWrapper, T, ErrorCode, T const *, T const * , T *, T, T, std::size_t, bool, std::size_t);

/**
 * Apply a ramp-shaped gain scaling to an input signal.
 * The gain applied to the sample $input[i]$ is $baseGain+rampGain*ramp[i]$
 * @param input The signal to be scaled.
 * @param ramp The shape of the scaling, must match the signal length
 * \p numberOfElements.
 * @param [in,out] output The output signal. When the parameter 
 * \p accumulate is true, the result is added to \p output, and overwrites
 * the present value of \p output otherwise
 * @param baseGain The scalar base value of the gain, which is added to the
 * scaled ramp gain.
 * @param rampGain Scalar scaling factor to be applied to the gain ramp.
 * @param numberOfElements The number of elements in the \p input, \p ramp, and
 * \p output vectors.
 * @param accumulate Flag specifying whether the result overwrites the \p output
 * vector (if false), or whether it is added to \p output (if true).
 * @param alignmentElements Minimum alignment of the vector arguments \p input,
 * \p ramp, and \p output, measured in multiples of the element size. 
 */
template<typename T>
efl::ErrorCode vectorRampScaling( T const * input,
  T const * ramp,
  T * output,
  T baseGain,
  T rampGain,
  std::size_t numberOfElements,
  bool accumulate /*= false*/,
  std::size_t alignmentElements /*= 0*/)
{
  return VectorRampScalingWrapper<T>::call( input, ramp, output,
   baseGain, rampGain, numberOfElements, accumulate, alignmentElements );
}

// ===============================================================================================================
// Implementation details: Define the function templates for each API function.

} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_VECTOR_FUNCTIONS_HPP_INCLUDED
