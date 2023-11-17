/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpml/initialise_parameter_library.hpp>

#include <pybind11/pybind11.h>

namespace visr
{
namespace python
{
namespace pml
{
void exportDoubleBufferingProtocol( pybind11::module & m );
void exportMessageQueueProtocol( pybind11::module & m );
void exportSharedDataProtocol( pybind11::module & m );

void exportBiquadCoefficientParameter( pybind11::module & m );
void exportEmptyParameterConfig( pybind11::module & m );
void exportFilterRoutingParameter( pybind11::module & m );
void exportIndexedValueParameters( pybind11::module & m );
void exportInterpolationParameter( pybind11::module & m );
void exportListenerPosition( pybind11::module & m );
void exportMatrixParameters( pybind11::module & m );
void exportObjectVector( pybind11::module & m );
void exportScalarParameters( pybind11::module & m );
void exportSignalRoutingParameter( pybind11::module & m );
void exportSparseGainRoutingParameter( pybind11::module & m );
void exportStringParameter( pybind11::module & m );
void exportTimeFrequencyParameter( pybind11::module & m );
void exportVectorParameters( pybind11::module & m );
}
}
}


PYBIND11_MODULE( pml, m )
{
  pybind11::module::import( "efl" );
  pybind11::module::import( "visr" );
  pybind11::module::import( "objectmodel" );
  pybind11::module::import( "rbbl" );

  using namespace visr::python::pml;

  // Export the communication protocols
  exportDoubleBufferingProtocol( m );
  exportMessageQueueProtocol( m );
  exportSharedDataProtocol( m );

  exportBiquadCoefficientParameter( m );
  exportEmptyParameterConfig( m );
  exportFilterRoutingParameter( m );
  exportIndexedValueParameters( m );
  exportInterpolationParameter( m );
  exportListenerPosition( m );
  exportMatrixParameters( m);
  exportObjectVector( m );
  exportScalarParameters( m );
  exportSignalRoutingParameter( m );
  exportSparseGainRoutingParameter( m );
  exportStringParameter( m );
  exportTimeFrequencyParameter( m );
  exportVectorParameters( m );

  // Register parameter types and communication protocols
  visr::pml::initialiseParameterLibrary();
}
