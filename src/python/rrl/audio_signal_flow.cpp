/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librrl/audio_signal_flow.hpp>

#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
#include <librrl/runtime_profiler.hpp>
#endif

#include <libvisr/component.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <ciso646>
#include <vector>

namespace visr
{

using rrl::AudioSignalFlow;
namespace py = pybind11;

namespace python
{
namespace rrl
{

namespace // unnamed
{

py::array_t<SampleType> wrapProcess( visr::rrl::AudioSignalFlow & flow, py::array const & input )
{
  using DataType = SampleType; // Possibly replace by a template parameter later.

  if( not input.dtype().is( py::dtype::of<DataType>() ) )
  {
    throw std::invalid_argument( "AudioSignalFlow::process(): The data type input matrix does not match the used sample data type." );
  }
  if( input.ndim() != 2 )
  {
    throw std::invalid_argument( "AudioSignalFlow::process(): The input matrix is not 2D" );
  }
  if( input.shape(0) != static_cast<py::ssize_t>(flow.numberOfCaptureChannels()) )
  {
    throw std::invalid_argument( "AudioSignalFlow::process(): Dimension 1 input of the input matrix does not match the number of capture channels." );
  }
  if( input.shape(1) != static_cast<py::ssize_t>(flow.period()) )
  {
    throw std::invalid_argument( "AudioSignalFlow::process(): Dimension 0 input of the input matrix does not match the block size of the signal flow." );
  }

  py::array outputSignal( py::dtype::of<SampleType>(),
  { flow.numberOfPlaybackChannels(), flow.period() },
  { sizeof( SampleType )*flow.period(), sizeof( SampleType ) } // TODO: Take care of alignment
                              );
  std::size_t const outChannelStride = outputSignal.strides(0) / sizeof(DataType);
  std::size_t const outSampleStride = outputSignal.strides( 1 ) / sizeof( DataType );

  try
  {
    flow.process( static_cast<SampleType const *>(input.data()),
                  input.strides(0)/sizeof(SampleType),
                  input.strides(1)/sizeof(SampleType),
                  static_cast<SampleType *>(outputSignal.mutable_data()),
                  outChannelStride,
                  outSampleStride );
  }
  catch( std::exception const & ex )
  {
    // For the time being we are using std::invalid_argument because it is recognised by py and translated to a proper Python exeption
    // todo: register a more fitting exception, e.g., std::runtime_error
    throw std::invalid_argument( detail::composeMessageString( "Exception while execution signal flow:", ex.what() ) );
  }
  return outputSignal;
}

} // unnamed namespace

using ParameterExchangeMutex = visr::rrl::AudioSignalFlow::ParameterExchangeMutexType;

/**
 * Context manager class to use critical section guard with the Python with statement.
 */ 
class ParameterExchangeMutexGuard
{
public:
  /**
   * Construct a context manager object from the critical section object of the audio signal flow.
   */
  explicit ParameterExchangeMutexGuard( ParameterExchangeMutex & section )
   : mSection( section )
  {}

  /**
   * Construct the context manager directly from the audio signal flow.
   * This is a convenience constructor that avoids the need to get the criticial section object.
   */
  explicit ParameterExchangeMutexGuard( AudioSignalFlow & flow )
   : mSection( flow.parameterExchangeMutex() )
  {}

  ~ParameterExchangeMutexGuard() = default;

  ParameterExchangeMutex & enter() 
  {
    {
      py::gil_scoped_release release;
      mSection.lock();
    }
    return mSection;
  }

  bool exit( py::object exceptionType, py::object exceptionValue, py::object exceptionTrace )
  {
    mSection.unlock();
    return false; // Indicate that the exception has not been handled.
  }
private:
  ParameterExchangeMutex & mSection;
};


void exportAudioSignalFlow( py::module & m )
{
  py::class_<AudioSignalFlow>  cls( m, "AudioSignalFlow" );

  py::class_< ParameterExchangeMutex >( cls, "ParameterExchangeMutex",
R"(Python binding for the mutex type used by AudioSignalFlow to guard the exchange of parameter data.)" )
    .def( "lock", &ParameterExchangeMutex::lock, py::call_guard<py::gil_scoped_release>(), R"( Acquire the lock.)" )
    .def( "unlock", &ParameterExchangeMutex::unlock, R"(Release the lock)" )
    ;

  py::class_< ParameterExchangeMutexGuard >( cls, "ParameterExchangeMutexGuard",
R"( Context manager to acquire the parameter exchange critical section lock with the Python "with" statement.)")
    .def( py::init< ParameterExchangeMutex & >(), R"( Construct the context manager from the parameter exchange critical section mutex.)" )
    .def( py::init< AudioSignalFlow & >(), R"(Convenience constructor the context manager directly from the AudioSignalFlow.)" )
    .def( "__enter__", &ParameterExchangeMutexGuard::enter, py::return_value_policy::reference,
R"(Implement the "__enter__" method of the ContextManager API. Called within the with statment, usually not called directly by users.)" )
    .def( "__exit__", &ParameterExchangeMutexGuard::exit,
R"(Implement the "__exit__" method of the ContextManager API. Called within the with statment, usually not called directly by users.)" )
    ;

  cls
   .def( py::init<visr::Component&>(), py::keep_alive<1, 2>() )
   .def_property_readonly( "numberOfAudioCapturePorts", &AudioSignalFlow::numberOfAudioCapturePorts )
   .def_property_readonly( "numberOfAudioPlaybackPorts", &AudioSignalFlow::numberOfAudioPlaybackPorts )
   .def_property_readonly( "numberOfCaptureChannels", &AudioSignalFlow::numberOfCaptureChannels )
   .def_property_readonly( "numberOfPlaybackChannels", &AudioSignalFlow::numberOfPlaybackChannels )
   .def_property_readonly( "numberParameterReceivePorts", &AudioSignalFlow::numberExternalParameterReceivePorts )
   .def_property_readonly( "numberParameterSendPorts", &AudioSignalFlow::numberExternalParameterSendPorts )
   .def( "parameterReceivePort", &AudioSignalFlow::externalParameterReceivePort, py::arg("portName"), py::return_value_policy::reference )
   .def( "parameterSendPort", &AudioSignalFlow::externalParameterSendPort, py::arg("portName"), py::return_value_policy::reference )
   .def( "parameterReceivePorts", &AudioSignalFlow::externalParameterReceiveEndpoints, py::return_value_policy::reference )
   .def( "parameterSendPorts", &AudioSignalFlow::externalParameterSendEndpoints )
   .def( "audioCapturePortName", &AudioSignalFlow::audioCapturePortName, py::arg("index"), py::return_value_policy::reference )
   .def( "audioPlaybackPortName", &AudioSignalFlow::audioPlaybackPortName, py::arg( "index" ), py::return_value_policy::reference )
   .def( "audioCapturePortWidth", &AudioSignalFlow::audioCapturePortWidth, py::arg( "index" ) )
   .def( "audioPlaybackPortWidth", &AudioSignalFlow::audioPlaybackPortWidth, py::arg( "index" ) )
   .def( "audioCapturePortOffset", &AudioSignalFlow::audioCapturePortOffset, py::arg( "index" ) )
   .def( "audioPlaybackPortOffset", &AudioSignalFlow::audioPlaybackPortOffset, py::arg( "index" ) )
   .def( "audioCapturePortSampleType", &AudioSignalFlow::audioCapturePortSampleType, py::arg( "index" ) )
   .def( "audioPlaybackPortSampleType", &AudioSignalFlow::audioPlaybackPortSampleType, py::arg( "index" ) )
   .def( "audioCapturePortIndex", &AudioSignalFlow::audioCapturePortIndex, py::arg( "name" ) )
   .def( "audioPlaybackPortIndex", &AudioSignalFlow::audioPlaybackPortIndex, py::arg( "name" ) )
   .def( "process", [](visr::rrl::AudioSignalFlow & flow, py::array const & input) /*-> py::array_t<SampleType>*/ { return wrapProcess( flow, input );}, py::arg("audioInput"), py::return_value_policy::take_ownership, "process() variant for flows with no audio inputs" )
   .def( "process",
     [](visr::rrl::AudioSignalFlow & flow) /*-> py::array_t<SampleType>*/
     {
       std::initializer_list<std::size_t> const shape{ 0, flow.period() };
       py::array_t<SampleType> dummy( shape, nullptr );
       return wrapProcess( flow, dummy );
     },
     py::return_value_policy::take_ownership, "process() variant for flows with no audio inputs." )
   .def( "parameterExchangeMutex", &AudioSignalFlow::parameterExchangeMutex,
     py::return_value_policy::reference, R"(Obtain the mutex for guarding the parameter data exchange,)" )
#ifdef VISR_RRL_RUNTIME_SYSTEM_PROFILING
   .def( "runtimeProfilingEnabled", &AudioSignalFlow::runtimeProfilingEnabled )
   .def( "enableRuntimeProfiling", &AudioSignalFlow::enableRuntimeProfiling, py::arg( "measurementBufferSize" ) )
   .def( "disableRuntimeProfiling", &AudioSignalFlow::disableRuntimeProfiling )
   .def( "runtimeProfiler", static_cast< visr::rrl::RuntimeProfiler &(AudioSignalFlow::*)()>(
      &AudioSignalFlow::runtimeProfiler ), py::return_value_policy::reference )
#endif
  ;
}

} // namespace rrl
} // namespace python
} // namespace visr
