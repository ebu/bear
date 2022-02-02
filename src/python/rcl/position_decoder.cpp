/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librcl/position_decoder.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <libpanning/LoudspeakerArray.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace visr
{
namespace python
{
namespace rcl
{

void exportPositionDecoder( pybind11::module & m )
{
  pybind11::class_<visr::rcl::PositionDecoder, visr::AtomicComponent >( m, "PositionDecoder" )
    .def( pybind11::init<visr::SignalFlowContext const &, char const *, visr::CompositeComponent*,
          pml::ListenerPosition::PositionType const &,
          pml::ListenerPosition::OrientationYPR const &>(),
          pybind11::arg( "context" ), pybind11::arg( "name" ),
          pybind11::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
          pybind11::arg("positionOffset") = pml::ListenerPosition::PositionType(),
          pybind11::arg("orientation") = pml:: ListenerPosition::OrientationYPR() )
    .def( pybind11::init<visr::SignalFlowContext const &, char const *, visr::CompositeComponent*,
          pml::ListenerPosition::PositionType const &,
          pml::ListenerPosition::OrientationQuaternion const &>(),
          pybind11::arg( "context" ), pybind11::arg( "name" ),
          pybind11::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
          pybind11::arg("positionOffset") = pml::ListenerPosition::PositionType(),
          pybind11::arg("orientation") )
  ;
}

} // namepace rcl
} // namespace python
} // namespace visr
