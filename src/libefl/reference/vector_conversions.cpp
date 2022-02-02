/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "../vector_conversions.hpp"
#include "vector_conversions_impl.hpp"

#include <boost/preprocessor/seq/for_each_product.hpp>

namespace visr
{
namespace efl
{

namespace reference
{

// Prevent Doxygen from trying to parse these explicit instantiations.
/// @cond NEVER
#define EXPLICITLY_INSTANTIATE_VECTOR_CONVERT( R, PARAMS ) \
template ErrorCode vectorConvert<BOOST_PP_SEQ_ELEM( 0, PARAMS ),BOOST_PP_SEQ_ELEM( 1, PARAMS )>( BOOST_PP_SEQ_ELEM( 0, PARAMS ) const *,\
BOOST_PP_SEQ_ELEM( 1, PARAMS ) *, std::size_t, std::size_t );
BOOST_PP_SEQ_FOR_EACH_PRODUCT( EXPLICITLY_INSTANTIATE_VECTOR_CONVERT, (VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES)(VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES))

#define EXPLICITLY_INSTANTIATE_VECTOR_CONVERT_INPUT_STRIDE( R, PARAMS ) \
template ErrorCode vectorConvertInputStride\
<BOOST_PP_SEQ_ELEM( 0, PARAMS ),BOOST_PP_SEQ_ELEM( 1, PARAMS )>\
( BOOST_PP_SEQ_ELEM( 0, PARAMS ) const *,\
BOOST_PP_SEQ_ELEM( 1, PARAMS ) *, std::size_t, std::size_t, std::size_t );
BOOST_PP_SEQ_FOR_EACH_PRODUCT( EXPLICITLY_INSTANTIATE_VECTOR_CONVERT_INPUT_STRIDE, (VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES)(VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES))

#define EXPLICITLY_INSTANTIATE_VECTOR_CONVERT_OUTPUT_STRIDE( R, PARAMS ) \
template ErrorCode vectorConvertOutputStride\
<BOOST_PP_SEQ_ELEM( 0, PARAMS ),BOOST_PP_SEQ_ELEM( 1, PARAMS )>\
( BOOST_PP_SEQ_ELEM( 0, PARAMS ) const *,\
BOOST_PP_SEQ_ELEM( 1, PARAMS ) *, std::size_t, std::size_t, std::size_t );
BOOST_PP_SEQ_FOR_EACH_PRODUCT( EXPLICITLY_INSTANTIATE_VECTOR_CONVERT_OUTPUT_STRIDE, (VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES)(VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES))

#define EXPLICITLY_INSTANTIATE_VECTOR_CONVERT_INPUT_OUTPUT_STRIDE( R, PARAMS ) \
template ErrorCode vectorConvertInputOutputStride\
<BOOST_PP_SEQ_ELEM( 0, PARAMS ),BOOST_PP_SEQ_ELEM( 1, PARAMS )>\
( BOOST_PP_SEQ_ELEM( 0, PARAMS ) const *,\
BOOST_PP_SEQ_ELEM( 1, PARAMS ) *, std::size_t, std::size_t, std::size_t, std::size_t );
BOOST_PP_SEQ_FOR_EACH_PRODUCT( EXPLICITLY_INSTANTIATE_VECTOR_CONVERT_INPUT_OUTPUT_STRIDE, (VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES)(VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES))
/// @endcond NEVER

} // namespace reference
} // namespace efl
} // namespace visr
