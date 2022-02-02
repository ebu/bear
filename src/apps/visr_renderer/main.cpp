/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "options.hpp"

#include <libefl/denormalised_number_handling.hpp>
#include <libefl/basic_matrix.hpp>
#include <libefl/vector_functions.hpp>

#include <libpanning/LoudspeakerArray.h>

#include <libpml/initialise_parameter_library.hpp>

#include <libvisr/signal_flow_context.hpp>
#include <libvisr/version.hpp>

#include <librrl/audio_signal_flow.hpp>

#include <libaudiointerfaces/audio_interface_factory.hpp>
#include <libaudiointerfaces/audio_interface.hpp>

#include <libsignalflowspython/visr_renderer.hpp>

#ifdef VISR_PYTHON_SUPPORT
#include <libpythonsupport/initialisation_guard.hpp>
#include <libpythonsupport/gil_ensure_guard.hpp>
#endif

#include <boost/algorithm/string.hpp> // case-insensitive string compare
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstdio> // for getc(), for testing purposes
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

int main( int argc, char const * const * argv )
{
    using namespace visr;
    using namespace visr::apps::baseline_renderer;
    
    try
    {
        // Load all parameters and communication protocols into the respective factories.
        pml::initialiseParameterLibrary();
        
        efl::DenormalisedNumbers::State const oldDenormNumbersState
        = efl::DenormalisedNumbers::setDenormHandling();
        
        Options cmdLineOptions;
        std::stringstream errMsg;
        switch (cmdLineOptions.parse(argc, argv, errMsg))
        {
            case Options::ParseResult::Failure:
                std::cerr << "Error while parsing command line options: " << errMsg.str() << std::endl;
                return EXIT_FAILURE;
            case Options::ParseResult::Help:
                cmdLineOptions.printDescription(std::cout);
                return EXIT_SUCCESS;
            case Options::ParseResult::Version:
                // TODO: Outsource the version string generation to a central file.
                std::cout << "VISR object-based loudspeaker renderer " << visr::version::versionString() << std::endl;
                return EXIT_SUCCESS;
            case Options::ParseResult::Success:
                break; // carry on
        }
        
        boost::filesystem::path const arrayConfigPath(cmdLineOptions.getOption<std::string>("array-config"));
        if (!exists(arrayConfigPath))
        {
            std::cerr << "The specified loudspeaker array configuration file \""
            << arrayConfigPath.string() << "\" does not exist." << std::endl;
            return EXIT_FAILURE;
        }
        panning::LoudspeakerArray loudspeakerArray( arrayConfigPath.string() );

        const std::size_t numberOfLoudspeakers = loudspeakerArray.getNumRegularSpeakers();
        const std::size_t numberOfSpeakersAndSubs = numberOfLoudspeakers + loudspeakerArray.getNumSubwoofers();
        
        const std::size_t numberOfOutputChannels
        = cmdLineOptions.getDefaultedOption<std::size_t>("output-channels", numberOfSpeakersAndSubs);
        
        const std::size_t numberOfObjects = cmdLineOptions.getOption<std::size_t>("input-channels");
        const std::size_t periodSize = cmdLineOptions.getDefaultedOption<std::size_t>("period", 1024);
        const std::size_t numberOfEqSections = cmdLineOptions.getDefaultedOption<std::size_t>("object-eq-sections", 0);
        const std::size_t samplingRate = cmdLineOptions.getDefaultedOption<std::size_t>("sampling-frequency", 44100);
        
        const std::string audioBackend = cmdLineOptions.getDefaultedOption<std::string>("audio-backend", "default");
        
        const std::size_t  sceneReceiverPort = cmdLineOptions.getDefaultedOption<std::size_t>("scene-port", 4242);
        
        const std::string trackingConfiguration = cmdLineOptions.getDefaultedOption<std::string>("tracking", std::string());
        
        // Initialise with a valid, albeit empty JSON string if no reverb config is provided. This translates to a reverb
        // rendering path with zero reverb object 'slots'
        const std::string reverbConfiguration= cmdLineOptions.getDefaultedOption<std::string>( "reverb-config", "{}" );
        
        visr::audiointerfaces::AudioInterface::Configuration const baseConfig(numberOfObjects,numberOfOutputChannels,samplingRate,periodSize);
        const bool lowFrequencyPanning = cmdLineOptions.getDefaultedOption("low-frequency-panning", false );

        std::string const metadapterConfig = cmdLineOptions.getDefaultedOption<std::string>( "metadapter-config", std::string() );

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

      
        /********************************* SETTING TOP LEVEL COMPONENT AND ITS CALLBACK  **********************************/
        // Assume a fixed length for the interpolation period.
        // Ideally, this roughly matches the update rate of the scene sender.
        const std::size_t cInterpolationLength = std::max( static_cast<std::size_t>(2048), periodSize );

        /* Set up the filter matrix for the diffusion filters. */
        std::size_t const diffusionFilterLength = 63; // fixed filter length of the filters in the compiled-in matrix
        std::size_t const diffusionFiltersInFile = 64; // Fixed number of filters in file.
        // First create a filter matrix containing all filters from a initializer list that is compiled into the program.
        efl::BasicMatrix<SampleType> allDiffusionCoeffs( diffusionFiltersInFile,
                                                        diffusionFilterLength,
#include "files/quasiAllpassFIR_f64_n63_initializer_list.txt"
                                                        , cVectorAlignmentSamples );
        
        // Create a second filter matrix that matches the number of required filters.
        efl::BasicMatrix<SampleType> diffusionCoeffs( numberOfLoudspeakers, diffusionFilterLength, cVectorAlignmentSamples );
        for( std::size_t idx( 0 ); idx < diffusionCoeffs.numberOfRows( ); ++idx )
        {
            efl::vectorCopy( allDiffusionCoeffs.row( idx ), diffusionCoeffs.row( idx ), diffusionFilterLength, cVectorAlignmentSamples );
        }
        
        SignalFlowContext context( periodSize, samplingRate );
        
        std::unique_ptr<signalflowspython::VisrRenderer> renderer;
        
        {
#if VISR_PYTHON_SUPPORT
          visr::pythonsupport::InitialisationGuard::initialise();
          visr::pythonsupport::GilEnsureGuard guard;
#endif
          renderer.reset( new signalflowspython::VisrRenderer( 
                                           context,
                                           "", nullptr,
                                           loudspeakerArray,
                                           numberOfObjects,
                                           numberOfOutputChannels,
                                           cInterpolationLength,
                                           diffusionCoeffs,
                                           trackingConfiguration,
                                           sceneReceiverPort,
                                           numberOfEqSections,
                                           reverbConfiguration,
                                           lowFrequencyPanning,
                                           metadapterConfig ) );

        }
        
        rrl::AudioSignalFlow audioFlow( *renderer );
        std::unique_ptr<visr::audiointerfaces::AudioInterface>
          audioInterface( audiointerfaces::AudioInterfaceFactory::create( audioBackend, baseConfig, specConf ) );

        audioInterface->registerCallback( &rrl::AudioSignalFlow::processFunction, &audioFlow );

        /*******************************************************************/
        
        
        // should there be a separate start() method for the audio interface?
        audioInterface->start( );
        
        // Rendering runs until q<Return> is entered on the console.
        std::cout << "VISR object-based renderer running. Press \"q<Return>\" or Ctrl-C to quit." << std::endl;
        char c;
        do
        {
            c = std::getc( stdin );
        }
        while( c != 'q' );
        
        audioInterface->stop( );
        
//        audioInterface->unregisterCallback( &rrl::AudioSignalFlow::processFunction );
        
        efl::DenormalisedNumbers::resetDenormHandling( oldDenormNumbersState );
    }
    catch( std::exception const & ex )
    {
        std::cout << "Exception caught on top level: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
