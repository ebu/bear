/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "options.hpp"
#include "signal_flow.hpp"

#include <librrl/audio_signal_flow.hpp>

#include <libvisr/signal_flow_context.hpp>
#include <libvisr/version.hpp>

#include <libaudiointerfaces/audio_interface.hpp>
#include <libaudiointerfaces/audio_interface_factory.hpp>

#include <boost/filesystem.hpp>

#include <ciso646>
#include <cstddef>
#include <cstdlib>
#include <cstdio> // for getc(), for testing purposes
#include <fstream>
#include <iostream>
#include <sstream>

int main( int argc, char const * const * argv )
{
    using namespace visr;
    using namespace visr::apps::feedthrough;
    
    Options cmdLineOptions;
    std::stringstream errMsg;
    switch( cmdLineOptions.parse( argc, argv, errMsg ) )
    {
    case Options::ParseResult::Failure:
      std::cerr << "Error while parsing command line options: " << errMsg.str( ) << std::endl;
      return EXIT_FAILURE;
    case Options::ParseResult::Help:
      cmdLineOptions.printDescription( std::cout );
      return EXIT_SUCCESS;
    case Options::ParseResult::Version:
      // TODO: Outsource the version string generation to a central file.
      std::cout << "VISR Matrix convolver utility " << visr::version::versionString() << std::endl;
      return EXIT_SUCCESS;
    case Options::ParseResult::Success:
      break; // carry on
    }
    
    try
    {
      if( cmdLineOptions.getDefaultedOption<bool>( "list-audio-backends", false ) )
      {
	std::cout << "Supported audio backends:" << "TBD" << std::endl;
	return EXIT_SUCCESS;
      }

      std::size_t const numberOfInputChannels = cmdLineOptions.getOption<std::size_t>( "input-channels" );
      std::size_t const numberOfOutputChannels = cmdLineOptions.getOption<std::size_t>( "output-channels" );

      std::size_t const periodSize = cmdLineOptions.getDefaultedOption<std::size_t>( "period", 1024 );
      SamplingFrequencyType const samplingFrequency = cmdLineOptions.getDefaultedOption<SamplingFrequencyType>( "sampling-frequency", 48000 );
      std::string const audioBackend = cmdLineOptions.getDefaultedOption<std::string>( "audio-backend", "default" );
      audiointerfaces::AudioInterface::Configuration const
	baseConfig( numberOfInputChannels, numberOfOutputChannels,
		    samplingFrequency, periodSize);
      std::string specConf;
      bool const hasAudioInterfaceOptionString = cmdLineOptions.hasOption("audio-ifc-options");
      bool const hasAudioInterfaceOptionFile = cmdLineOptions.hasOption("audio-ifc-option-file");
      if( hasAudioInterfaceOptionString and hasAudioInterfaceOptionFile )
      {
	throw std::invalid_argument( "The options \"--audio-ifc-options\" and \"--audio-ifc-option-file\" cannot both be given.");
      }
      if( hasAudioInterfaceOptionFile )
      {
	boost::filesystem::path const audioIfcConfigFile( cmdLineOptions.getOption<std::string>( "audio-ifc-option-file" ) );
	if( not exists( audioIfcConfigFile ) )
	{
	  throw std::invalid_argument( "The file specified by the \"--audio-ifc-option-file\" option does not exist.");
	}
	std::ifstream cfgStream( audioIfcConfigFile.string() );
	if( not cfgStream )
	{
	  throw std::invalid_argument( "The file specified by the \"--audio-ifc-option-file\" could not be read.");
	}
	std::ostringstream fileContent;
	fileContent << cfgStream.rdbuf();
	specConf = fileContent.str();
      }
      else
      {
	specConf = hasAudioInterfaceOptionString ? cmdLineOptions.getOption<std::string>( "audio-ifc-options" ) : std::string();
      }
      std::unique_ptr<audiointerfaces::AudioInterface> audioInterface
        = audiointerfaces::AudioInterfaceFactory::create( audioBackend,
							  baseConfig,
							  specConf );

      /**** SETTING UP TOP LEVEL COMPONENT AND ITS CALLBACK **************/
      SignalFlowContext context( periodSize, samplingFrequency );
      Feedthrough topLevel( context, "feedthrough", nullptr,
			    numberOfInputChannels, numberOfOutputChannels );
      rrl::AudioSignalFlow flow( topLevel );

      audioInterface->registerCallback( &rrl::AudioSignalFlow::processFunction, &flow );
      /*******************************************************************/

      audioInterface->start( );

      std::cout << "VISR feedthrough app running. Press \"q<Return>\" or Ctrl-C to quit."
		<< std::endl;
      char c;
      do
      {
	c = std::getc( stdin );
      }
      while( c != 'q' );

      std::getc( stdin );
      audioInterface->stop( );

      audioInterface->unregisterCallback( &rrl::AudioSignalFlow::processFunction );
    }
    catch( std::exception const & ex )
    {
      std::cout << "Error while setting up or rendering: "
		<< ex.what() << std::endl;
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
