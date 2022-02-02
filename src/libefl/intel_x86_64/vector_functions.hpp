/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_INTEL_X86_64_VECTOR_FUNCTIONS_HPP_INCLUDED
#define VISR_LIBEFL_INTEL_X86_64_VECTOR_FUNCTIONS_HPP_INCLUDED

#include "../vector_functions.hpp"

#include <complex>

namespace visr
{
namespace efl
{
namespace intel_x86_64
{

enum class Feature
{
  FMA,
  AVX,
  SSE
};

  
/**
 * Multiply two vectors.
 * @tparam T The element type of the operands.
 * @param factor1 First vector to multiply.
 * @param factor2 Second vector to multiply.
 * @param[out] result The result of the operation.
 * @param numElements The number of elements to be multiplied.
 * @param alignment Assured alignment of all vector arguments (measured in elements).
 */
  template<typename T, Feature f>
VISR_EFL_LIBRARY_SYMBOL 
ErrorCode vectorMultiply( T const * const factor1,
                          T const * const factor2,
                          T * const result,
                          std::size_t numElements,
                          std::size_t alignment = 0 );

/**
 * Multiply two vectors inplace.
 * @tparam T The element type of the operands.
 * @param factor1 First vector to multiply.
 * @param[in,out] factor2Result Second vector to multiply, holds also the result of the operation.
 * @param numElements The number of elements to be multiplied.
 * @param alignment Assured alignment of all vector arguments (measured in elements).
 */
template<typename T>
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorMultiplyInplace( T const * const factor1,
		       T * const factor2Result,
		       std::size_t numElements,
		       std::size_t alignment = 0);

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
VISR_EFL_LIBRARY_SYMBOL 
ErrorCode vectorMultiplyConstant( T constantValue,
                                  T const * const factor,
                                  T * const result,
                                  std::size_t numElements,
                                  std::size_t alignment = 0 );

/**
 * Multiply a vector with a constant inplace.
 * @tparam T The element type of the operands.
 * @param constantValue The constant scaling factor.
 * @param[in,out] factorResult Vector to multiply with, also holds the result.
 * @param numElements The number of elements to be multiplied.
 * @param alignment Assured alignment of all vector arguments (measured in elements).
 */
template<typename T>
VISR_EFL_LIBRARY_SYMBOL
ErrorCode vectorMultiplyConstantInplace( T constantValue,
                                         T * const factorResult,
                                         std::size_t numElements,
                                         std::size_t alignment = 0 );  

template< typename T >
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorMultiplyAdd( T const * const factor1,
		   T const * const factor2,
		   T const * const addend,
		   T * const result,
		   std::size_t numElements,
		   std::size_t alignment /*= 0*/ );

template<typename T>
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorMultiplyConstantAdd( T constFactor,
			   T const * const factor,
			   T const * const addend,
			   T * const result,
			   std::size_t numElements,
			   std::size_t alignment = 0 );

template<typename T, Feature f >
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorMultiplyAddInplace( T const * const factor1,
			  T const * const factor2,
			  T * const accumulator,
			  std::size_t numElements,
			  std::size_t alignment /*= 0*/ );

template<typename T, Feature f>
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorMultiplyConstantAddInplace( T constFactor,
				  T const * const factor,
				  T * const accumulator,
				  std::size_t numElements,
				  std::size_t alignment /*= 0*/ );

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
template<typename T, Feature f>
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorRampScaling( T const * input,
  T const * ramp,
  T * output,
  T baseGain,
  T rampGain,
  std::size_t numberOfElements,
  bool accumulate /*= false*/,
  std::size_t alignmentElements /*= 0*/ );

} // namespace intel_x86_64
} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_INTEL_X86_64_VECTOR_FUNCTIONS_HPP_INCLUDED
