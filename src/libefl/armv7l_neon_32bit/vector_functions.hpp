/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_ARMV7L_NEON_32BIT_VECTOR_FUNCTIONS_HPP_INCLUDED
#define VISR_LIBEFL_ARMV7L_NEON_32BIT_VECTOR_FUNCTIONS_HPP_INCLUDED

#include "../vector_functions.hpp"

#include <complex>

namespace visr
{
namespace efl
{
namespace armv7l_neon_32bit
{

template<typename T>
VISR_EFL_LIBRARY_SYMBOL
ErrorCode vectorAdd( T const * const op1,
		     T const * const op2,
		     T * const result,
		     std::size_t numElements,
		     std::size_t alignment = 0 );

template<typename T>
VISR_EFL_LIBRARY_SYMBOL
ErrorCode vectorAddInplace( T const * const op1,
                            T * const op2Result,
                            std::size_t numElements,
                            std::size_t alignment = 0);

template<typename T>
VISR_EFL_LIBRARY_SYMBOL
ErrorCode vectorAddConstant( T constantValue,
                             T const * const op,
                             T * const result,
                             std::size_t numElements,
                             std::size_t alignment = 0);

template<typename T>
VISR_EFL_LIBRARY_SYMBOL
ErrorCode vectorAddConstantInplace(T constantValue,
				   T * const opResult,
				   std::size_t numElements,
				   std::size_t alignment = 0);

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

template<typename T>
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorMultiplyAddInplace( T const * const factor1,
			  T const * const factor2,
			  T * const accumulator,
			  std::size_t numElements,
			  std::size_t alignment /*= 0*/ );

template<typename T>
VISR_EFL_LIBRARY_SYMBOL ErrorCode
vectorMultiplyConstantAddInplace( T constFactor,
				  T const * const factor,
				  T * const accumulator,
				  std::size_t numElements,
				  std::size_t alignment /*= 0*/ );

} // namespace armv7l_neon_32bit
} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_ARMV7L_NEON_32BIT_VECTOR_FUNCTIONS_HPP_INCLUDED
