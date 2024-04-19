/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpythoncomponents/wrapper.hpp>

#include <libvisr/constants.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <librrl/audio_signal_flow.hpp>

#include <librrl/integrity_checking.hpp>

#include <libefl/basic_matrix.hpp>

#include <libpml/initialise_parameter_library.hpp>

#include <libpythonsupport/initialisation_guard.hpp>


#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

namespace visr
{
namespace pythoncomponents
{
namespace test
{

BOOST_AUTO_TEST_CASE( MultiplePmlInitialisation )
{
  visr::pml::initialiseParameterLibrary();

  std::string moduleName = "initpml";

  boost::filesystem::path basePath{ CMAKE_CURRENT_SOURCE_DIR };
  boost::filesystem::path const modulePath = basePath / "python";

  {
    pythonsupport::InitialisationGuard::initialise();

    const std::size_t blockSize{ 64 };
    const SamplingFrequencyType samplingFrequency{ 48000 };
    std::size_t const numBlocks{ 16 };

    SignalFlowContext ctxt( blockSize, samplingFrequency );

    // Instantiate the atomic component (implemented in Python)
    // by a mixture or positional and keyword constructor arguments
    Wrapper pyAtom1( ctxt, "PythonAtom", nullptr, moduleName.c_str(),
                     "PmlAdder", "3,", "{'width':5}",
                     modulePath.string().c_str() );

    std::stringstream errMsg;
    bool res = rrl::checkConnectionIntegrity( pyAtom1, true, errMsg );
    BOOST_CHECK_MESSAGE( res, errMsg.str() );

    rrl::AudioSignalFlow flow( pyAtom1 );

    std::size_t const numInputChannels = 15;
    std::size_t const numOutputChannels = 5;

    std::vector< SampleType * > inputPtr( numInputChannels, nullptr );
    std::vector< SampleType * > outputPtr( numOutputChannels, nullptr );

    efl::BasicMatrix< SampleType > inputData( numInputChannels,
                                              blockSize * numBlocks );
    // TODO: Fill the input data with something useful

    efl::BasicMatrix< SampleType > outputData( numOutputChannels,
                                               blockSize * numBlocks );

    for( std::size_t blockIdx( 0 ); blockIdx < numBlocks; ++blockIdx )
    {
      for( std::size_t idx( 0 ); idx < numInputChannels; ++idx )
      {
        inputPtr[ idx ] = inputData.row( idx ) + blockIdx * blockSize;
      }
      for( std::size_t idx( 0 ); idx < numOutputChannels; ++idx )
      {
        outputPtr[ idx ] = outputData.row( idx ) + blockIdx * blockSize;
      }
      flow.process( &inputPtr[ 0 ], &outputPtr[ 0 ] );
    }
  } // There's no way to trigger the de-initialisation of the Python subsystem.
    // This happens when the application (here: unit test binary) is finished.
}

} // namespace test
} // namespace pythoncomponents
} // namespace visr
