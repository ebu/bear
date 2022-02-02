/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRRL_AUDIO_SIGNAL_FLOW_HPP_INCLUDED
#define VISR_LIBRRL_AUDIO_SIGNAL_FLOW_HPP_INCLUDED

#include "export_symbols.hpp"

//#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
//#include <runtime_profiler.hpp>
//#endif

#include <libvisr/audio_port_base.hpp>
#include <libvisr/communication_protocol_base.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <mutex>
#include <vector>

namespace visr
{
// Forward declarations
class AtomicComponent;
class ParameterPortBase;

namespace impl
{
class ComponentImplementation;
class CompositeComponentImplementation;
} // namespace impl

namespace rrl
{
// Forward declarations
class AudioSignalPool;
class AudioConnectionMap;
class ParameterConnectionMap;
#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
class RuntimeProfiler;
#endif

/**
 * Base class for signal flows, i.e., graphs of connected audio
 * components which perform an audio signal processing operation.
 * This base class provides the infrastructure for setting up the
 * graphs and for transferring the input and output samples.
 * For the audio processing, this class provides a callback interface
 * that must be called in regular intervals (i.e., for a fixed number
 * of samples consumed and generated, respectively).
 */
class VISR_RRL_LIBRARY_SYMBOL AudioSignalFlow
{
public:
  using SignalIndexType =
      std::size_t; // TODO: Check whether to introduce a consistently used type
                   // alias for indices

  /**
   * Constructor.
   * @param flow The component (composite or atomic) containing the processing
   * functionality.
   */
  explicit AudioSignalFlow( Component & flow );

  /**
   * Destructor.
   */
  ~AudioSignalFlow();

  /**
   * Method to transfer the capture and playback samples to and from
   * the locations where they are expected, and execute the contained atomic components.
   * @param captureSamples Array of pointers to the first sample of each channel
   * @param playbackSamples samples Array of pointers to the first sample in each playback channel
   * @param captureStrideSamples Stride (in number of elements) between successive samples
   * in the capture channel buffers. A value of 1 (default) denotes contiguous samples.
   * @param playbackStrideSamples Stride (in number of elements) between successive samples
   * in the playback channel buffers. A value of 1 (default) denotes contiguous samples.
   */
  bool process( SampleType const * const * captureSamples,
                SampleType * const * playbackSamples,
                std::size_t captureStrideSamples = 1,
                std::size_t playbackStrideSamples = 1 );

  /**
   * Alternative process() function assuming matrices with fixed channel and
   * element strides for the inputs and outputs.
   * @param captureSamples Pointer to the matrix of input samples.
   * @param[out] playbackSamples Pointer to the first element of the matrix of
   * output samples filled by the process() call.
   * @param captureChannelStride Number of samples between consecutive audio
   * channels of the input matrix.
   * @param captureSampleStride Number of samples between consecutive audio
   * channels of the input matrix. A value of 1 corresponds to consecutively
   * stored samples.
   * @param playbackChannelStride Number of samples between consecutive audio
   * channels of the output matrix.
   * @param playbackSampleStride Number of samples between consecutive audio
   * channels of the output matrix. A value of 1 corresponds to consecutively
   * stored samples.
   * @todo Provide a callback function for interfacing with audio interfaces.
   */
  void process( SampleType const * captureSamples,
                std::size_t captureChannelStride,
                std::size_t captureSampleStride,
                SampleType * playbackSamples,
                std::size_t playbackChannelStride,
                std::size_t playbackSampleStride );

  /**
   * A static, i.e., non-class function which can be registered as a
   * callback method. Calling this method triggers the transfer of the
   * passed samples and the invocation of the process() function of
   * the derived subclasses.
   * The method must only only be called after the initialisation of the
   * class is complete.
   * @param userData An opaque pointer, must hold the 'this' pointer
   * of the AudioSignalFlow object
   * @param captureSamples A pointer array to arrays of input samples
   * to be processed. The pointer array must hold numberOfCaptureChannels()
   * elements, and each sample array must hold period() samples.
   * @param playbackSamples  A pointer array to arrays of output samples
   * to hold the results of the operation. The pointer array must hold
   * numberOfPlaybackChannels() elements, and each sample array must hold
   * period() samples.
   * @param status A enumeration type to hold the result of
   * the process() function. Typically used to signal error conditions
   * or to request termination.
   * @todo After the redesign, the translation to a callback function (and
   * discarding the object pointer) needs to be done somewhere else!
   */
  static void processFunction( void * userData,
                               SampleType const * const * captureSamples,
                               SampleType * const * playbackSamples,
                               bool & status );

  /**
   * Return the number of samples processed in each process() function
   * @note At the moment this is required by the Python binding.
   */
  std::size_t period() const;

  /**
   * Query methods for the external audio inputs and outputs.
   * The audio inputs (capture) and outputs (playback) are combined into one
   * buffer each, and the order is currently determined by the AudioSignalFlow
   * and can change randomly. The query methods allow to retrieve properties of
   * the external port mappings.
   */
  //@{
  /**
   * Query the width of the capture input vector, i.e., the number of external
   * input channels of the graph.
   */
  std::size_t numberOfCaptureChannels() const;

  /**
   * Query the width of the playback output vector, i.e., the number of external
   * output signals of the graph.
   */
  std::size_t numberOfPlaybackChannels() const;
  //@}

  /**
   * Return the number of separate audio capture ports.
   */
  std::size_t numberOfAudioCapturePorts() const;

  /**
   * Return the number of separate audio playback ports.
   */
  std::size_t numberOfAudioPlaybackPorts() const;

  /**
   * Return the name of the capture port indexed by \p idx
   * @throw std::out_of_range If the \p idx exceeds the number of capture ports.
   */
  char const * audioCapturePortName( std::size_t idx ) const;

  /**
   * Return the name of the playback port indexed by \p idx.
   * @throw std::out_of_range If the \p idx exceeds the number of playback
   * ports.
   */
  char const * audioPlaybackPortName( std::size_t idx ) const;

  /**
   * Return the port width of the capture port indexed by \p idx.
   * @throw std::out_of_range If the \p idx exceeds the number of capture ports.
   */
  std::size_t audioCapturePortWidth( std::size_t idx ) const;

  /**
   * Return the port width of the playback port indexed by \p idx.
   * @throw std::out_of_range If the \p idx exceeds the number of playback
   * ports.
   */
  std::size_t audioPlaybackPortWidth( std::size_t idx ) const;

  /**
   * Return the array offset of the capture port indexed by \p idx within the
   * capture channel array.
   * @throw std::out_of_range If the \p idx exceeds the number of capture ports.
   */
  std::size_t audioCapturePortOffset( std::size_t idx ) const;

  /**
   * Return the array offset of the playback port indexed by \p idx within the
   * playback channel array.
   * @throw std::out_of_range If the \p idx exceeds the number of playback
   * ports.
   */
  std::size_t audioPlaybackPortOffset( std::size_t idx ) const;

  /**
   * Return the native sample type of the capture port indexed by \p idx.
   * @throw std::out_of_range If the \p idx exceeds the number of capture ports.
   */
  visr::AudioSampleType::Id audioCapturePortSampleType( std::size_t idx ) const;

  /**
   * Return the native sample type of the playback port indexed by \p idx.
   * @throw std::out_of_range If the \p idx exceeds the number of playback
   * ports.
   */
  visr::AudioSampleType::Id audioPlaybackPortSampleType(
      std::size_t idx ) const;

  /**
   * Query the index of an external capture port by name.
   * @throw std::out_of_range If no external capture port with name \p name
   * exists.
   */
  std::size_t audioCapturePortIndex( char const * name ) const;

  /**
   * Query the index of an external port port by name.
   * @throw std::out_of_range If no external playback port with name \p name
   * exists.
   */
  std::size_t audioPlaybackPortIndex( char const * name ) const;

  //@}

  /**
   * Query and access external parameter ports.
   */
  //@{

  using ProtocolReceiveEndpoints =
      std::map< std::string,
                std::unique_ptr< CommunicationProtocolBase::Output > >;
  using ProtocolSendEndpoints =
      std::map< std::string,
                std::unique_ptr< CommunicationProtocolBase::Input > >;

  std::size_t numberExternalParameterReceivePorts() const
  {
    return mProtocolReceiveEndpoints.size();
  }

  std::size_t numberExternalParameterSendPorts() const
  {
    return mProtocolSendEndpoints.size();
  }

  ProtocolReceiveEndpoints const & externalParameterReceiveEndpoints() const
  {
    return mProtocolReceiveEndpoints;
  }

  ProtocolSendEndpoints const & externalParameterSendEndpoints() const
  {
    return mProtocolSendEndpoints;
  }

  /**
   * Return a input protocol for a named top-level parameter port.
   * @throw std::out_of_range No top-level parameter port with this name exists.
   */
  CommunicationProtocolBase::Output & externalParameterReceivePort(
      char const * portName );

  /**
   * Return a output protocol for a named top-level parameter port.
   * @throw std::out_of_range No top-level parameter port with this name exists.
   */
  CommunicationProtocolBase::Input & externalParameterSendPort(
      char const * portName );

  //@}

  /**
   * Support for thread-safe access to the external parameter ports.
   */
  //@{

  /**
   * Data type (mutex type) used to as a guard to the parameters.
   */
  using ParameterExchangeMutexType = std::timed_mutex;

  /**
   * Retrieve a reference to the mutex data structure guarding the
   * external parameter ports.
   * External users must acquire, i.e., lock, this mutex before making
   * thread-critical accesses to parameter ports.
   * It is advisable to use a RAII-type guard object.
   * @note Calling code should block the critical section only for very
   * brief durations, and not execute code that might block, because this
   * would affect the audio rendering code.
   */
  ParameterExchangeMutexType & parameterExchangeMutex()
      const;
  //@}

#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING

  /**
   * Support for runtime profiling
   */
  //@{
  friend class visr::rrl::RuntimeProfiler;

  bool runtimeProfilingEnabled() const;

  bool enableRuntimeProfiling( std::size_t measurementBufferSize );

  bool disableRuntimeProfiling();

  visr::rrl::RuntimeProfiler const & runtimeProfiler() const;

  visr::rrl::RuntimeProfiler & runtimeProfiler();
  //@}
#endif // VISR_RRL_RUNTIME_SYSTEM_PROFILING
private:
  /**
   * Initialise the audio subsystem, i.e., creating audio buffers and connecting
   * the audio ports to the sample buffers. This might also introduce
   * infrastructure components.
   * @param [out] messages Output stream to receive verbose information about
   * connection errors.
   * @param [in] originalConnections List of audio connections of the
   * hierarchical model.
   * @param [out] finalConnections List of final audio connections of the
   * flattened model, possibly with added infrastructure components.
   * @todo Consider moving this out of the public header, either by using the
   * pimpl idiom or by making it a free function in the implementation file.
   */
  bool initialiseAudioConnections(
      std::ostream & messages,
      AudioConnectionMap const & originalConnections,
      AudioConnectionMap & finalConnections );

  /**
   * Initialise the parameter infrastructure.
   * @return True if the initialisation was successful, false otherwise. In this
   * case, \p messages should provide an explanation.
   * @param [out] messages Output stream containing error messages and warnings
   * generated during the initialisation.
   * @param [in] originalConnections List of parameter connections of the
   * hierarchical model.
   * @param [out] finalConnections Connection map to be filled during the
   * initialisation process, using the flattened version of the signal flow .
   * @todo Consider moving this out of the public header, either by using the
   * pimpl idiom or by making it a free function in the implementation file.
   */
  bool initialiseParameterInfrastructure(
      std::ostream & messages,
      ParameterConnectionMap const & originalConnections,
      ParameterConnectionMap & finalConnections );

  /**
   * Initialise the schedule for executing the contained elements.
   * @return Boolean value indicating whether the initialisation was successful.
   * @param [out] messages Output stream containing error messages and warnings
   * generated during the initialisation.
   * @param audioConnections The audio connection relations of the final signal
   * flow (possibly including additional infrastructure components created
   * during initialisation)
   * @param parameterConnections The parameter connection relations of the final
   * signal flow (possibly including additional infrastructure components
   * created during initialisation)
   * @todo Consider moving this out of the public header, either by using the
   * pimpl idiom or by making it a free function in the implementation file.
   */
  bool initialiseSchedule(
      std::ostream & messages,
      AudioConnectionMap const & audioConnections,
      ParameterConnectionMap const & parameterConnections );
  /**
   * Return the total number of communication protocols in the signal
   * flow.
   */
  std::size_t numberCommunicationProtocols() const;

  /**
   * Method called within the processFunction callback to execute the atomic
   * components of the graph
   */
  void executeComponents();

  /**
   * The signal flow handled by this object.
   * Can be either an atomic or a (hierarchical) composite component/
   */
  impl::ComponentImplementation & mFlow;

  /**
   * Parameter infrastructure
   */
  //@{

  using CommunicationProtocolContainer =
      std::vector< std::unique_ptr< CommunicationProtocolBase > >;

  CommunicationProtocolContainer mCommunicationProtocols;
  //@}

  /**
   * The communication area for this signal flow.
   */
  std::unique_ptr< AudioSignalPool > mAudioSignalPool;

  /**
   * These ports are the top-level system inputs and outputs.
   * They correspond to the capture and playback indices.
   * @note at the moment the order of the ports is determined by the system.
   */
  //@{
  std::vector< impl::AudioPortBaseImplementation * > mTopLevelAudioInputs;
  std::vector< impl::AudioPortBaseImplementation * > mTopLevelAudioOutputs;
  //@}

  std::vector< char * > mCaptureChannels;
  std::vector< char * > mPlaybackChannels;

  /**
   * Data structures to hold top-level parameter ports (or their input/output
   * facilities)
   */
  //@{
  ProtocolReceiveEndpoints mProtocolReceiveEndpoints;

  ProtocolSendEndpoints mProtocolSendEndpoints;

  //@}

  using InternalComponentList =
      std::vector< std::unique_ptr< AtomicComponent > >;

  InternalComponentList mInfrastructureComponents;

  using ProcessingSchedule = std::vector< AtomicComponent * >;

  ProcessingSchedule mProcessingSchedule;

  /**
   * Synchronisation object for accesses to external parameter ports.
   */
  mutable std::unique_ptr< ParameterExchangeMutexType >
      mParameterExchangeMutex;

#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
  /**
   *
   */
  //@{

  std::unique_ptr< RuntimeProfiler > mRuntimeProfiler;
  //@}
#endif
};

} // namespace rrl
} // namespace visr

#endif // #ifndef VISR_LIBRRL_AUDIO_SIGNAL_FLOW_HPP_INCLUDED
