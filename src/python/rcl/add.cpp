/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librcl/add.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <pybind11/pybind11.h>
namespace visr
{
namespace python
{
namespace rcl
{

void exportAdd( pybind11::module & m )
{
  pybind11::class_<visr::rcl::Add, visr::AtomicComponent >( m, "Add",
R"(Component for adding multichannel multichannel audio signals.

Audio ports:

input_<i>"
  Audio input signals to be added, numbered from 0..`numInputs`-1. The width is  determined by the 
  constructor parameter `width`."
output
  Multichannel audio output signal, width determined by paramter **width**.)" )
    .def( pybind11::init<visr::SignalFlowContext const &, char const *, visr::CompositeComponent*, std::size_t, std::size_t>(),
          pybind11::arg( "context" ), pybind11::arg( "name" ),
          pybind11::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
          pybind11::arg( "width" ), pybind11::arg( "numInputs" ),
R"(Constructor, creates an instance of Add.

Parameters
==========

context: visr.SignalFlowContext
  Common audio processing parameters (e.g., sampling rate and block size)
name: string
  Name of the component.
parent: visr.CompositeComponent or None
  The composite component that contains the present object, or None for a 
  top-level component.
numInputs: int
  The number of input ports (addends) )" )
    .def( "process", &visr::rcl::Add::process );
}

} // namepace rcl
} // namespace python
} // namespace visr
