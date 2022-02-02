/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <pybind11/pybind11.h>

// Forward declarations
namespace visr
{
namespace python
{
namespace rrl
{
void exportAudioSignalFlow( pybind11::module & m );
void exportIntegrityChecking( pybind11::module & m );
#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
void exportRuntimeProfiler( pybind11::module & m );
#endif
void exportFlexibleBufferWrapper( pybind11::module & m );
} // namespace rrl
} // namespace python
} // namespace visr

PYBIND11_MODULE( rrl, m )
{
  using namespace visr::python::rrl;
  exportAudioSignalFlow( m );
  exportIntegrityChecking( m );
#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
  exportRuntimeProfiler( m );
#endif
  exportFlexibleBufferWrapper( m );
}
