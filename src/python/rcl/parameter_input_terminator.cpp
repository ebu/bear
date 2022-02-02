/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librcl/parameter_input_terminator.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/communication_protocol_base.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/parameter_config_base.hpp>
#include <libvisr/parameter_port_base.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <pybind11/pybind11.h>

namespace visr
{
namespace python
{
namespace rcl
{

namespace py = pybind11;

void exportParameterInputTerminator( py::module & m )
{
  using visr::rcl::ParameterInputTerminator;

  py::class_<visr::rcl::ParameterInputTerminator,
    visr::AtomicComponent >( m, "ParameterInputTerminator" )
    .def( pybind11::init<visr::SignalFlowContext const&, char const *,
      visr::CompositeComponent*, visr::ParameterType,
      visr::CommunicationProtocolType, visr::ParameterConfigBase const &>(),
      pybind11::arg("context"), pybind11::arg("name"),
      pybind11::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
      pybind11::arg("parameterType"), pybind11::arg("protocolType"), pybind11::arg("parameterConfig") )
    .def( pybind11::init<visr::SignalFlowContext const&, char const *,
      visr::CompositeComponent*, visr::ParameterPortBase const &>(),
      pybind11::arg("context"), pybind11::arg("name"),
      pybind11::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
      pybind11::arg("templatePort") )

  ;
}

} // namepace rcl
} // namespace python
} // namespace visr
