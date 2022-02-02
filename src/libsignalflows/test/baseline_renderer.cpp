/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libsignalflows/baseline_renderer.hpp>

#include <libefl/basic_matrix.hpp>

#include <libpanning/LoudspeakerArray.h>

#include <libvisr/signal_flow_context.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ciso646>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

namespace visr
{
namespace objectmodel
{
namespace test
{


BOOST_AUTO_TEST_CASE( InstantiateRenderer )
{
  boost::filesystem::path const arrayConfigFile( CMAKE_SOURCE_DIR "/config/generic/bs2051-9+10+3.xml" );
  BOOST_CHECK( exists( arrayConfigFile ) and not is_directory( arrayConfigFile ) );

  panning::LoudspeakerArray arrayConfig;
  arrayConfig.loadXmlFile( arrayConfigFile.string() );

  std::size_t const numberOfInputs = 16;
  std::size_t const numberOfOutputs = 40;
  std::size_t const period = 128;
  std::size_t const interpolationPeriod = 16 * period;
  std::size_t const numRealLoudspeakers = arrayConfig.getNumRegularSpeakers();
  std::size_t const diffusionFilterLength = 512;
  efl::BasicMatrix<SampleType> const diffusionFilters( numRealLoudspeakers, diffusionFilterLength );

  std::string const trackingConfig( "" );

  // Construct the reverb configuration
  std::size_t const numReverbObjects = 5;
  std::size_t const discreteReflectionsPerObject = 4;
  double const lateFilterLengthSeconds = 0.05;
#if VISR_PML_USE_SNDFILE_LIBRARY
  std::string const lateDiffusionFilters( CMAKE_SOURCE_DIR "/config/filters/random_phase_allpass_64ch_512taps.wav" );
#endif
  double const maximumDiscreteReflectionDelay = 0.23456f;

  std::size_t const numInputEqSections = 0;

  std::stringstream reverbConfig;
  reverbConfig << "{ \"numReverbObjects\": " << numReverbObjects
               << ", \"discreteReflectionsPerObject\": " << discreteReflectionsPerObject
               << ", \"lateReverbFilterLength\": " << lateFilterLengthSeconds
               << ", \"maxDiscreteReflectionDelay\": " << maximumDiscreteReflectionDelay
// If no sndfile support is available, fall back to the default-generated decorrelation filters.
#if VISR_PML_USE_SNDFILE_LIBRARY
               << ", \"lateReverbDecorrelationFilters\": \"" << lateDiffusionFilters << "\" "
#endif
               << "}";

  SignalFlowContext context( period, 48000 );

  signalflows::BaselineRenderer( context, "", nullptr,
                                 arrayConfig,
                                 numberOfInputs,
                                 numberOfOutputs,
                                 interpolationPeriod,
                                 diffusionFilters,
                                 trackingConfig,
                                 8888,
                                 numInputEqSections,
                                 reverbConfig.str(),
                                 false // No frequency-dependent panning.
                                 );
}

} // namespace test
} // namespace objectmodel
} // namespce visr
