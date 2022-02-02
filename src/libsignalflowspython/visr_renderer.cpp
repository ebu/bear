/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "visr_renderer.hpp"

#include <librcl/scene_decoder.hpp>

#if VISR_PYTHON_SUPPORT
#include <libpythoncomponents/wrapper.hpp>
#endif

#include <boost/format.hpp>

namespace visr
{
namespace signalflowspython
{

VisrRenderer::VisrRenderer( SignalFlowContext const & context,
                            char const * name,
                            CompositeComponent * parent,
                            panning::LoudspeakerArray const & loudspeakerConfiguration,
                            std::size_t numberOfInputs,
                            std::size_t numberOfOutputs,
                            std::size_t interpolationPeriod,
                            efl::BasicMatrix<SampleType> const & diffusionFilters,
                            std::string const & trackingConfiguration,
                            std::size_t sceneReceiverPort,
                            std::size_t numberOfObjectEqSections,
                            std::string const & reverbConfig,
                            bool frequencyDependentPanning,
                            std::string const & metadapterConfig
                            )
 : CompositeComponent( context, name, parent )
 , mSceneReceiver( context, "SceneReceiver", this, sceneReceiverPort, rcl::UdpReceiver::Mode::Asynchronous )
 , mSceneDecoder( nullptr )
 , mCoreRenderer( context, "CoreRenderer", this, loudspeakerConfiguration, numberOfInputs, numberOfOutputs,
                  interpolationPeriod, diffusionFilters, trackingConfiguration, numberOfObjectEqSections,
                  reverbConfig, frequencyDependentPanning )
 , mInput( "input", *this, numberOfInputs )
 , mOutput( "output", *this, numberOfOutputs )

{
  if( metadapterConfig.empty() )
  {
    // std::make_unique (C++14) would be handy.
    mSceneDecoder.reset( new rcl::SceneDecoder( context, "SceneDecoder", this ) );
    parameterConnection( mSceneReceiver.parameterPort( "messageOutput" ), mSceneDecoder->parameterPort( "datagramInput" ) );
    parameterConnection( mSceneDecoder->parameterPort( "objectVectorOutput" ), mCoreRenderer.parameterPort( "objectDataInput" ) );
  }
  else
  {
#if VISR_PYTHON_SUPPORT
    std::string const formatString = "{'processorConfig': '%s', 'objectVectorInput': False, 'objectVectorOutput': True, 'oscControlPort': False, 'jsonControlPort': False }";
    std::string const kwArgs = str( boost::format( formatString ) % metadapterConfig );

    mSceneDecoder.reset( new pythoncomponents::Wrapper( context, "Metadapter", this,
      "metadapter.visrintegration", //  char const * moduleName,
      "Component",                  //  char const * componentClassName,
      "",                           //  char const * positionalArguments = "",
      kwArgs.c_str(),               //  char const * keywordArguments = "",
      ""                            //  No module search path
      ) );
    parameterConnection( mSceneReceiver.parameterPort( "messageOutput" ), mSceneDecoder->parameterPort( "objectIn" ) );
    parameterConnection( mSceneDecoder->parameterPort( "objectOut" ), mCoreRenderer.parameterPort( "objectDataInput" ) );
#else
    throw std::invalid_argument( "Providing a metadapter configuration requires a VISR built with Python support." );
#endif
  }

  audioConnection( mInput, mCoreRenderer.audioPort( "audioIn") );
  audioConnection( mCoreRenderer.audioPort( "audioOut"), mOutput );

  if( not trackingConfiguration.empty() )
  {
    mTrackingReceiver.reset( new rcl::UdpReceiver( context, "TrackingReceiver", this, 8888, rcl::UdpReceiver::Mode::Synchronous) );
    mTrackingPositionDecoder.reset( new rcl::PositionDecoder( context, "TrackingPositionDecoder", this ) );

    parameterConnection( mTrackingPositionDecoder->parameterPort("positionOutput"), mCoreRenderer.parameterPort("trackingPositionInput") );
  }
}

VisrRenderer::~VisrRenderer( )
{
}

} // namespace signalflowspython
} // namespace visr
