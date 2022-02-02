/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "options.hpp"
#include "init_filter_matrix.hpp"

#include <libefl/denormalised_number_handling.hpp>
#include <libefl/initialise_library.hpp>

#include <librbbl/fft_wrapper_factory.hpp>
#include <librbbl/filter_routing.hpp>
#include <librbbl/index_sequence.hpp>

#include <libvisr/signal_flow_context.hpp>
#include <libvisr/version.hpp>

#include <librcl/fir_filter_matrix.hpp>

#include <librrl/audio_signal_flow.hpp>
#include <libaudiointerfaces/audio_interface_factory.hpp>

#include <libaudiointerfaces/audio_interface_factory.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

int main( int argc, char const * const * argv )
{
  using namespace visr;
  using namespace visr::apps::matrix_convolver;

  efl::DenormalisedNumbers::State const oldDenormNumbersState
    = efl::DenormalisedNumbers::setDenormHandling( );

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
    // Query options
    if( cmdLineOptions.getDefaultedOption<bool>( "list-audio-backends", false ) )
    {
      std::cout << "Supported audio backends:" << "TBD" << std::endl;
      return EXIT_SUCCESS;
    }
    if( cmdLineOptions.getDefaultedOption<bool>( "list-fft-libraries", false ) )
    {
      std::cout << "Supported FFT libraries:"
          << rbbl::FftWrapperFactory<SampleType>::listImplementations() << std::endl;
      return EXIT_SUCCESS;
    }

    std::string const audioBackend = cmdLineOptions.getDefaultedOption<std::string>( "audio-backend", "default" );

    std::size_t const numberOfInputChannels = cmdLineOptions.getOption<std::size_t>( "input-channels" );
    std::size_t const numberOfOutputChannels = cmdLineOptions.getOption<std::size_t>( "output-channels" );
    std::string const routingsString = cmdLineOptions.getDefaultedOption<std::string>( "routings", "[]" );
    rbbl::FilterRoutingList const routings( rbbl::FilterRoutingList::fromJson( routingsString ) );
    const std::size_t maxFilterRoutings = cmdLineOptions.getDefaultedOption<std::size_t>( "max-routings", routings.size() );
    if( routings.size() > maxFilterRoutings )
    {
      throw std::invalid_argument( "The number of initial filter routings exceeds the value specified in \"--maxRoutings\"." );
    }

    // Initialise the impulse response matrix
    // Use max() as special value to denote "no maximum length specified"
    std::size_t const maxFilterLengthOption = cmdLineOptions.getDefaultedOption<std::size_t>( "max-filter-length", std::numeric_limits<std::size_t>::max() );
    std::size_t const maxFilterOption = cmdLineOptions.getDefaultedOption<std::size_t>( "max-filters", std::numeric_limits<std::size_t>::max( ) ); // max() denotes
    std::string const filterList = cmdLineOptions.getDefaultedOption<std::string>( "filters", std::string() );
    std::string const indexOffsetString = cmdLineOptions.getDefaultedOption<std::string>( "filter-file-index-offsets", std::string( ) );
    rbbl::IndexSequence const indexOffsets( indexOffsetString );
    efl::BasicMatrix<SampleType> initialFilters( cVectorAlignmentSamples );
    initFilterMatrix( filterList, maxFilterLengthOption, maxFilterOption, indexOffsets, initialFilters );

    // The final values for the number and length of filter slots are determined by the logic of the initialisation function.
    std::size_t const maxFilters = initialFilters.numberOfRows();

    std::size_t const periodSize = cmdLineOptions.getDefaultedOption<std::size_t>( "period", 1024 );
    SamplingFrequencyType const samplingFrequency = cmdLineOptions.getDefaultedOption<SamplingFrequencyType>( "sampling-frequency", 48000 );

    std::string const fftLibrary = cmdLineOptions.getDefaultedOption<std::string>( "fft-library", "default" );

    bool const optDsp = cmdLineOptions.getDefaultedOption<bool>("dsp-optimisation", false );
    if( optDsp )
    {
      if( not efl::initialiseLibrary("") )
      {
	throw std::runtime_error( "Error initialising DSP function library." );
      }
    }

    SignalFlowContext const context{ periodSize, samplingFrequency };

    rcl::FirFilterMatrix convolver( context, "MatrixConvolver", nullptr/*instantiate as top-level flow*/, numberOfInputChannels, numberOfOutputChannels,
                                    initialFilters.numberOfColumns(), maxFilters, maxFilterRoutings,
                                    initialFilters, routings,
                                    rcl::FirFilterMatrix::ControlPortConfig::None /*no control inputs*/,
                                    fftLibrary.c_str() );

    rrl::AudioSignalFlow flow( convolver );

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

    audiointerfaces::AudioInterface::Configuration baseConfig(numberOfInputChannels,numberOfOutputChannels,samplingFrequency,periodSize);

      
    std::unique_ptr<visr::audiointerfaces::AudioInterface> audioInterface( audiointerfaces::AudioInterfaceFactory::create( audioBackend, baseConfig, specConf) );
      
    audioInterface->registerCallback( &rrl::AudioSignalFlow::processFunction, &flow );

    // should there be a separate start() method for the audio interface?
    audioInterface->start( );

    // Rendering runs until q<Return> is entered on the console.
    std::cout << "VISR matrix convolver running. Press \"q<Return>\" or Ctrl-C to quit." << std::endl;
    char c;
    do
    {
      c = std::getc( stdin );
    }
    while( c != 'q' );

    audioInterface->stop( );

    // Should there be an explicit stop() method for the sound interface?
    audioInterface->unregisterCallback( &rrl::AudioSignalFlow::processFunction );

    efl::DenormalisedNumbers::resetDenormHandling( oldDenormNumbersState );
  }
  catch( std::exception const & ex )
  {
    std::cout << "Exception caught on top level: " << ex.what() << std::endl;
    efl::DenormalisedNumbers::resetDenormHandling( oldDenormNumbersState );
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
