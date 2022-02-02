/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "check_error.hpp"

#include <python/libpythonbindinghelpers/container_access.hpp>

#include <libefl/vector_conversions.hpp>

#include <libefl/basic_vector.hpp>

#include <libvisr/detail/compose_message_string.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <boost/preprocessor/seq/for_each_product.hpp>

namespace py = pybind11;

namespace visr
{
namespace efl
{
namespace python
{

namespace // unnamed
{

// template< typename InputType, typename OutputType >
// ErrorCode vectorConvert( InputType const * const src,
//                          OutputType * const dest,
//                          std::size_t numElements,
//                          std::size_t alignment = 0 )
template< template<typename> class Container, typename InputType, typename OutputType >
void exportVectorConvert( py::module & m )
{
  m.def( "vectorConvert",
         []( Container<InputType> const & src, Container<OutputType> & dest, std::size_t numElements, std::size_t alignment )
   {
     InputType const * const srcPtr = visr::python::bindinghelpers::ContainerAccess<Container, InputType >::constantPointer( src, numElements, "src" );
     OutputType * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, OutputType >::mutablePointer( dest, numElements, "dest" );
     detail::checkError( efl::vectorConvert<InputType, OutputType>( srcPtr, destPtr, numElements, alignment ) );
   }, py::arg( "src" ), py::arg("dest"), py::arg("numElements"), py::arg("alignment")=0 );
}

// template< typename InputType, typename OutputType >
// VISR_EFL_LIBRARY_SYMBOL
// ErrorCode vectorConvertInputStride( InputType const * const src,
//                                     OutputType * const dest,
//                                     std::size_t numElements,
//                                     std::size_t inputStride,
//                                     std::size_t alignment = 0 )
template< template<typename> class Container, typename InputType, typename OutputType >
void exportVectorConvertInputStride( py::module & m )
{
  m.def( "vectorConvertInputStride",
         []( Container<InputType> const & src, Container<OutputType> & dest, std::size_t numElements,
         std::size_t inputStride,  std::size_t alignment )
   {
     InputType const * const srcPtr = visr::python::bindinghelpers::ContainerAccess<Container, InputType >::constantPointer( src, numElements, "src" );
     OutputType * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, OutputType >::mutablePointer( dest, numElements, "dest" );
     detail::checkError( efl::vectorConvertInputStride<InputType,
       OutputType>( srcPtr, destPtr, numElements, inputStride,
       alignment ) );
   }, py::arg( "src" ), py::arg("dest"), py::arg("numElements"),
   py::arg("inputStride"), py::arg("alignment")=0 );
}

// template< typename InputType, typename OutputType >
// VISR_EFL_LIBRARY_SYMBOL
// ErrorCode vectorConvertOutputStride( InputType const * const src,
//                                      OutputType * const dest,
//                                      std::size_t numElements,
//                                      std::size_t outputStride,
//                                      std::size_t alignment = 0 )
template< template<typename> class Container, typename InputType, typename OutputType >
void exportVectorConvertOutputStride( py::module & m )
{
  m.def( "vectorConvertOutputStride",
         []( Container<InputType> const & src, Container<OutputType> & dest, std::size_t numElements,
         std::size_t outputStride,  std::size_t alignment )
   {
     InputType const * const srcPtr = visr::python::bindinghelpers::ContainerAccess<Container, InputType >::constantPointer( src, numElements, "src" );
     OutputType * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, OutputType >::mutablePointer( dest, numElements, "dest" );
     detail::checkError( efl::vectorConvertOutputStride<InputType,
       OutputType>( srcPtr, destPtr, numElements, outputStride,
       alignment ) );
   }, py::arg( "src" ), py::arg("dest"), py::arg("numElements"),
   py::arg("outputStride"), py::arg("alignment")=0 );
}

// template< typename InputType, typename OutputType >
// VISR_EFL_LIBRARY_SYMBOL
// ErrorCode vectorConvertInputOutputStride( InputType const * const src,
//                                           OutputType * const dest,
//                                           std::size_t numElements,
//                                           std::size_t inputStride,
//                                           std::size_t outputStride,
//                                           std::size_t alignment = 0 )
template< template<typename> class Container, typename InputType, typename OutputType >
void exportVectorConvertInputOutputStride( py::module & m )
{
  m.def( "vectorConvertInputOutputStride",
         []( Container<InputType> const & src, Container<OutputType> & dest, std::size_t numElements,
         std::size_t inputStride, std::size_t outputStride,  std::size_t alignment )
   {
     InputType const * const srcPtr = visr::python::bindinghelpers::ContainerAccess<Container, InputType >::constantPointer( src, numElements, "src" );
     OutputType * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, OutputType >::mutablePointer( dest, numElements, "dest" );
     detail::checkError( efl::vectorConvertInputOutputStride<InputType,
       OutputType>( srcPtr, destPtr, numElements, inputStride, outputStride,
       alignment ) );
   }, py::arg( "src" ), py::arg("dest"), py::arg("numElements"),
   py::arg( "inputStride" ), py::arg("outputStride"), py::arg("alignment")=0 );
}

} // unnamed namespace

#define ALL_CONVERSION_FUNCTIONS \
  (Convert)(ConvertInputStride)(ConvertOutputStride)\
  (ConvertInputOutputStride)

#define CONTAINER_TYPES \
  (visr::python::bindinghelpers::PyArray) (efl::BasicVector)

#define REGISTER_CONVERSION_FUNCTION( R, PRODUCT ) \
  BOOST_PP_CAT( exportVector, BOOST_PP_SEQ_ELEM( 0, PRODUCT ))<BOOST_PP_SEQ_ELEM( 1, PRODUCT ), BOOST_PP_SEQ_ELEM( 2, PRODUCT ), BOOST_PP_SEQ_ELEM( 3, PRODUCT ) >( m );

void exportVectorConversions( py::module & m)
{
    BOOST_PP_SEQ_FOR_EACH_PRODUCT( REGISTER_CONVERSION_FUNCTION,\
    (ALL_CONVERSION_FUNCTIONS)((visr::python::bindinghelpers::PyArray))\
    (VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES)(VISR_EFL_CONVERSION_FUNCTION_ALL_TYPES))

    // BasicVectors are instantiated only for float types, therefor we restrict the conversion
    // bindings taking BasicVectors to these types.
    BOOST_PP_SEQ_FOR_EACH_PRODUCT( REGISTER_CONVERSION_FUNCTION,\
    (ALL_CONVERSION_FUNCTIONS)((efl::BasicVector))\
    (VISR_EFL_CONVERSION_FUNCTION_FLOAT_TYPES)(VISR_EFL_CONVERSION_FUNCTION_FLOAT_TYPES))
}

} // namespace python
} // namespace efl
} // namespace visr
