/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "vector_functions.hpp"

#include "function_wrapper.hpp"

#include "reference/vector_functions.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each_product.hpp>

namespace visr
{
namespace efl
{

#define ALL_NUMERIC_VECTOR_FUNCTIONS \
  (Zero) (Fill) (Copy) (Ramp) \
  (Add) (AddInplace) (AddConstant) (AddConstantInplace) \
  (Subtract) (SubtractInplace) (SubtractConstant) (SubtractConstantInplace) \
  (Multiply) (MultiplyInplace) (MultiplyConstant) (MultiplyConstantInplace) \
  (MultiplyAdd) (MultiplyAddInplace) (MultiplyConstantAdd) (MultiplyConstantAddInplace) \
  (CopyStrided) (FillStrided) (RampScaling)

/**
 * The numeric data types
 */
#define NUMERIC_DATATYPES \
  (float) (double)(std::complex<float>)(std::complex<double>)

/**
 * Convenience macro to define the function pointers (explicit template instantiations)
 * for a given vector function and a data type.
 * @note Not used anymore because explicit instantiation is performed directly in EXPLICIT_WRAPPER_INSTANTIATION
 */
#define EFL_FUNCTION_WRAPPER_INSTANTIATION( Wrapper, DataType, referenceFunction )\
template<> VISR_EFL_LIBRARY_SYMBOL decltype(Wrapper< DataType >::sPtr) Wrapper< DataType >::sPtr{ &referenceFunction< DataType > };

/**
 * Macro to perform an explicit instantation of a function wrapper class.
 * TO be used in a Boost preprocessor 'loop' construct to initialise all wrappers.
 */
#define EXPLICIT_WRAPPER_INSTANTIATION( R, PRODUCT ) \
  template<> VISR_EFL_LIBRARY_SYMBOL decltype( BOOST_PP_CAT(Vector,  BOOST_PP_CAT(BOOST_PP_SEQ_ELEM( 0, PRODUCT ),Wrapper))< BOOST_PP_SEQ_ELEM( 1, PRODUCT ) >::sPtr) \
  BOOST_PP_CAT(Vector,  BOOST_PP_CAT(BOOST_PP_SEQ_ELEM( 0, PRODUCT ),Wrapper))< BOOST_PP_SEQ_ELEM( 1, PRODUCT ) >::sPtr{ \
  BOOST_PP_CAT( reference::vector, BOOST_PP_SEQ_ELEM( 0, PRODUCT ))<  BOOST_PP_SEQ_ELEM( 1, PRODUCT ) > };

BOOST_PP_SEQ_FOR_EACH_PRODUCT( EXPLICIT_WRAPPER_INSTANTIATION, (ALL_NUMERIC_VECTOR_FUNCTIONS)(NUMERIC_DATATYPES))

/**
 * LIst additional data type for which vectorCopy() is defined.
 */
#define ADDITIONAL_COPY_DATATYPES \
  (int8_t)(uint8_t)(int16_t)(uint16_t)(int32_t)(uint32_t)(int64_t)(uint64_t)(long double)

BOOST_PP_SEQ_FOR_EACH_PRODUCT( EXPLICIT_WRAPPER_INSTANTIATION, ((Copy))(ADDITIONAL_COPY_DATATYPES))

} // namespace efl
} // namespace visr
