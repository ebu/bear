/* Copyright Institute of Sound and Vibration Research - All rights reserved */


#include <libvisr/audio_port_base.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/audio_sample_type.hpp>
#include <libvisr/port_base.hpp>
#include <libvisr/constants.hpp>

#include <libvisr/component.hpp>
#include <libvisr/signal_flow_context.hpp>

// Extend the interface beyond the audio ports visible in the C++ API
#include <libvisr/impl/audio_port_base_implementation.hpp>
#include <libvisr/impl/component_implementation.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <ciso646>
#include <cstdint>

namespace visr
{
namespace python
{
namespace visr
{

namespace py = pybind11;

namespace // unnamed
{

template<typename DataType>
py::array_t<DataType> getInputBuffer( AudioInputT<DataType> & port )
{
  py::buffer_info info ( const_cast<DataType*>(port.data()),
    sizeof(DataType),
    py::format_descriptor<DataType>::format(),
    2 /* number of dimensions */,
    { port.width(), port.channelStrideSamples() },
    { sizeof( DataType ) * port.channelStrideSamples(), sizeof( DataType ) }
  );
  return py::array_t<DataType>( info );
}

template<typename DataType>
py::array_t<DataType> getOutputBuffer( AudioOutputT<DataType> & port )
{
  return py::array_t<DataType>( py::buffer_info( port.data(),
    sizeof( DataType ),
    py::format_descriptor<DataType>::format(),
    2 /* number of dimensions */,
    { port.width(), port.channelStrideSamples() },
    { sizeof( DataType ) * port.channelStrideSamples(), sizeof( DataType ) } ) );
}

template<typename DataType>
py::array_t<DataType> getInputChannel( AudioInputT<DataType> & port, std::size_t index )
{
  py::buffer_info info( const_cast<DataType*>(port.at(index)),
    sizeof( DataType ),
    py::format_descriptor<DataType>::format(),
    1 /* number of dimensions */,
    { port.channelStrideSamples() },
    { sizeof( DataType ) }
  );
  return py::array_t<DataType>( info );
}

template<typename DataType>
py::array_t<DataType> getOutputChannel( AudioOutputT<DataType> & port, std::size_t index )
{
  return py::array_t<DataType>( py::buffer_info( const_cast<DataType*>(port.at( index )),
    sizeof( DataType ),
    py::format_descriptor<DataType>::format(),
    1 /* number of dimensions */,
    { port.channelStrideSamples() },
    { sizeof( DataType ) } ) );
}

template<typename DataType>
void setOutputBuffer( AudioOutputT<DataType> & port, py::array_t<DataType> & matrix )
{
  py::buffer_info info = matrix.request();
  // TODO: check compatibility of matrices.
  std::size_t const numChannels = port.width();
  std::size_t const bufferSize = port.channelStrideSamples();  // Should be blockSize
  if( (info.ndim != 2) or (info.shape[0] != static_cast<py::ssize_t>(numChannels))
      or (info.shape[1] != static_cast<py::ssize_t>(bufferSize) ) )  {
    throw std::invalid_argument( "AudioOutputPort.set(): matrix shape does not match." );
  }
  // Perform copying
  // TODO: Replace by optimised implementation
  std::size_t const stride0 = info.strides[0] / sizeof(DataType);
  std::size_t const stride1 = info.strides[1] / sizeof(DataType);
  DataType const * srcPtr = matrix.data();
  for( std::size_t chIdx(0); chIdx < numChannels; ++chIdx )
  {
    DataType * destPtr = port[chIdx];
    DataType const * chPtr = srcPtr + stride0 * chIdx;
    for( std::size_t sampleIdx(0); sampleIdx < bufferSize; ++sampleIdx, ++destPtr, chPtr += stride1 )
    {
      *destPtr = *chPtr;
    }
  }
}

/// Bind an audio input of a given sample type to the specified name.
template<typename DataType >
void exportAudioInput( py::module & m, char const * name )
{
  py::class_<AudioInputT<DataType>, AudioInputBase >( m, name, py::buffer_protocol() )
    .def( py::init<char const*, Component &, std::size_t>(),
      py::arg("name"), py::arg("parent"), py::arg("width") = 0 )
    .def_buffer( []( AudioInputT<DataType> &port ) -> py::buffer_info
  {
    return py::buffer_info( const_cast<DataType*>(port.data()),
      sizeof( DataType ),
      py::format_descriptor<DataType>::format(),
      2 /* number of dimensions */,
      { port.width(), port.channelStrideSamples() },
      { sizeof( DataType ) * port.channelStrideSamples(), sizeof( DataType ) }
    );
  } )
  .def( "data", &getInputBuffer<DataType> )
  .def( "channel", &getInputChannel<DataType>, py::arg("index") )
  .def( "__getitem__", &getInputChannel<DataType>, py::arg( "index" ) )
    ;
}

/// Bind an audio output of a given sample type to the specified name.
template<typename DataType >
void exportAudioOutput( py::module & m, char const * name )
{
  py::class_<AudioOutputT<DataType>, AudioOutputBase >( m, name, py::buffer_protocol() )
    .def( py::init<char const*, Component &, std::size_t>(),
      py::arg( "name" ), py::arg( "parent" ), py::arg( "width" ) = 0 )
    .def_buffer( []( AudioOutputT<DataType> &port ) -> py::buffer_info
  {
    return py::buffer_info( const_cast<DataType*>(port.data()),
      sizeof( DataType ),
      py::format_descriptor<DataType>::format(),
      2 /* number of dimensions */,
      { port.width(), port.channelStrideSamples() },
      { sizeof( DataType ) * port.channelStrideSamples(), sizeof( DataType ) }
    );
  } )

    .def( "data", &getOutputBuffer<DataType>, py::return_value_policy::reference_internal )
    .def( "set", &setOutputBuffer<DataType> )
    // TODO: can we get an assign operator?
    .def( "channel", &getOutputChannel<DataType>, py::return_value_policy::reference_internal, py::arg( "index" ) )
    .def( "__getitem__", &getOutputChannel<DataType>, py::return_value_policy::reference_internal, py::arg( "index" ) )
    ;
}

} // unnamed namespace

void exportAudioPort( py::module & m)
{
  /**
   * Define the common base class for audio ports.
   * Instantiation of this class is not intended, therefore no constructors are exposed.
   */
  py::class_<AudioPortBase>( m, "AudioPortBase" )
    .def_property( "width", &AudioPortBase::width, &AudioPortBase::setWidth )
    .def_property_readonly( "alignmentSamples", &AudioPortBase::alignmentSamples )
    .def_property_readonly( "alignmentBytes", &AudioPortBase::alignmentBytes )
    .def_property_readonly( "channelStrideSamples", &AudioPortBase::channelStrideSamples )
    .def_property_readonly( "channelStrideBytes", &AudioPortBase::channelStrideBytes )
    .def_property_readonly( "sampleSize", &AudioPortBase::sampleSize )
    .def_property_readonly( "sampleType", &AudioPortBase::sampleType )
    .def_property_readonly( "initialised", []( AudioPortBase const & port ){ return port.implementation().initialised(); } )
    .def_property_readonly( "name", []( AudioPortBase const & port ) { return port.implementation().name(); } )
    .def_property_readonly( "qualifiedName", []( AudioPortBase const & port ) { return port.implementation().parent().name()
      + ":" +port.implementation().name(); }, "Return the port name as a component:port combination" )
    .def_property_readonly( "fullName", []( AudioPortBase const & port ) { return port.implementation().parent().name()
    + ":" +port.implementation().name(); }, "Return the port name as a component:port combination with a fully hierarchical component name." )
    .def_property_readonly( "direction", []( AudioPortBase const & port ) { return port.implementation().direction(); } )
    ;

  /**
   * Instantiable class for polymorphic audio inputs with a parameterisable sample type.
   * The name deliberately differs from the C++ class (which is a class template) for a slicker syntax.
   * It also enables inputs with default sample type (float) using 'AudioInput' by using the default argument for the sample type.
   */
  py::class_<AudioInputBase, AudioPortBase>( m, "AudioInput", "Audio port class with a selectable sample type" )
    .def( py::init<char const *, Component&, AudioSampleType::Id, std::size_t>(), 
      py::arg("name"), py::arg("parent"), py::arg("sampleType") = AudioSampleType::floatId, py::arg("width") = 0,
      "Constructor with choosable sample type")
  ;

  /**
  * Instantiable class for polymorphic audio outputs with a parameterisable sample type.
  * The name deliberately differs from the C++ class (which is a class template) for a slicker syntax.
  * It also enables inputs with default sample type (float) using 'AudioOutput' by using the default argument for the sample type.
  */
  py::class_<AudioOutputBase, AudioPortBase>( m, "AudioOutput", "Audio output port class with parameterisable sample type" )
    //.def( py::init<char const *, Component &, AudioSampleType::Id>() )
    .def( py::init<char const *, Component&, AudioSampleType::Id, std::size_t>(),
      py::arg( "name" ), py::arg( "parent" ), py::arg( "sampleType" ) = AudioSampleType::floatId, py::arg( "width" ) = 0,
      "Constructor with choosable sample type" )
  ;

  exportAudioInput<float>( m, "AudioInputFloat" );
  exportAudioInput<double>( m, "AudioInputDouble" );
  exportAudioInput<long double>( m, "AudioInputLongDouble" );
  exportAudioInput<int8_t>( m, "AudioInputInt8" );
  exportAudioInput<uint8_t>( m, "AudioInputUint8" );
  exportAudioInput<int16_t>( m, "AudioInputInt16" );
  exportAudioInput<uint16_t>( m, "AudioInputUint16" );
  exportAudioInput<int32_t>( m, "AudioInputInt32" );
  exportAudioInput<uint32_t>( m, "AudioInputUint32" );
  exportAudioInput<std::complex<float> >( m, "AudioInputComplexFloat" );
  exportAudioInput<std::complex<double> >( m, "AudioInputComplexDouble" );

  exportAudioOutput<float>( m, "AudioOutputFloat" );
  exportAudioOutput<double>( m, "AudioOutputDouble" );
  exportAudioOutput<long double>( m, "AudioOutputLongDouble" );
  exportAudioOutput<int8_t>( m, "AudioOutputInt8" );
  exportAudioOutput<uint8_t>( m, "AudioOutputUint8" );
  exportAudioOutput<int16_t>( m, "AudioOutputInt16" );
  exportAudioOutput<uint16_t>( m, "AudioOutputUint16" );
  exportAudioOutput<int32_t>( m, "AudioOutputInt32" );
  exportAudioOutput<uint32_t>( m, "AudioOutputUint32" );
  exportAudioOutput<std::complex<float> >( m, "AudioOutputComplexFloat" );
  exportAudioOutput<std::complex<double> >( m, "AudioOutputComplexDouble" );
}

} // namepace visr
} // namespace python
} // namespace visr
