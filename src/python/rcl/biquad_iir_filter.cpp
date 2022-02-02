/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librcl/biquad_iir_filter.hpp>

#include <librbbl/biquad_coefficient.hpp>

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
namespace py = pybind11;

void exportBiquadIirFilter( pybind11::module & m )
{
  using visr::rcl::BiquadIirFilter;
  using SampleType = visr::SampleType;
  using visr::CompositeComponent;
  using visr::SignalFlowContext;

  py::class_< BiquadIirFilter, AtomicComponent >( m, "BiquadIirFilter",
                                                        R"( 
Multichannel IIR filtering component based on second-order IIR sections (biquads).

Audio ports:
  input: Multichannel audio signal, the witdth is determined by the constructor parameter `numberOfChannels`.
  output: Multichannel output signal, width is determined by the constructor parameter `numberOfChannels`.

Parameter ports:
  eqInput: Optional parameter input port for receiving updated EQ settings of type :obj:`pml.BiquadMatrixParameterFloat`.
           This port is activated by the constructor parameter `controlInputs` (default: :code:`True`)
)" )
      .def(
          py::init< SignalFlowContext const &, char const *,
                    CompositeComponent *, std::size_t, std::size_t, bool >(),
          py::arg( "context" ), py::arg( "name" ), py::arg( "parent" ),
          py::arg( "numberOfChannels" ), py::arg( "numberOfBiquads" ),
          py::arg( "controlInput" ) = true,
          R"(Constructor that initialises all biquad IIR sections the default value (flat EQ).

Args:
  context: (visr.SignalFlowContext) Common audio processing parameters 
           (e.g., sampling rate and block size)
  name: (string) Name of the component.
  parent: (:class:`visr.CompositeComponent` or None) The composite component that contains 
          the present object, or None in case of a top-level component.
  numberOfChannels: (int) The number of individual audio channels processed.
  numberOfBiquads: (int) The number of second-order sections processed per channels.
)" )
      .def( py::init< SignalFlowContext const &, char const *,
                      CompositeComponent *, std::size_t, std::size_t,
                      rbbl::BiquadCoefficient< SampleType > const &,
                      bool >(),
            py::arg( "context" ), py::arg( "name" ), py::arg( "parent" ),
            py::arg( "numberOfChannels" ), py::arg( "numberOfBiquads" ),
            py::arg( "initialBiquad" ), py::arg( "controlInput" ) = true,
            "Constructor initialising all biquad IIR sections to the same "
            "given value." )
      .def( py::init< SignalFlowContext const &, char const *,
                      CompositeComponent *, std::size_t, std::size_t,
                      rbbl::BiquadCoefficientList< SampleType > const &,
                      bool >(),
            py::arg( "context" ), py::arg( "name" ), py::arg( "parent" ),
            py::arg( "numberOfChannels" ), py::arg( "numberOfBiquads" ),
            py::arg( "initialBiquads" ), py::arg( "controlInput" ) = true,
            "Constructor initialising all channels to the same sequence of "
            "biquad IIR sections" )
      .def( py::init< SignalFlowContext const &, char const *,
                      CompositeComponent *, std::size_t, std::size_t,
                      rbbl::BiquadCoefficientMatrix< SampleType > const &,
                      bool >(),
            py::arg( "context" ), py::arg( "name" ), py::arg( "parent" ),
            py::arg( "numberOfChannels" ), py::arg( "numberOfBiquads" ),
            py::arg( "initialBiquads" ), py::arg( "controlInput" ) = true,
            "Constructor initialising the biquad IIR sections to individual "
            "values." );
}

} // namespace rcl
} // namespace python
} // namespace visr
