/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/initialise_library.hpp>

#include <pybind11/pybind11.h>

namespace visr
{
namespace efl
{
namespace python
{

namespace py = pybind11;

void exportInitialiseLibrary( py::module & m )
{
  m
    .def( "initialiseLibrary", &visr::efl::initialiseLibrary,
	  py::arg( "processorType" ) = "" )
    .def( "uninitialiseLibrary", &visr::efl::uninitialiseLibrary )
    ;
}

} // namespace python
} // namespace efl
} // namespace visr
