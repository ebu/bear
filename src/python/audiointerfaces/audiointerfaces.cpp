/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <pybind11/pybind11.h>

// Forward declarations
namespace visr
{
namespace python
{
namespace audiointerfaces
{
void exportAudioInterface( pybind11::module & m );
void exportAudioInterfaceFactory( pybind11::module & m );
#ifdef VISR_AUDIOINTERFACES_JACK_SUPPORT
  void exportJackInterface( pybind11::module & m );
#endif
#ifdef VISR_AUDIOINTERFACES_PORTAUDIO_SUPPORT
void exportPortaudioInterface( pybind11::module & m );
#endif
}
}
}

PYBIND11_MODULE( audiointerfaces, m )
{
  pybind11::module::import( "visr" );
  pybind11::module::import( "rrl" );

  using namespace visr::python::audiointerfaces;
  exportAudioInterface( m );
  exportAudioInterfaceFactory( m );
#ifdef VISR_AUDIOINTERFACES_JACK_SUPPORT
  exportJackInterface( m );
#endif
#ifdef VISR_AUDIOINTERFACES_PORTAUDIO_SUPPORT
  exportPortaudioInterface( m );
#endif
}
