/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "check_error.hpp"

#include <libefl/filter_functions.hpp>

#include <libefl/basic_vector.hpp>

#include <libvisr/detail/compose_message_string.hpp>

#include <python/libpythonbindinghelpers/container_access.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <boost/preprocessor/seq/for_each_product.hpp>

#include <complex>

namespace visr
{
namespace efl
{
namespace python
{

namespace py = pybind11;

using namespace detail;

using visr::python::bindinghelpers::ContainerAccess;

template< template<typename> class Container, typename T >
void export_iirFilterBiquadsSingleChannel( py::module & m )
{
  m.def( "iirFilterBiquadsSingleChannel",
    []( Container<T> & input, Container<T> & output,
      Container<T> & states, Container<T> & coeffs,
      std::size_t numElements, std::size_t numSections,
      std::size_t stateStride, std::size_t coeffStride,
      std::size_t alignment )
	 {
	   T const * inputPtr = ContainerAccess<Container, T >::constantPointer( input, numElements, "input" );
     T * outputPtr = ContainerAccess<Container, T >::mutablePointer( output, numElements, "dest" );
     T * statesPtr = ContainerAccess<Container, T >::mutablePointer( states,
       numSections*stateStride, "states" );
     T const * coeffsPtr = ContainerAccess<Container, T >::constantPointer( coeffs,
       coeffStride * numSections, "coeffs" );
     checkError( visr::efl::iirFilterBiquadsSingleChannel<T>( inputPtr, outputPtr, statesPtr,
         coeffsPtr, numElements, numSections, stateStride, coeffStride, alignment ) );
	 }, py::arg("input"), py::arg("output"), py::arg("states"), py::arg("coeffs"),
    py::arg("numElements"), py::arg( "numSections" ),
    py::arg("stateStride") = 2, py::arg("coeffStride") = 0,
    py::arg("alignment")=0 );
}

#define ALL_FILTER_FUNCTIONS \
  (iirFilterBiquadsSingleChannel)

/**
 * The container types to be supported by the bindings.
 */
#define CONTAINER_TYPES \
( visr::python::bindinghelpers::PyArray )( efl::BasicVector )

/**
 * The numeric data types
 */
#define STANDARD_FLOAT_DATATYPES \
  (float) (double)

/**
 * Macro to create a function call exportVector[FunctionName]<ContainerType, DataType>( m );
 */
#define REGISTER_EFL_FUNCTION( R, PRODUCT ) \
  BOOST_PP_CAT( export_, BOOST_PP_SEQ_ELEM( 0, PRODUCT ))<BOOST_PP_SEQ_ELEM( 1, PRODUCT ), BOOST_PP_SEQ_ELEM( 2, PRODUCT ) >( m );

void exportFilterFunctions( py::module & m)
{
  // Register all vector functions for the standard numeric types and the two container types.
  BOOST_PP_SEQ_FOR_EACH_PRODUCT( REGISTER_EFL_FUNCTION, (ALL_FILTER_FUNCTIONS)(CONTAINER_TYPES)(STANDARD_FLOAT_DATATYPES))
}

} // namespace python
} // namespace efl
} // namespace visr
