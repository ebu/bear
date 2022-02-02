/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "audio_signal_flow.hpp"

#include "audio_connection_map.hpp"
#include "communication_area.hpp"
#include "integrity_checking.hpp"
#include "parameter_connection_graph.hpp"
#include "parameter_connection_map.hpp"
#include "port_utilities.hpp"
#include "scheduling_graph.hpp"

#include "signal_routing_internal.hpp"

#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
#include "runtime_profiler.hpp"
#endif

#include <libefl/alignment.hpp>
#include <libefl/vector_functions.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/audio_sample_type.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <libvisr/communication_protocol_base.hpp>
#include <libvisr/communication_protocol_factory.hpp>
#include <libvisr/communication_protocol_type.hpp>
#include <libvisr/detail/compose_message_string.hpp>

#include <libvisr/impl/audio_connection_descriptor.hpp>
#include <libvisr/impl/audio_port_base_implementation.hpp>
#include <libvisr/impl/component_implementation.hpp>
#include <libvisr/impl/composite_component_implementation.hpp>
#include <libvisr/impl/parameter_port_base_implementation.hpp>
#include <libvisr/impl/time_implementation.hpp>

#include <algorithm>
#include <cassert>
#include <ciso646>
#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <sstream>
#include <utility> // for std::pair and std::make_pair

#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
#include <chrono>
#endif

// Preliminary solution agains potential additional audio dependencies if new infrastructure elements (such as input routing blocks)
// are introduced.
// Needs to be tested more thoroughly.
#define WORKAROUND_CYCLIC_DEPENDENCIES 1

namespace visr
{
namespace rrl
{

AudioSignalFlow::AudioSignalFlow( Component & flow )
 : mFlow( flow.implementation() )
 , mParameterExchangeMutex( new ParameterExchangeMutexType{} )
{
  std::stringstream checkMessages;
  bool const checkResult = checkConnectionIntegrity( mFlow, true/* hierarchical*/, checkMessages );
  if( not checkResult )
  {
    throw std::runtime_error( detail::composeMessageString( "Signal flow graph is inconsistent: ", checkMessages.str() ) );
  }
  checkMessages.clear();

  // TODO: homogenise interfaces between parameter and audio connection maps.
  AudioConnectionMap const originalAudioConnections( mFlow, true );
  AudioConnectionMap const flatAudioConnections = originalAudioConnections.resolvePlaceholders();

  ParameterConnectionMap originalParameterConnections;
  if( not fillRecursive( originalParameterConnections, mFlow, checkMessages ) )
  {
    throw std::runtime_error(checkMessages.str() );
  }
  ParameterConnectionMap const flatParameterConnections = resolvePlaceholders( originalParameterConnections );

  AudioConnectionMap adjustedAudioConnections;
  bool const initAudioResult = initialiseAudioConnections( checkMessages, flatAudioConnections,
                                                           adjustedAudioConnections);
  if( not initAudioResult )
  {
    throw std::runtime_error( detail::composeMessageString( "AudioSignalFlow: Audio connections could not be initialised.",
                                                            checkMessages.str() ) );
  }

  ParameterConnectionMap adjustedParameterConnections;
  bool const initParamResult = initialiseParameterInfrastructure( checkMessages, flatParameterConnections,
                                                                  adjustedParameterConnections);
  if( not initParamResult )
  {
    throw std::runtime_error( detail::composeMessageString( "AudioSignalFlow: Parameter infrastructure could not be initialised.",
                                                             checkMessages.str() ) );
  }



  // TODO: Use the full list of components 
  bool const initScheduleResult = initialiseSchedule( checkMessages, adjustedAudioConnections, adjustedParameterConnections );
  if( not initScheduleResult )
  {
    throw std::runtime_error( detail::composeMessageString( "AudioSignalFlow: Execution schedule could not be created.",
                                                             checkMessages.str()) );
  }

  visr::impl::TimeImplementation & timeImpl = mFlow.timeImplementation();
  timeImpl.resetCounter();
}

AudioSignalFlow::~AudioSignalFlow()
{
}

std::size_t AudioSignalFlow::period() const
{
  return mFlow.period();
}

/*static*/ void 
AudioSignalFlow::processFunction( void* userData,
                                  SampleType const * const * captureSamples,
                                  SampleType * const * playbackSamples,
                                  bool& status )
{
  AudioSignalFlow* flowObj = reinterpret_cast<AudioSignalFlow*>( userData );
  status = flowObj->process( captureSamples, playbackSamples );
}


bool
AudioSignalFlow::process( SampleType const * const * captureSamples,
                          SampleType * const * playbackSamples,
                          std::size_t captureStrideSamples /*= 1*/,
                          std::size_t playbackStrideSamples /*= 1*/ )
{
  // This assumes that all capture ports have the default sample type "SampleType"
  for( std::size_t chIdx( 0 ); chIdx < numberOfCaptureChannels(); ++chIdx )
  {
    SampleType * chPtr = reinterpret_cast<SampleType*>(mCaptureChannels[chIdx]);
    efl::ErrorCode const res = (captureStrideSamples == 1)
     ? efl::vectorCopy( captureSamples[chIdx], chPtr, mFlow.period(), 0 )
     : efl::vectorCopyStrided( captureSamples[chIdx], chPtr, captureStrideSamples,
                              1 /*destination stride*/, mFlow.period(), 0);
    if( res != efl::noError )
    {
      throw std::runtime_error( "AudioSignalFlow: Error while copying input samples samples." );
    }
  }
  try
  {
    executeComponents();
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( detail::composeMessageString("Error during execution of processing schedule: ", ex.what()) );
  }
  // This assumes that all capture ports have the default sample type "SampleType"
  for( std::size_t chIdx( 0 ); chIdx < numberOfPlaybackChannels(); ++chIdx )
  {
    SampleType const * chPtr = reinterpret_cast<SampleType const*>(mPlaybackChannels[chIdx]);
    efl::ErrorCode const res = ( playbackStrideSamples == 1 )
     ? efl::vectorCopy( chPtr, playbackSamples[chIdx], mFlow.period(), 0 )
     : efl::vectorCopyStrided( chPtr, playbackSamples[chIdx], 1 /*source stride*/,
                               playbackStrideSamples, mFlow.period(), 0);
    if( res != efl::noError )
    {
      throw std::runtime_error( "AudioSignalFlow: Error while copying output samples samples." );
    }
  }
  // TODO: use a sophisticated enumeration to signal error conditions
  return true; // Means 'no error'
}

void AudioSignalFlow::process( SampleType const * captureSamples,
                               std::size_t captureChannelStride,
                               std::size_t captureSampleStride,
                               SampleType * playbackSamples,
                               std::size_t playbackChannelStride,
                               std::size_t playbackSampleStride )
{
  for( std::size_t chIdx( 0 ); chIdx < numberOfCaptureChannels(); ++chIdx )
  {
    SampleType const * src = captureSamples + chIdx * captureChannelStride;
    SampleType * dest = reinterpret_cast<SampleType*>(mCaptureChannels[chIdx]);

    efl::ErrorCode const res = efl::vectorCopyStrided( src, dest, captureSampleStride, 1, mFlow.period(), 0 );
    if( res != efl::noError )
    {
      throw std::runtime_error( "AudioSignalFlow: Error while copying input samples samples." );
    }
  }
  try
  {
    executeComponents();
  }
  catch( std::exception const & ex )
  {
    throw std::runtime_error( detail::composeMessageString( "Error during execution of processing schedule: ", ex.what() ) );
  }
  // This assumes that all capture ports have the default sample type "SampleType"
  for( std::size_t chIdx( 0 ); chIdx < numberOfPlaybackChannels(); ++chIdx )
  {
    SampleType const * src = reinterpret_cast<SampleType const *>(mPlaybackChannels[chIdx]);
    SampleType * dest = playbackSamples + chIdx * playbackChannelStride;
    efl::ErrorCode const res = efl::vectorCopyStrided( src, dest, 1, playbackSampleStride, mFlow.period(), 0 );
    if( res != efl::noError )
    {
      throw std::runtime_error( "AudioSignalFlow: Error while copying output samples samples." );
    }
  }
}

void AudioSignalFlow::executeComponents()
{
  std::lock_guard<ParameterExchangeMutexType>
    guard( parameterExchangeMutex() );
  try
  {
#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
    if( mRuntimeProfiler )
    {
      RuntimeProfiler::MeasurementVector & timing
       = mRuntimeProfiler->currentData();
      std::size_t compIdx{ 0 };
      for( AtomicComponent * pc : mProcessingSchedule )
      {
        auto const startTime = std::chrono::high_resolution_clock::now();
        pc->process();
        auto const endTime = std::chrono::high_resolution_clock::now();
        RuntimeProfiler::TimeType const elapsed
          = std::chrono::duration<RuntimeProfiler::TimeType>( endTime - startTime ).count();
        timing[ compIdx ] = elapsed;
        ++compIdx;
      }
      mRuntimeProfiler->finishIteration();
    }
    else
#endif
    {
      for( AtomicComponent * pc : mProcessingSchedule )
      {
        pc->process();
      }
    }
    visr::impl::TimeImplementation & timeImpl = mFlow.timeImplementation();
    timeImpl.advanceBlockCounter();
  }
  catch( std::exception const & ex )
  {
    // TODO: Retrieve information about current object and add it to the exception message
    // TODO: Consider adding a name() function to ProcessableInterface
    throw std::runtime_error( std::string("Exception while executing audio signal flow: ") + ex.what() );
  }
}

std::size_t AudioSignalFlow::numberOfAudioCapturePorts() const
{
  return mTopLevelAudioInputs.size();
}

std::size_t AudioSignalFlow::numberOfAudioPlaybackPorts() const
{
  return mTopLevelAudioOutputs.size( );
}

char const * AudioSignalFlow::audioCapturePortName( std::size_t idx ) const
{
  if( idx >= numberOfAudioCapturePorts() )
  {
    throw std::out_of_range("AudioSignalFlow::audioCapturePortName(): index exceeds number of ports" );
  }
  return mTopLevelAudioInputs.at( idx )->name();
}

char const * AudioSignalFlow::audioPlaybackPortName( std::size_t idx ) const
{
  if( idx >= numberOfAudioPlaybackPorts( ) )
  {
    throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortName(): index exceeds number of ports" );
  }
  return mTopLevelAudioOutputs.at( idx )->name( );
}

std::size_t AudioSignalFlow::audioCapturePortWidth( std::size_t idx ) const
{
  if( idx >= numberOfAudioCapturePorts() )
  {
    throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortWidth(): index exceeds number of ports" );
  }
  return mTopLevelAudioInputs.at( idx )->width( );
}

std::size_t AudioSignalFlow::audioPlaybackPortWidth( std::size_t idx ) const
{
  if( idx >= numberOfAudioPlaybackPorts() )
  {
    throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortWidth(): index exceeds number of ports" );
  }
  return mTopLevelAudioOutputs.at( idx )->width( );
}

std::size_t AudioSignalFlow::audioCapturePortOffset( std::size_t idx ) const
{
  if( idx >= numberOfAudioCapturePorts() )
  {
    throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortWidth(): index exceeds number of ports" );
  }
  return std::accumulate( mTopLevelAudioInputs.begin(), mTopLevelAudioInputs.begin()+idx, static_cast<std::size_t>(0),
      []( std::size_t acc, impl::AudioPortBaseImplementation const * port )
      { return acc + port->width(); } );
}

std::size_t AudioSignalFlow::audioPlaybackPortOffset( std::size_t idx ) const
{
  if( idx >= numberOfAudioPlaybackPorts() )
  {
    throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortWidth(): index exceeds number of ports" );
  }
  return std::accumulate( mTopLevelAudioOutputs.begin(), mTopLevelAudioOutputs.begin()+idx, static_cast<std::size_t>(0),
      []( std::size_t acc, impl::AudioPortBaseImplementation const * port )
      { return acc + port->width(); } );
}

std::size_t AudioSignalFlow::audioCapturePortIndex( char const * name ) const
{
  for( std::size_t idx{ 0 }; idx < mTopLevelAudioInputs.size(); ++idx )
  {
    if( std::strcmp( mTopLevelAudioInputs.at( idx )->name(), name ) == 0 )
    {
      return idx;
    }
  }
  throw std::out_of_range( "AudioSignalFlow::audioCapturePortWidth(): index exceeds number of ports" );
}

std::size_t AudioSignalFlow::audioPlaybackPortIndex( char const * name ) const
{
  for( std::size_t idx{ 0 }; idx < mTopLevelAudioOutputs.size(); ++idx )
  {
    if( std::strcmp( mTopLevelAudioOutputs.at( idx )->name(), name ) == 0 )
    {
      return idx;
    }
  }
  throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortWidth(): index exceeds number of ports" );
}

visr::AudioSampleType::Id AudioSignalFlow::
audioCapturePortSampleType( std::size_t idx ) const
{
  if( idx >= numberOfAudioCapturePorts() )
  {
    throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortWidth(): index exceeds number of ports" );
  }
  return mTopLevelAudioInputs.at( idx )->sampleType();
}

visr::AudioSampleType::Id AudioSignalFlow::
audioPlaybackPortSampleType( std::size_t idx ) const
{
  if( idx >= numberOfAudioPlaybackPorts() )
  {
    throw std::out_of_range( "AudioSignalFlow::audioPlaybackPortWidth(): index exceeds number of ports" );
  }
  return mTopLevelAudioOutputs.at( idx )->sampleType();
}

std::size_t AudioSignalFlow::numberOfCaptureChannels() const
{
  return mCaptureChannels.size();
}

std::size_t AudioSignalFlow::numberOfPlaybackChannels() const
{
  return mPlaybackChannels.size();
}

bool AudioSignalFlow::initialiseParameterInfrastructure( std::ostream & messages, ParameterConnectionMap const & originalConnections, ParameterConnectionMap & finalConnections )
{
  bool result = true;
  mCommunicationProtocols.clear();
  if( not mFlow.isComposite() )
  {
    // TODO: Maybe move to a specific method for atomic flows
    for( auto paramPort : mFlow.parameterPorts() )
    {
      visr::ParameterType const paramType = paramPort->parameterType();
      CommunicationProtocolType const protocol = paramPort->protocolType();
      ParameterConfigBase const & paramConfig = paramPort->parameterConfig();
      std::unique_ptr<CommunicationProtocolBase> protocolInstance
        = CommunicationProtocolFactory::createProtocol( protocol, paramType, paramConfig );
      if( not protocolInstance )
      {
        messages << "AudioSignalFlow: Could not instantiate communication protocol for parameter connection.\n";
        result = false;
      }
      if( paramPort->direction() == PortBase::Direction::Input )
      {
        std::unique_ptr<CommunicationProtocolBase::Output> protocolOutput = CommunicationProtocolFactory::createOutput( protocol );
        // TODO: Do we need to care for failing dynamic_casts (this would be a logical error).
        ParameterInputBase & paramInput = dynamic_cast<ParameterInputBase&>(paramPort->containingPort());
        protocolInstance->connectInput( &(paramInput.protocolInput()) );
        protocolInstance->connectOutput( protocolOutput.get() );
        ProtocolReceiveEndpoints::value_type insertPair{ std::string( paramPort->name() ), std::move( protocolOutput ) };
        mProtocolReceiveEndpoints.insert( std::move( insertPair ) );
      }
      else
      {
        std::unique_ptr<CommunicationProtocolBase::Input> protocolInput = CommunicationProtocolFactory::createInput( protocol );
        ParameterOutputBase & paramOutput = dynamic_cast<ParameterOutputBase&>(paramPort->containingPort());
        protocolInstance->connectOutput( &(paramOutput.protocolOutput()) );
        protocolInstance->connectInput( protocolInput.get() );
        ProtocolSendEndpoints::value_type insertPair{ std::string( paramPort->name() ), std::move( protocolInput ) };
        mProtocolSendEndpoints.insert( std::move( insertPair ) );
      }
      mCommunicationProtocols.push_back( std::move( protocolInstance ) );
    }
    return result;
  }
  // Composite component case starts here.
  ParameterConnectionGraph const connectionGraph( originalConnections );
  for( ParameterConnectionGraph::ConnectedPorts const & connection : connectionGraph.connectedPorts() )
  {
    // First check the connection topology.
    std::size_t const numSenders = connection.numSenders();
    std::size_t const numReceivers = connection.numReceivers();
    // This should not happen if the connections are created with registerParameterConnection()
    // TODO: improve error reporting (i.e., provide as much as possible information about the connection)
    if( numSenders < 1 )
    {
      messages << "AudioSignalFlow: A parameter connection must have at least one sending port.\n";
      result = false;
    }
    if( numReceivers < 1 )
    {
      messages << "AudioSignalFlow: A parameter connection must have at least one receiving port.\n";
      result = false;
    }

    // Because we need a reference port, we cannot continue with this connection if there's no sender.
    // Normally we try to perform the operations as much as possible to obtain reasonable error messages.
    if( numSenders < 1 )
    {
      continue;
    }

    // Use one port as reference, all other ports in this connection must have compatible porotocols and parameters.
    impl::ParameterPortBaseImplementation const * refPort = connection.sendPorts()[0];
    // We checked before that connection.sendPorts() is not empty.
    // Don't check against the first port which is the reference.
    for( auto portIt( ++connection.sendPorts().begin() ); portIt < connection.sendPorts().end(); ++portIt )
    {
      bool const res = checkParameterPortCompatibility( *refPort, **portIt, messages );
      if( not res )
      {
        result = false;
        continue;
      }
    }
    for( auto portIt( connection.receivePorts().begin() ); portIt < connection.receivePorts().end(); ++portIt )
    {
      bool const res = checkParameterPortCompatibility( *refPort, **portIt, messages );
      if( not res )
      {
        result = false;
        continue;
      }
    }
    try
    {
      visr::ParameterType const paramType = refPort->parameterType();
      CommunicationProtocolType const protocol = refPort->protocolType();
      ParameterConfigBase const & paramConfig = refPort->parameterConfig();
      std::unique_ptr<CommunicationProtocolBase> protocolInstance
        = CommunicationProtocolFactory::createProtocol( protocol, paramType, paramConfig );
      if( not protocolInstance )
      {
        messages << "AudioSignalFlow: Could not instantiate communication protocol for parameter connection.\n";
        result = false;
      }
      // Connect to all ports of the connection.
      // This will also fail if the connection does not match the arity (one-to-one, one-to-many ...) of the protocol
      for( auto port : connection.sendPorts() )
      {
        if( port->parent().isTopLevel() ) // Whether this is 'symbolic' input on the top level.
        {
          // This is a toplevel parameter input. We need to create a communication protocol output port for that.
          // TODO: Improve exception safety!
          std::unique_ptr<CommunicationProtocolBase::Output> protocolOutput = CommunicationProtocolFactory::createOutput( protocol );
          protocolInstance->connectOutput( protocolOutput.get() );
          ProtocolReceiveEndpoints::value_type insertPair{ std::string(port->name()), std::move(protocolOutput) };
          mProtocolReceiveEndpoints.insert( std::move(insertPair) );
        }
        else
        {
          try
          {
            ParameterOutputBase & paramOutput = dynamic_cast<ParameterOutputBase&>(port->containingPort());
            protocolInstance->connectOutput( &(paramOutput.protocolOutput()) );
          }
          catch( std::exception const & ex )
          {
            result = false;
            messages << "Error connection protocol output to output port \"" << fullyQualifiedName( *port )
                     << "\": " << ex.what() << ".\n";
          }
        }
      }
      for( auto port: connection.receivePorts() )
      {
        if( port->parent().isTopLevel() ) // Whether this is 'symbolic' output on the top level.
        {
          // This is a toplevel parameter output. We need to create a communication protocol input port for that.
          // TODO: Improve exception safety!
          auto protocolIt = mProtocolSendEndpoints.insert( std::make_pair( port->name(), CommunicationProtocolFactory::createInput( protocol ) ) );
          protocolInstance->connectInput( protocolIt.first->second.get() );
        }
        else
        {
          try
          {
            ParameterInputBase & paramInput = dynamic_cast<ParameterInputBase&>(port->containingPort());
            protocolInstance->connectInput( &(paramInput.protocolInput()) );
          }
          catch( std::exception const & ex )
          {
            result = false;
            messages << "Error connection protocol output to output port \"" << fullyQualifiedName( *port )
                     << "\": " << ex.what() << ".\n";
          }
        }
      }
      mCommunicationProtocols.push_back( std::move( protocolInstance ) );
    }
    catch( const std::exception& ex )
    {
      messages << "AudioSignalFlow: Error while initialising the communication parameter infrastructure: "
               << ex.what() << ".\n";
      result = false;
    }
  }

  // At the moment, there are no infrastructure modifications.
  // Therefore the connection map is passed back unaltered.
  finalConnections = originalConnections;
  return result;
}

CommunicationProtocolBase::Output & AudioSignalFlow::externalParameterReceivePort( char const * portName )
{
  auto const findIt = mProtocolReceiveEndpoints.find( std::string(portName) );
  if( findIt == mProtocolReceiveEndpoints.end() )
  {
    throw std::out_of_range( detail::composeMessageString("External receive port named \"", portName , "\" does not exist." ) );
  }
  return *(findIt->second);
}

CommunicationProtocolBase::Input & AudioSignalFlow::externalParameterSendPort( char const * portName )
{
  auto const findIt = mProtocolSendEndpoints.find( std::string( portName ) );
  if( findIt == mProtocolSendEndpoints.end() )
  {
    throw std::out_of_range(
      detail::composeMessageString("External send port named \"", portName , "\" does not exist." ) );
  }
  return *(findIt->second);
}

bool AudioSignalFlow::initialiseSchedule( std::ostream & messages,
                                          AudioConnectionMap const & audioConnections,
                                          ParameterConnectionMap const & parameterConnections )
{
  bool result = true;
  if( not mFlow.isComposite() )
  {
    mProcessingSchedule.clear();
    AtomicComponent* atom = dynamic_cast<AtomicComponent *>( &(mFlow.component()) );
    if( not atom )
    {
      throw std::logic_error( "AudioSignalFlow: Internal error: Casting of non-composite component to AtomicComponent failed.");
    }
    mProcessingSchedule.push_back( atom );
  }
  else
  {
    SchedulingGraph depGraph;
    // TODO: check result!
    depGraph.initialise( mFlow, audioConnections, parameterConnections );
    auto const schedule = depGraph.sequentialSchedule();
    mProcessingSchedule.assign( schedule.begin(), schedule.end() );
  }
  return result;
}


std::size_t AudioSignalFlow::numberCommunicationProtocols() const
{
  return mCommunicationProtocols.size();
}

namespace // unnamed
{

// Data structure holding an ordered list of 
using ConnectionList = std::vector<std::pair<AudioChannel, AudioChannel > >;

using ChannelVector = std::vector<AudioChannel>;

using PortOffsetLookup = std::map<impl::AudioPortBaseImplementation *, std::size_t>;

/**
 * Check whether a connection list is contiguous, i.e., refers to a contiguous index range of send channels according to the offset table
 * \p offsetTable
 * @pre The elements of \p connections are ordered according to their receive channel index.
 * @throw std::out_of_range if a send port referenced in \channels is not contained in \p offsetTable
 */
bool contiguousRange( ChannelVector const & channels, PortOffsetLookup const & offsetTable )
{
  // at() throws if the indexed element is not contained.
  auto const findIt =  std::adjacent_find( channels.begin(), channels.end(),
    [&offsetTable](AudioChannel const & lhs,AudioChannel const & rhs)
    {
      return offsetTable.at(lhs.port()) + lhs.channel() + 1 != offsetTable.at(rhs.port()) + rhs.channel();
    } );
  return findIt == channels.end();
//  return findContiguityGap(channels.begin(), channels.end()) == channels.end();
}

// Not used at the moment.
// Reimplement if we want to adjust the order of send ports maximise the number of contiguous receive ranges (or minimise the the number of
// copy operations.
#if 0
/**
 * Internal function used to detect the first contiguity gap of the send ports
 */

 /**
 * Data structure to hold an adjacency wish.
 * Format:
 * get<0>: The first send port of the requested adjacency pair
 * get<1>: The second send port of the requested adjacency pair.
 * get<2>: The receive port requesting the adjacent pair of send ports.
 */
using AdjacencyWish = std::tuple<impl::AudioPortBaseImplementation const *, impl::AudioPortBaseImplementation const *, impl::AudioPortBaseImplementation const * >;

using AdjacencyWishList = std::multiset<AdjacencyWish>;

ChannelVector::const_iterator findContiguityGap( ChannelVector::const_iterator beginIt, ChannelVector::const_iterator endIt)
{
  return std::adjacent_find( beginIt, endIt,
   []( AudioChannel const & lhs, AudioChannel const & rhs )
  { return (lhs.port() != rhs.port()) or (rhs.channel() != lhs.channel() + 1);} );
}

std::tuple<bool, AdjacencyWishList> possiblyContiguousRange( ChannelVector const & channels, impl::AudioPortBaseImplementation * receivePort )
{
  AdjacencyWishList wishList;
  for( ChannelVector::const_iterator runIt = channels.begin();; )
  {
    ChannelVector::const_iterator gapIt = findContiguityGap( runIt, channels.end() );
    if( gapIt == channels.end() )
    {
      return std::make_tuple( true, wishList );
    }
    ChannelVector::const_iterator nextIt = gapIt + 1;
    if( nextIt == channels.end() ) // Shouldn't be possible due to the workings of adjacent_find.
    {
      return std::make_tuple( true, wishList );
    }
    if( (runIt->channel() != runIt->port()->width()-1) or nextIt->channel() != 0 )
    {
      return std::make_tuple( false, AdjacencyWishList{} );
    }
    wishList.insert( std::make_tuple( runIt->port(), nextIt->port(), receivePort ) );
    runIt = nextIt;
  }
}
#endif

ChannelVector sendChannels( impl::AudioPortBaseImplementation const * receivePort, AudioConnectionMap const & flatConnections )
{
  ConnectionList receivingConnections;
  std::copy_if( flatConnections.begin(), flatConnections.end(), std::back_inserter( receivingConnections ),
      [receivePort]( AudioConnectionMap::value_type const & conn ) { return conn.second.port() == receivePort; } );
  if( receivingConnections.size() != receivePort->width() )
  {
    std::stringstream str; str << "AudioSignalFlow: Number of incoming channels (" << receivingConnections.size() << ") to receive port " << fullyQualifiedName( *receivePort )
      << " does not match the port width (" << receivePort->width() << ").";
    throw std::logic_error( str.str() );
  }
  ChannelVector sendRange( receivePort->width(), AudioChannel( nullptr, 0 ) );
  for( ConnectionList::value_type const & conn : receivingConnections )
  {
    std::size_t receiveChannel = conn.second.channel();
    if( receiveChannel >= receivePort->width() )
    {
      std::stringstream str; str << "AudioSignalFlow: Connection entry " << conn.first << "->" << conn.second << " exceeds the receiver port width (" << receivePort->width() << ").";
      throw std::logic_error( str.str() );
    }
    if( sendRange[receiveChannel].port() != nullptr )
    {
      std::stringstream str; str << "AudioSignalFlow: More than one connection to receive channel " << conn.second << ".";
      throw std::logic_error( str.str() );
    }
    sendRange[receiveChannel] = conn.first;
  }
  return sendRange;
}

} // namespace unnamed.

bool AudioSignalFlow::initialiseAudioConnections( std::ostream & messages, AudioConnectionMap const & originalConnections, AudioConnectionMap & finalConnections )
{
  AudioConnectionMap tmpConnections; // To be assigned to or swapped with finalConnections after the initialisation was successful.

  // Collect all used audio sample types.
  std::set<AudioSampleType::Id> usedSampleTypes;
  if( mFlow.isComposite() )
  {
  std::transform( originalConnections.begin(), originalConnections.end(),
    std::inserter( usedSampleTypes, usedSampleTypes.begin() ), []( AudioConnectionMap::value_type const & connection ) { return connection.first.port()->sampleType(); } );
  }
  else
  {
    // For atomic top-level components, the sample types are determined by the audio ports of the component itself.
    std::transform( mFlow.audioPorts().begin(), mFlow.audioPorts().end(),
      std::inserter( usedSampleTypes, usedSampleTypes.begin() ), []( impl::AudioPortBaseImplementation const * port ) { return port->sampleType(); } );
  }

  std::size_t const alignment = cVectorAlignmentBytes; // Maybe replace by a function that returns the current alignment setting.
  std::size_t const blockSize = mFlow.period();
  std::size_t totalAudioPoolSize = 0;

  std::map<AudioSampleType::Id, PortOffsetLookup> allSendPortOffsets;
  std::map<AudioSampleType::Id, PortOffsetLookup> allReceivePortOffsets;

  for( AudioSampleType::Id sampleTypeId : usedSampleTypes )
  {
    PortOffsetLookup sendOffsets;
    PortOffsetLookup receiveOffsets;
    std::size_t sendPortOffset = 0;

    if( mFlow.isComposite() ) // Atomic, non-composite top-level components receive special treatment (below).
    {
      AudioConnectionMap typedConnections;
      std::copy_if( originalConnections.begin(), originalConnections.end(), std::inserter( typedConnections, typedConnections.begin() ),
                   [sampleTypeId]( AudioConnectionMap::value_type const & connection ) { return connection.first.port()->sampleType() == sampleTypeId; } );
      std::vector<impl::AudioPortBaseImplementation*> allSendPorts;
      std::transform( typedConnections.begin(), typedConnections.end(), std::back_inserter( allSendPorts ), []( AudioConnectionMap::value_type const & conn ) { return conn.first.port(); } );
      std::sort( allSendPorts.begin(), allSendPorts.end() ); // Sorting is a required before unique.
      allSendPorts.erase( std::unique( allSendPorts.begin(), allSendPorts.end() ), allSendPorts.end() ); // remove duplicates.
      // Partition the send ports such that the external ports come first.
      std::partition( allSendPorts.begin(), allSendPorts.end(), []( impl::AudioPortBaseImplementation const * port ) { return isToplevelPort( port ); } );

      for( impl::AudioPortBaseImplementation * sendPort : allSendPorts )
      {
        sendOffsets.insert( std::make_pair( sendPort, sendPortOffset ) );
        sendPortOffset += sendPort->width();
      }
      // sendPortOffset now contains the port index one past the all original send ports.

      // Now check the receive ports and see where we need routing components to get the receive ports continuous.
      std::vector<impl::AudioPortBaseImplementation*> allReceivePorts;
      std::transform( typedConnections.begin(), typedConnections.end(), std::back_inserter( allReceivePorts ), []( AudioConnectionMap::value_type const & conn ) { return conn.second.port(); } );
      std::sort( allReceivePorts.begin(), allReceivePorts.end() ); // Sorting is a required before unique.
      allReceivePorts.erase( std::unique( allReceivePorts.begin(), allReceivePorts.end() ), allReceivePorts.end() );
      // Partition the send ports such that the external ports come first.
      std::partition( allReceivePorts.begin(), allReceivePorts.end(), []( impl::AudioPortBaseImplementation const * port ) { return isToplevelPort( port ); } );
      for( impl::AudioPortBaseImplementation * receivePort : allReceivePorts )
      {
        if( receivePort->width() == 0 ) // Zero-width port are legal, but we need to skip the computation (sends[0] would be illegal)
        {
          continue;
        }
        AudioConnectionMap localConnections; // all connections ending in receivePort
        std::copy_if( typedConnections.begin(), typedConnections.end(), std::inserter( localConnections, localConnections.begin() ),
                      [receivePort]( AudioConnectionMap::value_type const & connection ) { return connection.second.port() == receivePort; } );

        ChannelVector sends = sendChannels( receivePort, localConnections ); // extracting by receivePort would not be necessary (already done for localConnections)
        if( contiguousRange( sends, sendOffsets ) )
        {
          impl::AudioPortBaseImplementation * correspondingSendPort = sends[0].port();
          auto const sendLookupIt = sendOffsets.find( correspondingSendPort );
          if( sendLookupIt == sendOffsets.end() )
          {
            throw std::logic_error( "Internal logic error: Send port not found in offset lookup table." );
          }
          std::size_t const receivePortOffset = sendLookupIt->second + sends[0].channel();

          receiveOffsets.insert( std::make_pair( receivePort, receivePortOffset ) );
          tmpConnections.insert( localConnections.begin(), localConnections.end() );
        }
        else
        {
          std::size_t const receiveWidth = receivePort->width();
          std::vector<std::size_t> sendIndices( receiveWidth );
          // Create index list
          for( std::size_t idx( 0 ); idx < receiveWidth; ++idx )
          {
            auto const sendPort = sends[idx].port();
            auto const offsetIt = sendOffsets.find( sendPort );
            if( offsetIt == sendOffsets.end() )
            {
              throw std::logic_error( "AudioSignalFlow::initialiseAudio(): Internal logic error"
                  "Send port not found in offset table." );
            }
            std::size_t const sendOffset = offsetIt->second + sends[idx].channel();
            sendIndices[idx] = sendOffset;
          }
          auto const minIndexIt = std::min_element( sendIndices.cbegin(), sendIndices.cend() );
          assert( minIndexIt != sendIndices.end() ); // could fail only if the sequence is empty
          std::size_t const minIndex = *minIndexIt;
          std::for_each( sendIndices.begin(), sendIndices.end(),
                         [minIndex]( std::size_t & val ) { val -= minIndex; } );

          // The width of the routing component must span all contained indices.
          auto const maxIndexIt = std::max_element( sendIndices.cbegin(), sendIndices.cend() );
          assert( maxIndexIt != sendIndices.end() ); // could fail only if the sequence is empty
          std::size_t const routingInputWidth = *maxIndexIt + 1;

          std::stringstream routingCompName;
          routingCompName << fullyQualifiedName( *receivePort ) << "_inputrouting";

          std::unique_ptr<AtomicComponent> routingComp
            = createSignalRoutingComponent( sampleTypeId,
                                            mFlow.context(),
                                            routingCompName.str().c_str(),
                                            static_cast<CompositeComponent*>(&(mFlow.component())),
                                            routingInputWidth,
                                            sendIndices );
          impl::ComponentImplementation & routingCompImpl = routingComp->implementation();

          impl::AudioPortBaseImplementation& routingIn = routingCompImpl.findAudioPort( "in" )->implementation();
          impl::AudioPortBaseImplementation& routingOut = routingCompImpl.findAudioPort( "out" )->implementation();

// Problem: By connecting all the input channels of the routing component we create additional dependencies which potentially make the signal flow
// cyclic or reduce the usable degree of parallelism
#ifdef WORKAROUND_CYCLIC_DEPENDENCIES
          for( std::size_t inputIdx( 0 ); inputIdx < routingInputWidth; ++inputIdx )
          {
            std::size_t const channelIndex = minIndex + inputIdx;
            PortOffsetLookup::const_iterator findPortIt = std::find_if( sendOffsets.cbegin(), sendOffsets.cend(),
              [channelIndex]( PortOffsetLookup::value_type const & entry )
            {
              return (channelIndex >= entry.second) and (channelIndex < entry.second + entry.first->width());
            } );
            if( findPortIt == sendOffsets.end() )
            {
              throw std::logic_error( "AudioSignalFlow::initialiseAudio(): Internal logic error"
                  "Send port needed for a contiguous send range of an internal routing port found in offset table." );
            }
            std::size_t const finalSendIndex = channelIndex - findPortIt->second;

            bool const usedChannel = std::find( sendIndices.begin(), sendIndices.end(), inputIdx ) != sendIndices.end();
            if( usedChannel )
            {
              tmpConnections.insert( AudioChannel( findPortIt->first, finalSendIndex ),
                                     AudioChannel( &routingIn, inputIdx ) );
            }
          }
#else
          for( std::size_t inputIdx( 0 ); inputIdx < routingInputWidth; ++inputIdx )
          {
            std::size_t const channelIndex = minIndex + inputIdx;
            PortOffsetLookup::const_iterator findPortIt = std::find_if( sendOffsets.cbegin(), sendOffsets.cend(),
              [channelIndex]( PortOffsetLookup::value_type const & entry )
            {
              return (channelIndex >= entry.second) and (channelIndex < entry.second + entry.first->width());
            } );
            if( findPortIt == sendOffsets.end() )
            {
              throw std::logic_error( "AudioSignalFlow::initialiseAudio(): Internal logic error"
                  "Send port needed for a contiguous send range of an internal routing port found in offset table." );
            }
            std::size_t const finalSendIndex = channelIndex - findPortIt->second;
            tmpConnections.insert( AudioChannel( findPortIt->first, finalSendIndex ),
                                   AudioChannel( &routingIn, inputIdx ) );
          }
#endif
          for( std::size_t sigIdx( 0 ); sigIdx < receiveWidth; ++sigIdx )
          {
            tmpConnections.insert( AudioChannel( &routingOut, sigIdx ), AudioChannel( receivePort, sigIdx ) );
          }

          sendOffsets.insert( std::make_pair( &routingOut, sendPortOffset ) );
          receiveOffsets.insert( std::make_pair( &routingIn, minIndex ) );
          // Insert the original receive port as well. Same offset as the send port of the inserted routing component.
          receiveOffsets.insert( std::make_pair( receivePort, sendPortOffset ) );
          sendPortOffset += receiveWidth;
          mInfrastructureComponents.push_back( std::move( routingComp ) );
        }
      }
    }
    else
    {
      // For atomic ports, all ports are considered as send ports, because buffers need to be allocated for them.
      for( auto portIt( mFlow.audioPorts().begin() ); portIt != mFlow.audioPorts().end(); ++portIt )
      {
        std::size_t const portWidth = (*portIt)->width();
        sendOffsets.insert( std::make_pair( *portIt, sendPortOffset ) );
        sendPortOffset += portWidth;
      }
    }
    allSendPortOffsets.insert( std::make_pair( sampleTypeId, std::move( sendOffsets ) ) );
    allReceivePortOffsets.insert( std::make_pair( sampleTypeId, std::move( receiveOffsets ) ) );

    std::size_t const typedPoolSize = sendPortOffset * efl::nextAlignedSize( blockSize*AudioSampleType::typeSize( sampleTypeId ), alignment );
    totalAudioPoolSize += typedPoolSize;
  }
  mAudioSignalPool.reset( new AudioSignalPool( totalAudioPoolSize, alignment ) );

  // Second run: Do the actual buffer assignment (for contiguous input and output ranges)
  std::size_t poolOffsetBytes = 0;
  for( AudioSampleType::Id sampleTypeId : usedSampleTypes )
  {
    std::size_t const typeOffset = poolOffsetBytes;
    std::size_t typedChannelCount = 0;
    std::size_t const channelSizeBytes = efl::nextAlignedSize( blockSize*AudioSampleType::typeSize(sampleTypeId), alignment );
    if( channelSizeBytes % AudioSampleType::typeSize(sampleTypeId) )
    {
      throw std::logic_error( "AudioSignalFlow::initialiseAudio(): Internal logic error"
                              "Aligned channel stride is not a multiple of the sample size." );
    }
    std::size_t const channelSizeSamples = channelSizeBytes / AudioSampleType::typeSize(sampleTypeId);

    PortOffsetLookup const sendOffsets = allSendPortOffsets[sampleTypeId] ;
    PortOffsetLookup const receiveOffsets = allReceivePortOffsets[sampleTypeId];

    for( auto const sendEntry : sendOffsets )
    {
      std::size_t const portWidth = sendEntry.first->width();
      std::size_t const channelOffset = sendEntry.second;
      std::size_t finalByteOffset = typeOffset + channelSizeBytes * channelOffset;
      sendEntry.first->setBufferConfig( mAudioSignalPool->basePointer() + finalByteOffset, channelSizeSamples );

      typedChannelCount += portWidth;
    }
    poolOffsetBytes += typedChannelCount * channelSizeBytes;

    // Same for receive ports.
    for( auto const receiveEntry : receiveOffsets )
    {
#ifndef WORKAROUND_CYCLIC_DEPENDENCIES
      impl::AudioPortBaseImplementation * port = receiveEntry.first;
      // Extra sanity checking whether the insertion of internal signal routing primitives has been successful.
      // Makes sense (and can be done) only for composite top-level flows.
      if( mFlow.isComposite() )
      {
        ChannelVector sends = sendChannels( port, tmpConnections );
        if( not contiguousRange( sends, sendOffsets ) )
        {
          throw std::logic_error( "AudioSignalFlow::initialiseAudio(): Internal logic error"
            "Non-contiguous port input range despite previous modification stage." );
        }
      }
#endif
      std::size_t const channelOffset = receiveEntry.second;
      std::size_t finalByteOffset = typeOffset + channelSizeBytes * channelOffset;
      receiveEntry.first->setBufferConfig( mAudioSignalPool->basePointer() + finalByteOffset, channelSizeSamples );
    }
  }
  if( poolOffsetBytes != totalAudioPoolSize )
  {
    throw std::logic_error( "AudioSignalFlow::initialiseAudio(): Internal logic error: "
        "Size of audio channel buffers differs from previously calculated value." );
  }

  mTopLevelAudioInputs.clear();
  std::copy_if( mFlow.audioPorts().begin(), mFlow.audioPorts().end(), std::back_inserter(mTopLevelAudioInputs),
                  []( impl::AudioPortBaseImplementation const * port ){ return port->direction() == PortBase::Direction::Input; } );

  mTopLevelAudioOutputs.clear();
  std::copy_if( mFlow.audioPorts().begin(), mFlow.audioPorts().end(), std::back_inserter(mTopLevelAudioOutputs),
                  []( impl::AudioPortBaseImplementation const * port ){ return port->direction() == PortBase::Direction::Output; } );

  // Not sure whether we want to keep that or whether the process() stage should access the top-level input and output ports directly.
  // The code below fails if there are audio ports with types differing from the standard type.
  // Also, if there is more than one in- or output, the ordering is undefined.
  mCaptureChannels.clear();
  mPlaybackChannels.clear();
  std::size_t externalCaptureChannelIdx = 0;
  for( impl::AudioPortBaseImplementation * port : mTopLevelAudioInputs )
  {
    char * basePointer = static_cast<char*>(port->basePointer());
    std::size_t const width = port->width();
    std::size_t const stride = port->channelStrideBytes();
    for( std::size_t chIdx( 0 ); chIdx < width; ++chIdx, ++externalCaptureChannelIdx )
    {
      mCaptureChannels.push_back( basePointer + chIdx * stride );
    }
  }
  std::size_t externalPlaybackChannelIdx = 0;
  for( impl::AudioPortBaseImplementation * port : mTopLevelAudioOutputs )
  {
    char * basePointer = static_cast<char*>(port->basePointer());
    std::size_t const width = port->width();
    std::size_t const stride = port->channelStrideBytes();
    for( std::size_t chIdx( 0 ); chIdx < width; ++chIdx, ++externalPlaybackChannelIdx )
    {
      mPlaybackChannels.push_back( basePointer + chIdx * stride );
    }
  }

  finalConnections.swap( tmpConnections );
  return true;
}

AudioSignalFlow::ParameterExchangeMutexType &
AudioSignalFlow::parameterExchangeMutex() const
{
  return *mParameterExchangeMutex;
}

#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
visr::rrl::RuntimeProfiler const &
AudioSignalFlow::runtimeProfiler()  const
{
  if( not mRuntimeProfiler )
  {
    throw std::logic_error( "Runtime profiling is not enabled." );
  }
  return *mRuntimeProfiler;
}

visr::rrl::RuntimeProfiler &
AudioSignalFlow::runtimeProfiler()
{
  if( not mRuntimeProfiler )
  {
    throw std::logic_error( "Runtime profiling is not enabled." );
  }
  return *mRuntimeProfiler;
}

bool AudioSignalFlow::runtimeProfilingEnabled() const
{
  return mRuntimeProfiler != nullptr;
}

bool AudioSignalFlow::enableRuntimeProfiling( std::size_t measurementBufferSize )
{
  bool const previous = runtimeProfilingEnabled();
  mRuntimeProfiler.reset( new RuntimeProfiler( *this, measurementBufferSize ) );
  return previous;
}

bool AudioSignalFlow::disableRuntimeProfiling()
{
  bool const previous = runtimeProfilingEnabled();
  mRuntimeProfiler.reset();
  return previous;
}
#endif

} // namespace rrl
} // namespace visr
