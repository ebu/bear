/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libvisr/time.hpp>

#include <pybind11/pybind11.h>

#include <memory>

namespace visr
{
namespace python
{
namespace visr
{


void exportTime( pybind11::module & m )
{
  pybind11::class_<Time, std::unique_ptr<Time, pybind11::nodelete> >( m, "Time" )
    .def_property_readonly( "sampleCount", &Time::sampleCount )
    .def_property_readonly( "blockCount", &Time::blockCount )
    .def_property_readonly( "renderingTime", &Time::renderingTime )
    ;
}

} // namepace visr
} // namespace python
} // namespace visr
