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
namespace feedthrough
{

Options::Options()
 : apputilities::Options()
{


  registerOption<std::string>( "audio-backend,D", "The audio backend. JACK_NATIVE activates the native Jack driver insteat of the PortAudio implementation." );
  registerOption<std::string>( "audio-ifc-options", "Audio interface optional configuration" );
  registerOption<std::string>( "audio-ifc-option-file", "Audio interface optional configuration file" );


  registerOption<bool>( "list-audio-backends", "List the supported audio backends that can be passed to the the \"--audio-backend\" (\"-D\") option." );

  registerOption<std::size_t>( "sampling-frequency,f", "Sampling frequency [Hz]" );
  registerOption<std::size_t>( "period,p", "Period (block length): The number of samples per audio block, also the block size of the partitioned convolution." );

  registerOption<std::size_t>( "input-channels,i", "Number of input channels for audio object signal." );
  registerOption<std::size_t>( "output-channels,o", "Number of audio output channels." );

  registerOption<bool>( "realtime,r", "Whether the app should be executed with high (realtime) priority." );
}

Options::~Options()
{
}

} // namespace feedthrough
} // namespace apps
} // namespace visr
