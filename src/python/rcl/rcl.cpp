/* Copyright Institute of Sound and Vibration Research - All rights reserved */


#include <pybind11/pybind11.h>

// Forward declarations
namespace visr
{
namespace python
{
namespace rcl
{
  void exportAdd( pybind11::module & m );
  void exportBiquadIirFilter( pybind11::module & m );
  void exportCAPGainCalculator( pybind11::module & m );
  void exportChannelObjectRoutingCalculator( pybind11::module & m );
  void exportCrossfadingFirFilterMatrix( pybind11::module & m );
  void exportDelayVector( pybind11::module & m );
  void exportDelayMatrix( pybind11::module & m );
  void exportDiffusionGainCalculator( pybind11::module & m );
  void exportFirFilterMatrix( pybind11::module & m );
  void exportGainMatrix( pybind11::module & m );
  void exportGainVector( pybind11::module & m );
  void exportHoaAllRadGainCalculator( pybind11::module & m );
  void exportInterpolatingFirFilterMatrix( pybind11::module & m );
  void exportListenerCompensation( pybind11::module & m );
  void exportNullSource( pybind11::module & m );
  void exportObjectGainEqCalculator( pybind11::module & m );
  void exportPanningCalculator( pybind11::module & m );
  void exportParameterInputTerminator( pybind11::module & m );
  void exportParameterOutputTerminator( pybind11::module & m );
  void exportPositionDecoder( pybind11::module & m );
  void exportScalarOscDecoder( pybind11::module & m );
  void exportSceneDecoder( pybind11::module & m );
  void exportSceneEncoder( pybind11::module & m );
  void exportSignalRouting( pybind11::module & m );
  void exportSparseGainMatrix( pybind11::module & m );
  void exportTimeFrequencyTransform( pybind11::module & m );
  void exportTimeFrequencyInverseTransform( pybind11::module & m );
  void exportUdpReceiver( pybind11::module & m );
  void exportUdpSender( pybind11::module & m );
}
}
}

PYBIND11_MODULE(rcl, m)
{
  pybind11::module::import( "visr" );
  pybind11::module::import( "panning" );
  pybind11::module::import( "pml" );

  m.doc() = 
R"(VISR default component library.

At the moment, this module contains atomic components (whereas the composite ones are in signalflows) for historic reasons.
In the future, however, this will change do a topical orgainisation.
)";

  using namespace visr::python::rcl;
  exportAdd( m );
  exportBiquadIirFilter( m );
  exportCAPGainCalculator( m );
  exportChannelObjectRoutingCalculator( m );
  exportCrossfadingFirFilterMatrix( m );
  exportDelayMatrix( m );
  exportDelayVector( m );
  exportDiffusionGainCalculator( m );
  exportFirFilterMatrix( m );
  exportGainMatrix( m );
  exportGainVector( m );
  exportHoaAllRadGainCalculator( m );
  exportInterpolatingFirFilterMatrix( m );
  exportListenerCompensation( m );
  exportNullSource( m );
  exportObjectGainEqCalculator( m );
  exportPanningCalculator( m );
  exportParameterInputTerminator( m );
  exportParameterOutputTerminator( m );
  exportPositionDecoder( m );
  exportScalarOscDecoder( m );
  exportSceneDecoder( m );
  exportSceneEncoder( m );
  exportSignalRouting( m );
  exportSparseGainMatrix( m );
  exportTimeFrequencyInverseTransform( m );
  exportTimeFrequencyTransform( m );
  exportUdpReceiver( m );
  exportUdpSender( m );
}
