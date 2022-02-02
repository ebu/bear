/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librcl/time_frequency_transform.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <libefl/basic_vector.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace visr
{
namespace python
{
namespace rcl
{

namespace py = pybind11;

void exportTimeFrequencyTransform( py::module & m )
{
  using visr::rcl::TimeFrequencyTransform;

  py::class_<TimeFrequencyTransform, visr::AtomicComponent> tft( m, "TimeFrequencyTransform" );

  py::enum_< TimeFrequencyTransform::Normalisation >( tft, "Normalisation")
    .value( "One", TimeFrequencyTransform::Normalisation::One )
    .value( "Unitary", TimeFrequencyTransform::Normalisation::Unitary )
    .value( "OneOverN", TimeFrequencyTransform::Normalisation::OneOverN )
  ;

  tft
   .def( py::init<visr::SignalFlowContext const&, char const *, visr::CompositeComponent*,
         std::size_t, std::size_t, std::size_t, std::size_t, char const *, TimeFrequencyTransform::Normalisation>(),
     py::arg("context"), py::arg("name"), py::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
     py::arg( "numberOfChannels" ), py::arg( "dftSize" ), py::arg( "windowLength" ), py::arg( "hopSize" ),
     py::arg( "fftImplementation" ) = "default",
     py::arg( "normalisation") = TimeFrequencyTransform::Normalisation::Unitary )
   .def( py::init<visr::SignalFlowContext const&, char const *, visr::CompositeComponent*,
     std::size_t, std::size_t, efl::BasicVector<visr::rcl::TimeFrequencyTransform::SampleType> const &, std::size_t, char const *, TimeFrequencyTransform::Normalisation>(),
     py::arg( "context" ), py::arg( "name" ), py::arg( "parent" ) = static_cast<visr::CompositeComponent*>(nullptr),
     py::arg( "numberOfChannels" ), py::arg( "dftSize" ), py::arg( "window" ), py::arg( "hopSize" ), py::arg( "fftImplementation" ) = "default", py::arg( "normalisation") = TimeFrequencyTransform::Normalisation::Unitary )
   .def( py::init( []( SignalFlowContext const & context,
          char const * name, CompositeComponent * parent, std::size_t numberOfChannels, std::size_t dftSize,
         std::vector<TimeFrequencyTransform::SampleType> const & window, std::size_t hopSize, char const * fftImplementation, TimeFrequencyTransform::Normalisation normalisation  )
     {
       efl::BasicVector<TimeFrequencyTransform::SampleType> windowVec( window.size(), cVectorAlignmentSamples );
       if( not window.empty() )
       {
         std::copy(window.begin(), window.end(), windowVec.data() );
       }
       return new TimeFrequencyTransform( context, name, parent, numberOfChannels, dftSize,
                                          windowVec, hopSize,fftImplementation, normalisation );
    }), py::arg( "context" ), py::arg( "name" ), py::arg( "parent" ) = static_cast<visr::CompositeComponent*>(nullptr),
      py::arg( "numberOfChannels" ), py::arg( "dftSize" ), py::arg( "window" ), py::arg( "hopSize" ),
      py::arg( "fftImplementation" ) = "default",
      py::arg( "normalisation") = TimeFrequencyTransform::Normalisation::Unitary
   )
  ;
}

} // namepace rcl
} // namespace python
} // namespace visr
