/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "signal_flow.hpp"

#include <libvisr/channel_list.hpp>

// #include <algorithm>
#include <array>
#include <vector>

namespace visr
{
namespace apps
{
namespace feedthrough
{

Feedthrough::Feedthrough( SignalFlowContext & context,
			  char const * name,
			  CompositeComponent * parent,
			  std::size_t inputChannels,
			  std::size_t outputChannels )
 : CompositeComponent( context, name, parent )
 , mInput( "input", *this, inputChannels )
 , mOutput( "output", *this, outputChannels )
 , mTerminator( context, "Terminator", this,
		inputChannels > outputChannels ? inputChannels - outputChannels : 0 )
{
  if( inputChannels > outputChannels )
  {
    audioConnection( mInput, ChannelRange( 0, outputChannels ),
		     mOutput, ChannelRange( 0, outputChannels ) );
    audioConnection( mInput, ChannelRange( outputChannels, inputChannels ),
		     mTerminator.audioPort("input"), ChannelRange( 0, inputChannels - outputChannels ) );
  }
  else
  {
    std::size_t startOutChannel{ 0 };
    while( startOutChannel < outputChannels )
    {
      std::size_t currChannels = std::min( inputChannels, outputChannels-startOutChannel );
      audioConnection( mInput, ChannelRange( 0, currChannels ),
		       mOutput, ChannelRange( startOutChannel, startOutChannel+currChannels ) );
      startOutChannel += currChannels;
  }
  }
}

Feedthrough::~Feedthrough( ) = default;

} // namespace feedthrough
} // namespace apps
} // namespace visr
