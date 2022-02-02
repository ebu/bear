/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "vector_conversions.hpp"
#include "reference/vector_conversions.hpp"

#include <boost/preprocessor/seq/for_each_product.hpp>

#include <cstdint>

namespace visr
{
namespace efl
{

#define CONVERSION_FUNCTIONS \
(Convert)(ConvertInputStride)(ConvertOutputStride)(ConvertInputOutputStride)

/**
 * Convenience macro to define the function pointers (explicit template
 * instantiations)
 * for a given vector function, specialised for two data types.
 */
#define REGISTER_TWO_TEMPLATE_PARAMETERS_FUNCTION( R, PRODUCT ) \
template<> VISR_EFL_LIBRARY_SYMBOL decltype( BOOST_PP_CAT( Vector, BOOST_PP_CAT(BOOST_PP_SEQ_ELEM( 0, PRODUCT ), Wrapper))< BOOST_PP_SEQ_ELEM( 1, PRODUCT ), BOOST_PP_SEQ_ELEM( 2, PRODUCT ) >::sPtr) BOOST_PP_CAT( Vector, BOOST_PP_CAT( BOOST_PP_SEQ_ELEM( 0, PRODUCT ), Wrapper))< BOOST_PP_SEQ_ELEM( 1, PRODUCT ), BOOST_PP_SEQ_ELEM( 2, PRODUCT ) >::sPtr{ BOOST_PP_CAT( reference::vector, BOOST_PP_SEQ_ELEM( 0, PRODUCT ))< BOOST_PP_SEQ_ELEM( 1, PRODUCT ), BOOST_PP_SEQ_ELEM( 2, PRODUCT ) > };

BOOST_PP_SEQ_FOR_EACH_PRODUCT( REGISTER_TWO_TEMPLATE_PARAMETERS_FUNCTION, (CONVERSION_FUNCTIONS)(VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES)(VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES))

} // namespace efl
} // namespace visr
