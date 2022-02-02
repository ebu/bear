/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libvisr/audio_port_base.hpp>
#include <libvisr/channel_list.hpp> 

#include <libvisr/impl/audio_connection_descriptor.hpp>
#include <libvisr/impl/audio_port_base_implementation.hpp>
#include <libvisr/impl/component_implementation.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <sstream>

namespace visr
{
namespace python
{
namespace visr
{

void exportAudioConnection( pybind11::module& m )
{
  pybind11::class_<impl::AudioConnection>(m, "AudioConnection" )
   .def( pybind11::init<>(), "Default constructor" )
   .def( pybind11::init( [](AudioPortBase & sendPort, ChannelList const & sendIndices, AudioPortBase & receivePort, ChannelList const & receiveIndices )
   {
     return new impl::AudioConnection( &(sendPort.implementation()), sendIndices, &(receivePort.implementation()), receiveIndices );
   }), pybind11::arg("sendPort"), pybind11::arg("sendIndices"), pybind11::arg("receivePort"), pybind11::arg("receiveIndices"), "Full constructor (usually not needed)." )
   .def( "__lt__", &impl::AudioConnection::operator<, pybind11::arg("rhs"), "Comparison operator (for ordering connections." )
   .def_property_readonly( "sender",
                           []( impl::AudioConnection const & self ) -> AudioPortBase const &
                           {
                             return self.sender()->containingPort();
                           }, pybind11::return_value_policy::reference )
   .def_property_readonly( "receiver",
                           []( impl::AudioConnection const & self ) -> AudioPortBase const &
                           {
                             return self.receiver()->containingPort();
                           }, pybind11::return_value_policy::reference )
   .def_property_readonly( "sendIndices", &impl::AudioConnection::sendIndices )
   .def_property_readonly( "receiveIndices", &impl::AudioConnection::receiveIndices )
   .def( "__str__", []( impl::AudioConnection const & conn )
   {
     std::stringstream repr;
     repr << conn.sender()->parent().name() << ":" << conn.sender()->name() << "[" << conn.sendIndices() << "]" << "->"
          << conn.receiver()->parent().name() << ":" << conn.receiver()->name() << "[" << conn.receiveIndices() << "]";
     return repr.str();
   })
    ;
}

} // namepace visr
} // namespace python
} // namespace visr

