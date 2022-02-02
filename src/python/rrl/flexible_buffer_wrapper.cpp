/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librrl/flexible_buffer_wrapper.hpp>
#include <librrl/audio_signal_flow.hpp>

#include <libvisr/component.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <ciso646>
#include <vector>

namespace visr
{
namespace py = pybind11;

namespace python
{
namespace rrl
{
class FlexibleBufferWrapper: public visr::rrl::FlexibleBufferWrapper
{
public:
  explicit FlexibleBufferWrapper( visr::rrl::AudioSignalFlow &flow )
   : visr::rrl::FlexibleBufferWrapper( flow )
   , mFlow( flow )
  {
  }

  py::array_t< SampleType > process( py::array const &input );

private:
  visr::rrl::AudioSignalFlow &mFlow;
};

py::array_t< SampleType > FlexibleBufferWrapper::process(
    py::array const &input )
{
  using DataType =
      SampleType; // Possibly replace by a template parameter later.

  if( not input.dtype().is( py::dtype::of< DataType >() ) )
  {
    throw std::invalid_argument(
        "FlexibleBufferWrapper::process(): The data type input matrix does not "
        "match the used sample data type." );
  }
  if( input.ndim() != 2 )
  {
    throw std::invalid_argument(
        "FlexibleBufferWrapper::process(): The input matrix is not 2D" );
  }
  if( input.shape( 0 ) !=
      static_cast< py::ssize_t >( mFlow.numberOfCaptureChannels() ) )
  {
    throw std::invalid_argument(
        "FlexibleBufferWrapper::process(): Dimension 1 input of the input "
        "matrix does not match the number of capture channels." );
  }
  std::size_t const numFrames = static_cast< std::size_t >( input.shape( 1 ) );

  py::array outputSignal(
      py::dtype::of< SampleType >(),
      { mFlow.numberOfPlaybackChannels(), numFrames },
      { sizeof( SampleType ) * numFrames, sizeof( SampleType ) }
      // TODO: Take care of alignment
  );
  std::size_t const outChannelStride =
      outputSignal.strides( 0 ) / sizeof( DataType );
  std::size_t const outSampleStride =
      outputSignal.strides( 1 ) / sizeof( DataType );

  try
  {
    visr::rrl::FlexibleBufferWrapper::process(
        static_cast< SampleType const * >( input.data() ),
        input.strides( 0 ) / sizeof( SampleType ),
        input.strides( 1 ) / sizeof( SampleType ),
        static_cast< SampleType * >( outputSignal.mutable_data() ),
        outChannelStride, outSampleStride,
        numFrames );
  }
  catch( std::exception const &ex )
  {
    // For the time being we are using std::invalid_argument because it is
    // recognised by py and translated to a proper Python exeption todo:
    // register a more fitting exception, e.g., std::runtime_error
    throw std::invalid_argument( detail::composeMessageString(
        "Exception while execution signal flow with flexible buffering:",
        ex.what() ) );
  }
  return outputSignal;
}

void exportFlexibleBufferWrapper( py::module &m )
{
  py::class_< FlexibleBufferWrapper >( m, "FlexibleBufferWrapper" )
      .def( py::init< visr::rrl::AudioSignalFlow & >(), py::arg( "flow" ) )
      .def("process", &FlexibleBufferWrapper::process,
          py::arg( "input" ),
          py::return_value_policy::take_ownership )
      .def( "process",
     [](FlexibleBufferWrapper & flow, std::size_t numFrames ) /*-> py::array_t<SampleType>*/
     {
       std::initializer_list<std::size_t> const shape{ 0, numFrames };
       py::array_t<SampleType> dummy( shape, nullptr );
       return flow.process( dummy );
     }, py::arg( "numFrames" ) = 0, "Overloaded process() call for signal flows with no inputs." )
      ;
}

} // namespace rrl
} // namespace python
} // namespace visr
