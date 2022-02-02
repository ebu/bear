/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "options.hpp"

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>

namespace visr
{
namespace apps
{
namespace matrix_convolver
{

Options::Options()
 : apputilities::Options()
{


  registerOption<std::string>( "audio-backend,D", "The audio backend. JACK_NATIVE activates the native Jack driver insteat of the PortAudio implementation." );
  registerOption<std::string>( "audio-ifc-options", "Audio interface optional configuration" );
  registerOption<std::string>( "audio-ifc-option-file", "Audio interface optional configuration file" );


  registerOption<bool>( "list-audio-backends", "List the supported audio backends that can be passed to the the \"--audio-backend\" (\"-D\") option." );
  registerOption<bool>( "list-fft-libraries", "List the supported FFT implementations that can be selected using the \"--fftLibrary\" option." );

  registerOption<std::size_t>( "sampling-frequency,f", "Sampling frequency [Hz]" );
  registerOption<std::size_t>( "period,p", "Period (block length): The number of samples per audio block, also the block size of the partitioned convolution." );

  registerOption<std::size_t>( "input-channels,i", "Number of input channels for audio object signal." );
  registerOption<std::size_t>( "output-channels,o", "Number of audio output channels." );

  registerOption<std::string>( "filters", "Initial impulse responses, specified as comma-separated list of one or multiple WAV files." );

  registerOption<std::string>( "filter-file-index-offsets", "Index offsets to address the impulses in the provided multichannel filter files."
    " If specified, the number of values must match the number of filter files." );

  registerOption<std::string>( "routings,r", "Initial routing entries, expects a JSON array consisting of objects \"{\"inputs\": nn, \"outputs\":nn, \"filters\":nn (\"gain\":XX)" );
  registerOption<std::size_t>( "max-filter-length,l", "Maximum length of the impulse responses, in samples."
    " If not given, it defaults to the longest provided filter, " );

  registerOption<std::size_t>( "max-routings", "Maximum number of filter routings." );
  registerOption<std::size_t>( "max-filters", "Maximum number of impulse responses that can be stored." );
    
  registerOption<std::string>( "fft-library", "Specify the FFT implementation to be used. Defaults to the default implementation for the platform." );

    registerOption<bool>("dsp-optimisation,d", "Activate platform-specific optimised DSP routines. Default: off" );

}

Options::~Options()
{
}

} // namespace matrix_convolver
} // namespace apps
} // namespace visr
