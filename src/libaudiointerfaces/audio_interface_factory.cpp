/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "audio_interface_factory.hpp"

#include "audio_interface.hpp"

#include <stdexcept>

#ifdef VISR_AUDIOINTERFACES_JACK_SUPPORT
#include <libaudiointerfaces/jack_interface.hpp>
#endif
#ifdef VISR_AUDIOINTERFACES_PORTAUDIO_SUPPORT
#include <libaudiointerfaces/portaudio_interface.hpp>
#endif

namespace visr
{
namespace audiointerfaces
{
AudioInterfaceFactory::Creator::Creator( CreateFunction fcn )
 : mCreateFunction( fcn )
{
}
std::unique_ptr< audiointerfaces::AudioInterface >
AudioInterfaceFactory::Creator::create(
    audiointerfaces::AudioInterface::Configuration const & baseConfig,
    std::string const & config ) const
{
  return std::unique_ptr< audiointerfaces::AudioInterface >(
      mCreateFunction( baseConfig, config ) );
}

std::vector< std::string > AudioInterfaceFactory::audioInterfacesList()
{
  std::vector< std::string > audioifcs;
  for( CreatorTable::const_iterator it = creatorTable().begin();
       it != creatorTable().end(); ++it )
  {
    audioifcs.push_back( it->first );
  }
  return audioifcs;
}

/*static*/ AudioInterfaceFactory::CreatorTable &
AudioInterfaceFactory::creatorTable()
{
  static AudioInterfaceFactory::CreatorTable sCreatorTable;
  return sCreatorTable;
}

/*static*/ std::unique_ptr< audiointerfaces::AudioInterface >
AudioInterfaceFactory::create(
    std::string const & interfaceName,
    audiointerfaces::AudioInterface::Configuration const & baseConfig,
    std::string const & config )
{
  CreatorTable::const_iterator findIt = creatorTable().find( interfaceName );
  if( findIt == creatorTable().end() )
  {
    throw std::invalid_argument(
        std::string( "AudioInterfaceFactory: Audio interface with name \"" ) +
        interfaceName + "\" is not registered." );
  }
  // todo: Need to catch construction errors?
  return std::unique_ptr< audiointerfaces::AudioInterface >(
      findIt->second.create( baseConfig, config ) );
}

/**
 * A helper class with whole purpose is to register the different object types
 * in the factory.
 */
struct InstantiateAudioInterfaceFactory
{
  InstantiateAudioInterfaceFactory()
  {
#ifdef VISR_AUDIOINTERFACES_JACK_SUPPORT
    AudioInterfaceFactory::registerAudioInterfaceType<
        audiointerfaces::JackInterface >( "Jack" );
#endif
#ifdef VISR_AUDIOINTERFACES_PORTAUDIO_SUPPORT
    AudioInterfaceFactory::registerAudioInterfaceType<
        audiointerfaces::PortaudioInterface >( "PortAudio" );
#endif
  }
};

/**
 * Object which is used to initialise the object factory.
 */
InstantiateAudioInterfaceFactory const cInstantiationHelper;

} // namespace audiointerfaces
} // namespace visr
