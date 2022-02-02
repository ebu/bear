/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libvisr/audio_port_base.hpp>
#include <libvisr/component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/parameter_port_base.hpp>
#include <libvisr/signal_flow_context.hpp>
#include <libvisr/status_message.hpp>

#include <libvisr/impl/component_implementation.hpp>
#include <libvisr/impl/audio_port_base_implementation.hpp>
#include <libvisr/impl/parameter_port_base_implementation.hpp>

#include <pybind11/pybind11.h>

#include <type_traits>

namespace visr
{
namespace python
{
namespace visr
{

namespace // unnamed
{

/**
 * Specialised iterator type for port implementation objects that return the containing concrete port instead.
 */
template< typename Container >
class PortIterator: public Container::iterator
{
public:
  using Base = typename Container::iterator; 

  PortIterator( Base baseIt )
    : Container::iterator( baseIt )
  {
  }

  auto operator*() -> decltype(&(Base::operator*()->containingPort()))
  // auto * operator*() // C++14 would be much slicker
  {
    return &(Base::operator*()->containingPort());
  }
};

} // unnamed namespace


void exportComponent( pybind11::module& m )
{
  pybind11::enum_<StatusMessage::Kind>( m, "StatusMessage" )
    .value( "Information", ::visr::StatusMessage::Information )
    .value( "Warning", ::visr::StatusMessage::Warning )
    .value( "Error", ::visr::StatusMessage::Error )
    .value( "Critical", ::visr::StatusMessage::Critical )
    ;

  pybind11::class_<Component>( m, "Component" ) 
    .def( pybind11::init<SignalFlowContext &, char const*, CompositeComponent *>(),
      pybind11::arg("context"), pybind11::arg("name"), pybind11::arg("parent") )
    .def_property_readonly_static( "nameSeparator", &Component::nameSeparator )
    .def_property_readonly( "name", &Component::name )
    .def_property_readonly( "fullName", &Component::fullName, pybind11::return_value_policy::reference )
    .def( "isComposite", &Component::isComposite )
    .def( "samplingFrequency", &Component::samplingFrequency ) 
    .def( "period", &Component::period )
    .def( "time", &Component::time, pybind11::return_value_policy::reference )
    .def( "isTopLevel", &Component::isTopLevel )
    .def( "audioPort", static_cast<AudioPortBase&(Component::*)(std::string const &)>(&Component::audioPort), pybind11::arg("portName"), "Return an audio port object by name", pybind11::return_value_policy::reference )
    .def( "parameterPort", static_cast<ParameterPortBase&(Component::*)(std::string const &)>(&Component::parameterPort), pybind11::arg( "portName" ), "Return a parameter port object by name", pybind11::return_value_policy::reference )
    .def( "status", static_cast<void(Component::*)(::visr::StatusMessage::Kind, char const*)>(&Component::status), pybind11::arg("statusId"), pybind11::arg("message") )
    .def_property_readonly( "audioPorts",
      []( Component & comp )
      {
        impl::ComponentImplementation::AudioPortContainer & portList = comp.implementation().audioPorts();
        PortIterator<impl::ComponentImplementation::AudioPortContainer> beginIt{ portList.begin() };
        PortIterator<impl::ComponentImplementation::AudioPortContainer> endIt{ portList.end() };
        return pybind11::make_iterator( beginIt, endIt );
      }, "Return an iterator over the audio ports of this component." )
    .def_property_readonly( "parameterPorts",
      []( Component & comp )
      {
        impl::ComponentImplementation::ParameterPortContainer & portList = comp.implementation().parameterPorts();
        PortIterator<impl::ComponentImplementation::ParameterPortContainer> beginIt{ portList.begin() };
        PortIterator<impl::ComponentImplementation::ParameterPortContainer> endIt{ portList.end() };
        return pybind11::make_iterator( beginIt, endIt );
      }, "Return an iterator over the parameter ports of this component." )
    ;
}

} // namepace visr
} // namespace python
} // namespace visr
