/* Copyright Institute of Sound and Vibration Research - All rights reserved */


#include <libvisr/parameter_port_base.hpp>

#include <libvisr/impl/parameter_connection_descriptor.hpp>
#include <libvisr/impl/parameter_port_base_implementation.hpp>
#include <libvisr/impl/component_implementation.hpp>

#include <pybind11/pybind11.h>
// #include <pybind11/stl.h>

#include <sstream>

namespace visr
{
namespace python
{
namespace visr
{

void exportParameterConnection( pybind11::module& m )
{
  pybind11::class_<impl::ParameterConnection>(m, "ParameterConnection" )
   .def( pybind11::init<>(), "Default constructor" )
   .def( pybind11::init( [](ParameterPortBase & sendPort, ParameterPortBase & receivePort )
   {
     return new impl::ParameterConnection( &(sendPort.implementation()), &(receivePort.implementation()) );
   }), pybind11::arg("sendPort"), pybind11::arg("receivePort"), "Full constructor (usually not needed)." )
   .def( "__lt__", &impl::ParameterConnection::operator<, pybind11::arg("rhs"), "Comparison operator (for ordering connections." )
   .def_property_readonly( "sender",
                           []( impl::ParameterConnection const & self ) -> ParameterPortBase &
                           {
                             return self.sender()->containingPort();
                           }, pybind11::return_value_policy::reference )
   .def_property_readonly( "receiver",
                           []( impl::ParameterConnection const & self ) -> ParameterPortBase &
                           {
                             return self.receiver()->containingPort();
                           }, pybind11::return_value_policy::reference )
   .def( "__str__", []( impl::ParameterConnection const & conn )
   {
     std::stringstream repr;
     repr << conn.sender()->parent().name() << ":" << conn.sender()->name() << "->"
          << conn.receiver()->parent().name() << ":" << conn.receiver()->name();
     return repr.str();
   })
   ;
}

} // namepace visr
} // namespace python
} // namespace visr

