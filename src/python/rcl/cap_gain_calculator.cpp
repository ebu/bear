/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librcl/cap_gain_calculator.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <libpanning/LoudspeakerArray.h>

#include <pybind11/pybind11.h>

namespace visr
{
namespace python
{
namespace rcl
{

void exportCAPGainCalculator( pybind11::module & m )
{
  namespace py = pybind11;

  using visr::rcl::CAPGainCalculator;

  py::class_<CAPGainCalculator, visr::AtomicComponent> cap( m, "CAPGainCalculator" );

  py::enum_<CAPGainCalculator::PanningMode>( cap, "PanningMode" )
    .value( "Nothing", CAPGainCalculator::PanningMode::Nothing )
    .value( "LF", CAPGainCalculator::PanningMode::LF )
    .value( "HF", CAPGainCalculator::PanningMode::HF )
    ;
  
  cap
   .def( py::init< visr::SignalFlowContext const&,
	 char const *, visr::CompositeComponent*,
	 std::size_t,
	 panning::LoudspeakerArray const &,
	 CAPGainCalculator::PanningMode,
	 bool>(),
	 py::arg("context"),
	 py::arg("name"),
	 py::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
	 py::arg( "numberOfObjects" ),
	 py::arg( "arrayConfig" ),
	 py::arg( "panningMode" ) = CAPGainCalculator::PanningMode::LF,
	 py::arg( "hfGainOutput" ) = false )
  ; 
}

} // namepace rcl
} // namespace python
} // namespace visr
