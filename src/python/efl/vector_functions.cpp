/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "check_error.hpp"

#include <libefl/vector_functions.hpp>

#include <python/libpythonbindinghelpers/container_access.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <boost/preprocessor/seq/for_each_product.hpp>

#include <complex>
#include <cstdint>

namespace visr
{
namespace efl
{
namespace python
{

namespace py = pybind11;

namespace // unnamed
{

template< template<typename> class Container, typename T >
void exportVectorZero( py::module & m )
{
  m.def( "vectorZero",
         []( Container<T> & dest, std::size_t numElements, std::size_t alignment )
	 {
	   T * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( dest, numElements, "dest" );
	   detail::checkError( visr::efl::vectorZero<T>( destPtr, numElements, alignment ) );
	 }, py::arg("dest"), py::arg("numElements"), py::arg("alignment")=0 );
}

template< template<typename> class Container, typename T >
void exportVectorFill( py::module & m )
{
  m.def( "vectorFill",
         []( const T value, Container<T> & dest, std::size_t numElements, std::size_t alignment )
	 {
	   T * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( dest, numElements, "dest" );
	   detail::checkError( visr::efl::vectorFill<T>( value, destPtr, numElements, alignment ) );
	 }, py::arg("value"), py::arg("dest"), py::arg("numElements"), py::arg("alignment")=0 );
}

template< template<typename> class Container, typename T >
void exportVectorCopy( py::module & m )
{
  m.def( "vectorCopy",
         []( Container<T> const & src, Container<T> & dest, std::size_t numElements, std::size_t alignment )
	 {
	   T const * const srcPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( src, numElements, "src" );
	   T * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( dest, numElements, "dest" );
	   detail::checkError( visr::efl::vectorCopy<T>( srcPtr, destPtr, numElements, alignment ) );
	 }, py::arg("src"), py::arg("dest"), py::arg("numElements"), py::arg("alignment")=0 );
}

template< template<typename> class Container, typename T >
void exportVectorCopyStrided( py::module & m )
{
  m.def( "vectorCopyStrided",
         []( Container<T> const & src, Container<T> & dest, std::size_t sourceStrideSamples, std::size_t destStrideSamples,
         std::size_t numElements, std::size_t alignment )
	 {
	   T const * const srcPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( src, numElements*sourceStrideSamples, "src" );
	   T * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( dest, numElements*destStrideSamples, "dest" );
	   detail::checkError( visr::efl::vectorCopyStrided<T>( srcPtr, destPtr, sourceStrideSamples,
                                                          destStrideSamples, numElements, alignment ) );
	 }, py::arg("src"), py::arg("dest"), py::arg( "srcStride")=1, py::arg( "destStride")=1, py::arg("numElements"), py::arg("alignment")=0 );
}

template< template<typename> class Container, typename T >
void exportVectorRamp( py::module & m )
{
  m.def( "vectorRamp",
         []( Container<T> & dest, std::size_t numElements, T startVal, T endVal,
             bool startInclusive, bool endInclusive, std::size_t alignment )
	 {
	   T * const destPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( dest, numElements, "dest" );
	   detail::checkError( visr::efl::vectorRamp<T>( destPtr, numElements,
						 startVal, endVal, startInclusive, endInclusive, alignment ) );
	 }, py::arg("dest"), py::arg("numElements"), py::arg("startVal"), py::arg("endVal"),
	    py::arg("startInclusive")=true, py::arg("endInclusive")=true, py::arg("alignment")=0 );
}

template< template<typename> class Container, typename T >
void exportVectorAdd( py::module & m )
{
  m.def( "vectorAdd",
         []( Container<T> const & op1, Container<T> const & op2, Container<T> & result,
	     std::size_t numElements, std::size_t alignment )
	 {
	   T const * const op1Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( op1, numElements, "op1" );
	   T const * const op2Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( op2, numElements, "op2" );
	   T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( result, numElements, "result" );
	   detail::checkError( visr::efl::vectorAdd<T>( op1Ptr, op2Ptr, resultPtr, numElements, alignment ) );
	 }, py::arg("op1"), py::arg("op2"), py::arg("result"), py::arg("numElements"), py::arg("alignment")=0 );
}

//template<typename T>
//ErrorCode vectorAddInplace(T const * const op1,
//  T * const op2Result,
//  std::size_t numElements,
//  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorAddInplace(py::module & m)
{
  m.def("vectorAddInplace",
    [](Container<T> const & addend1, Container<T> & addend2Result,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const addend1Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(addend1, numElements, "addend1");
    T * const addend2ResultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(addend2Result, numElements, "addend2Result");
    detail::checkError(visr::efl::vectorAddInplace<T>(addend1Ptr, addend2ResultPtr, numElements, alignment));
  }, py::arg("addend1"), py::arg("addend2Result"), py::arg("numElements"), py::arg("alignment") = 0);
}

//template<typename T>
//ErrorCode vectorAddConstant(T constantValue,
//  T const * const op,
//  T * const result,
//  std::size_t numElements,
//  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorAddConstant(py::module & m)
{
  m.def("vectorAddConstant",
    []( T constantValue, Container<T> const & addend, Container<T> & result,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const addendPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(addend, numElements, "addend");
    T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(result, numElements, "result");
    detail::checkError(visr::efl::vectorAddConstant<T>(constantValue, addendPtr, resultPtr, numElements, alignment));
  }, py::arg("constantValue"), py::arg("addend"), py::arg("result"), py::arg("numElements"), py::arg("alignment") = 0);
}

//template<typename T>
//ErrorCode vectorAddConstantInplace(T constantValue,
//  T * const opResult,
//  std::size_t numElements,
//  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorAddConstantInplace(py::module & m)
{
  m.def("vectorAddConstantInplace",
    [](T constantValue, Container<T> & addendResult,
      std::size_t numElements, std::size_t alignment)
  {
    T * const addendResultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(addendResult, numElements, "addendResult");
    detail::checkError(visr::efl::vectorAddConstantInplace<T>(constantValue, addendResultPtr, numElements, alignment));
  }, py::arg("constantValue"), py::arg("addendResult"), py::arg("numElements"), py::arg("alignment") = 0);
}

//template<typename T>
//ErrorCode vectorSubtract(T const * const subtrahend,
//  T const * const minuend,
//  T * const result,
//  std::size_t numElements,
//  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorSubtract(py::module & m)
{
  m.def("vectorSubtract",
    [](Container<T> const & subtrahend, Container<T> const & minuend, Container<T> & result,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const subtrahendPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(subtrahend, numElements, "op1");
    T const * const minuendPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(minuend, numElements, "op2");
    T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(result, numElements, "result");
    detail::checkError(visr::efl::vectorSubtract<T>(subtrahendPtr, minuendPtr, resultPtr, numElements, alignment));
  }, py::arg("subtrahend"), py::arg("minuend"), py::arg("result"), py::arg("numElements"), py::arg("alignment") = 0);
}

//template<typename T>
//ErrorCode vectorSubtractInplace(T const * const minuend,
//  T * const subtrahendResult,
//  std::size_t numElements,
//  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorSubtractInplace(py::module & m)
{
  m.def("vectorSubtractInplace",
    [](Container<T> const & minuend, Container<T> & subtrahendResult,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const minuendPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(minuend, numElements, "addend1");
    T * const subtrahendResultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(subtrahendResult, numElements, "addend2Result");
    detail::checkError(visr::efl::vectorSubtractInplace<T>(minuendPtr, subtrahendResultPtr, numElements, alignment));
  }, py::arg("minuend"), py::arg("subtrahendResult"), py::arg("numElements"), py::arg("alignment") = 0);
}

//template<typename T>
//ErrorCode vectorSubtractConstant(T constantMinuend,
//  T const * const subtrahend,
//  T * const result,
//  std::size_t numElements,
//  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorSubtractConstant(py::module & m)
{
  m.def("vectorSubtractConstant",
    [](T constantMinuend, Container<T> const & subtrahend, Container<T> & result,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const subtrahendPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(subtrahend, numElements, "subtrahend");
    T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(result, numElements, "result");
    detail::checkError(visr::efl::vectorSubtractConstant<T>(constantMinuend, subtrahendPtr, resultPtr, numElements, alignment));
  }, py::arg("constantMinuend"), py::arg("subtrahend"), py::arg("result"), py::arg("numElements"), py::arg("alignment") = 0);
}

//template<typename T>
//ErrorCode vectorSubtractConstantInplace(T constantMinuend,
//  T * const subtrahendResult,
//  std::size_t numElements,
//  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorSubtractConstantInplace(py::module & m)
{
  m.def("vectorSubtractConstantInplace",
    [](T constantMinuend, Container<T> & subtrahendResult,
      std::size_t numElements, std::size_t alignment)
  {
    T * const subtrahendResultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(subtrahendResult, numElements, "subtrahendResult");
    detail::checkError(visr::efl::vectorSubtractConstantInplace<T>(constantMinuend, subtrahendResultPtr, numElements, alignment));
  }, py::arg("constantMinuend"), py::arg("subtrahendResult"), py::arg("numElements"), py::arg("alignment") = 0);
}

template< template<typename> class Container, typename T >
void exportVectorMultiply( py::module & m )
{
  m.def( "vectorMultiply",
         []( Container<T> const & op1, Container<T> const & op2, Container<T> & result,
	     std::size_t numElements, std::size_t alignment )
	 {
	   T const * const op1Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( op1, numElements, "op1" );
	   T const * const op2Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( op2, numElements, "op2" );
	   T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( result, numElements, "result" );
	   detail::checkError( visr::efl::vectorMultiply<T>( op1Ptr, op2Ptr, resultPtr, numElements, alignment ) );
	 }, py::arg("factor1"), py::arg("factor2"), py::arg("result"), py::arg("numElements"), py::arg("alignment")=0 );
}

// template<typename T>
// ErrorCode vectorMultiplyInplace( T const * const factor1,
//                                  T * const factor2Result,
//                                  std::size_t numElements,
//                                  std::size_t alignment = 0)
template< template<typename> class Container, typename T >
void exportVectorMultiplyInplace( py::module & m )
{
  m.def( "vectorMultiplyInplace",
         []( Container<T> const & factor1, Container<T> & factor2Result,
             std::size_t numElements, std::size_t alignment )
           {
             T const * const factor1Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( factor1, numElements, "factor1" );
             T * const factor2ResultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer( factor2Result, numElements, "factor2Result" );
             detail::checkError( visr::efl::vectorMultiplyInplace<T>( factor1Ptr, factor2ResultPtr, numElements, alignment ) );
           }, py::arg("factor1"), py::arg("factor2Result"), py::arg("numElements"), py::arg("alignment")=0 );
}

// template<typename T>
// ErrorCode vectorMultiplyConstant( T constantValue,
//                                   T const * const factor,
//                                   T * const result,
//                                   std::size_t numElements,
//                                   std::size_t alignment = 0 )
template< template<typename> class Container, typename T >
void exportVectorMultiplyConstant(py::module & m)
{
  m.def("vectorMultiplyConstant",
    [](T constantValue, Container<T> const & factor, Container<T> & result,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const factorPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(factor, numElements, "factor");
    T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(result, numElements, "result");
    detail::checkError(visr::efl::vectorMultiplyConstant<T>(constantValue, factorPtr, resultPtr, numElements, alignment));
  }, py::arg("constantValue"), py::arg("factor"), py::arg("result"), py::arg("numElements"), py::arg("alignment") = 0);
}

// template<typename T>
// ErrorCode vectorMultiplyConstantInplace( T constantValue,
//                                          T * const factorResult,
//                                          std::size_t numElements,
//                                          std::size_t alignment = 0 )
template< template<typename> class Container, typename T >
void exportVectorMultiplyConstantInplace(py::module & m)
{
  m.def("vectorMultiplyConstantInplace",
    [](T constantValue, Container<T> & factorResult,
      std::size_t numElements, std::size_t alignment)
  {
    T * const factorResultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(factorResult, numElements, "factorResult");
    detail::checkError(visr::efl::vectorMultiplyConstantInplace<T>( constantValue, factorResultPtr, numElements, alignment));
  }, py::arg("constantValue"), py::arg("factorResult"), py::arg("numElements"), py::arg("alignment") = 0);
}

// template<typename T>
// ErrorCode vectorMultiplyAdd( T const * const factor1,
//   T const * const factor2,
//   T const * const addend,
//   T * const result,
//   std::size_t numElements,
//   std::size_t alignment = 0 )
template< template<typename> class Container, typename T >
void exportVectorMultiplyAdd(py::module & m)
{
  m.def("vectorMultiplyAdd",
    [](Container<T> const & factor1, Container<T> const & factor2,
      Container<T> const & addend, Container<T> & result,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const factor1Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(factor1, numElements, "factor1");
    T const * const factor2Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(factor2, numElements, "factor2");
    T const * const addendPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(addend, numElements, "addend");
    T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(result, numElements, "");
    detail::checkError(visr::efl::vectorMultiplyAdd<T>(factor1Ptr, factor2Ptr, addendPtr, resultPtr, numElements, alignment));
  }, py::arg("factor1"), py::arg("factor2"), py::arg("addend"), py::arg("result"), py::arg("numElements"), py::arg("alignment") = 0);
}

// template<typename T>
// ErrorCode vectorMultiplyAddInplace( T const * const factor1,
//   T const * const factor2,
//   T * const accumulator,
//   std::size_t numElements,
//   std::size_t alignment = 0 )
template< template<typename> class Container, typename T >
void exportVectorMultiplyAddInplace(py::module & m)
{
  m.def("vectorMultiplyAddInplace",
    [](Container<T> const & factor1, Container<T> const & factor2,
       Container<T> & accumulator,
       std::size_t numElements, std::size_t alignment)
  {
    T const * const factor1Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(factor1, numElements, "factor1");
    T const * const factor2Ptr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(factor2, numElements, "factor2");
    T * const accumulatorPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(accumulator, numElements, "accumulator");
    detail::checkError(visr::efl::vectorMultiplyAddInplace<T>(factor1Ptr, factor2Ptr, accumulatorPtr, numElements, alignment));
  }, py::arg("factor1"), py::arg("factor2"), py::arg("accumulator"), py::arg("numElements"), py::arg("alignment") = 0);
}

// template<typename T>
// ErrorCode vectorMultiplyConstantAdd( T constFactor,
//                                      T const * const factor,
//                                      T const * const addend,
//                                      T * const result,
//                                      std::size_t numElements,
//                                      std::size_t alignment = 0 )
template< template<typename> class Container, typename T >
void exportVectorMultiplyConstantAdd(py::module & m)
{
  m.def("vectorMultiplyConstantAdd",
    [](T constFactor, Container<T> const & factor, Container<T> const & addend, Container<T> & result,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const factorPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(factor, numElements, "factor");
    T const * const addendPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(addend, numElements, "addend");
    T * const resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(result, numElements, "");
    detail::checkError(visr::efl::vectorMultiplyConstantAdd<T>(constFactor, factorPtr, addendPtr, resultPtr, numElements, alignment));
  }, py::arg("constantFactor"), py::arg("factor"), py::arg("addend"), py::arg("result"), py::arg("numElements"), py::arg("alignment") = 0);
}

// template<typename T>
// ErrorCode vectorMultiplyConstantAddInplace( T constFactor,
//   T const * const factor,
//   T * const accumulator,
//   std::size_t numElements,
//   std::size_t alignment = 0 )
template< template<typename> class Container, typename T >
void exportVectorMultiplyConstantAddInplace(py::module & m)
{
  m.def("vectorMultiplyConstantAddInplace",
    [](T constFactor, Container<T> const & factor,Container<T> & accumulator,
      std::size_t numElements, std::size_t alignment)
  {
    T const * const factorPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(factor, numElements, "factor");
    T * const accumulatorPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(accumulator, numElements, "accumulator");
    detail::checkError(visr::efl::vectorMultiplyConstantAddInplace<T>(constFactor, factorPtr, accumulatorPtr, numElements, alignment));
  }, py::arg("constantValue"), py::arg("factor"), py::arg("accumulator"), py::arg("numElements"), py::arg("alignment") = 0);
}

//template<typename T>
//efl::ErrorCode vectorRampScaling(T const * input,
//  T const * ramp,
//  T * output,
//  T baseGain,
//  T rampGain,
//  std::size_t numberOfElements,
//  bool accumulate /*= false*/,
//  std::size_t alignmentElements /*= 0*/)
template< template<typename> class Container, typename T >
void exportVectorRampScaling(py::module & m)
{
  m.def("vectorRampScaling",
    [](Container<T> const & input, Container< T > const & ramp,
      Container<T> & output, T baseGain,
      T rampGain,
      std::size_t numElements, bool accumulate, std::size_t alignment)
  {
    T const * const inputPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer(input, numElements, "input");
    T const * const rampPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::constantPointer( ramp, numElements, "ramp`");
    T * const outputPtr = visr::python::bindinghelpers::ContainerAccess<Container, T >::mutablePointer(output, numElements, "output");
    detail::checkError(visr::efl::vectorRampScaling<T>(inputPtr, rampPtr, outputPtr,
      baseGain, rampGain, numElements, accumulate, alignment));
  }, py::arg("input"), py::arg("ramp"), py::arg("output"), py::arg( "baseGain" ),
    py::arg( "rampGain" ), py::arg("numElements"), py::arg( "accumulate" ) = false, py::arg("alignment") = 0 );
}

} // namespace unnamed

#define ALL_VECTOR_FUNCTIONS \
  (Zero)\
  (Fill) (Ramp) (Copy) (CopyStrided) \
  (Add) (AddInplace) (AddConstant) (AddConstantInplace) \
  (Subtract) (SubtractInplace) (SubtractConstant) (SubtractConstantInplace) \
  (Multiply) (MultiplyInplace) (MultiplyConstant) (MultiplyConstantInplace) \
  (MultiplyAdd) (MultiplyAddInplace) (MultiplyConstantAdd) (MultiplyConstantAddInplace)\
  (RampScaling)
/**
 * The container types to be supported by the bindings.
 */
#define CONTAINER_TYPES \
  (visr::python::bindinghelpers::PyArray) (efl::BasicVector)

/**
 * The numeric data types
 */
#define STANDARD_NUMERIC_DATATYPES \
  (float) (double)(std::complex<float>)(std::complex<double>)

/**
 * Macro to create a function call exportVector[FunctionName]<ContainerType, DataType>( m );
 */
#define REGISTER_VECTOR_FUNCTION( R, PRODUCT ) \
  BOOST_PP_CAT( exportVector, BOOST_PP_SEQ_ELEM( 0, PRODUCT ))<BOOST_PP_SEQ_ELEM( 1, PRODUCT ), BOOST_PP_SEQ_ELEM( 2, PRODUCT ) >( m );

void exportVectorFunctions( py::module & m)
{
  // Register all vector functions for the standard numeric types and the two container types.
  BOOST_PP_SEQ_FOR_EACH_PRODUCT( REGISTER_VECTOR_FUNCTION, (ALL_VECTOR_FUNCTIONS)(CONTAINER_TYPES)(STANDARD_NUMERIC_DATATYPES))

  // Register vectorCopy() for a number of additional datatypes (only for py::array_t)
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(REGISTER_VECTOR_FUNCTION, ((Copy))((visr::python::bindinghelpers::PyArray))\
      ((long double)(int8_t)(uint8_t)(int16_t)(uint16_t)(int32_t)(uint32_t)))
}

} // namespace python
} // namespace efl
} // namespace visr
