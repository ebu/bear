/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace visr
{
namespace python
{
namespace visr
{

void exportSignalFlowContext(py::module & m);
void exportParameterConfigBase(py::module & m);
void exportParameterBase(py::module & m);
void exportCommunicationProtocol(py::module & m);
void exportCommunicationProtocolFactory(py::module & m);
void exportChannelList(py::module & m);
void exportTime(py::module & m);
void exportComponent(py::module & m);
void exportAudioSampleType( py::module & m );
void exportAudioPort(py::module & m);
void exportParameterFactory(py::module & m);
void exportParameterPort(py::module & m);
void exportPort( py::module & m );
void exportAudioConnection( py::module & m );
void exportParameterConnection( py::module & m );
void exportCompositeComponent(py::module & m);
void exportAtomicComponent(py::module & m);

} // namespace visr
} // namespace python
} // namespace visr

PYBIND11_MODULE( visr, m )
{
  using namespace visr::python::visr;
  exportSignalFlowContext( m );
  exportParameterConfigBase( m );
  exportParameterBase( m );
  exportCommunicationProtocol( m );
  exportCommunicationProtocolFactory( m );
  exportParameterFactory( m );
  exportPort( m );
  exportChannelList( m );
  exportTime( m );
  exportComponent( m );
  exportAudioSampleType( m );
  exportAudioPort( m );
  exportParameterPort(m);
  exportAudioConnection( m );
  exportParameterConnection( m );
  exportCompositeComponent( m );
  exportAtomicComponent( m );
}
