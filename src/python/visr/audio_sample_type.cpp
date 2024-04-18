/* Copyright Institute of Sound and Vibration Research - All rights reserved */


#include <libvisr/audio_sample_type.hpp>
#include <libvisr/constants.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <ciso646>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace visr
{
namespace python
{
namespace visr
{

namespace py = pybind11;

namespace // unnamed
{

using AudioSampleTypeLookupEntry = std::tuple<py::dtype, AudioSampleType::Id>;
using AudioSampleTypeLookup = std::vector< AudioSampleTypeLookupEntry >;

static AudioSampleTypeLookup getSampleTypeIdLookup() {
  AudioSampleTypeLookup table = {
      AudioSampleTypeLookupEntry{ py::dtype::of<float>(), AudioSampleType::floatId },
      AudioSampleTypeLookupEntry{ py::dtype::of<double>(), AudioSampleType::doubleId },
      AudioSampleTypeLookupEntry{ py::dtype::of<long double>(), AudioSampleType::longDoubleId },
      AudioSampleTypeLookupEntry{ py::dtype::of<uint8_t>(), AudioSampleType::uint8Id },
      AudioSampleTypeLookupEntry{ py::dtype::of<int8_t>(), AudioSampleType::int8Id },
      AudioSampleTypeLookupEntry{ py::dtype::of<int16_t>(), AudioSampleType::uint16Id },
      AudioSampleTypeLookupEntry{ py::dtype::of<int16_t>(), AudioSampleType::uint16Id },
      AudioSampleTypeLookupEntry{ py::dtype::of<uint32_t>(), AudioSampleType::uint32Id },
      AudioSampleTypeLookupEntry{ py::dtype::of<int32_t>(), AudioSampleType::int32Id },
      AudioSampleTypeLookupEntry{ py::dtype::of< std::complex<float> >(), AudioSampleType::complexFloatId },
      AudioSampleTypeLookupEntry{ py::dtype::of< std::complex<double> >(), AudioSampleType::complexDoubleId }
  };

  return table;
}

} // unnamed namespace

void exportAudioSampleType( py::module & m )
{
  // shared pointer is used here so that the references to python objects that
  // this holds are destroyed when the module is destroyed, not at program exit
  // where the GIL is not held
  auto sampleTypeIdLookup = std::make_shared<AudioSampleTypeLookup>(getSampleTypeIdLookup());

  py::enum_<AudioSampleType::Id>( m, "AudioSampleType", "Enumeration of type ids for the supported audio sample types." )
    .value( "floatId", AudioSampleType::floatId )
    .value( "doubleId", AudioSampleType::doubleId )
    .value( "longDoubleId", AudioSampleType::longDoubleId )
    .value( "uint8Id", AudioSampleType::uint8Id )
    .value( "int8Id", AudioSampleType::int8Id )
    .value( "uint16Id", AudioSampleType::uint16Id )
    .value( "int16Id", AudioSampleType::int16Id )
    .value( "uint32Id", AudioSampleType::uint32Id )
    .value( "int32Id", AudioSampleType::int32Id )
    .value( "complexFloatId", AudioSampleType::complexFloatId )
    .value( "complexDoubleId", AudioSampleType::complexDoubleId )
    ;

  m.def( "audioSampleTypes",
    [sampleTypeIdLookup]()
    {
      std::vector<AudioSampleType::Id> sampleTypes;
      sampleTypes.reserve( sampleTypeIdLookup->size() );
      std::transform( sampleTypeIdLookup->begin(), sampleTypeIdLookup->end(), std::back_inserter( sampleTypes),
        []( AudioSampleTypeLookup::value_type const & v ) { return std::get<1>( v ); } );
      return sampleTypes;
    },
    "Return a list of registered audio sample types."
  );

  m.def( "audioSampleTypeToDtype", [sampleTypeIdLookup]( AudioSampleType::Id sampleType )
  {
    auto const findIt = std::find_if( sampleTypeIdLookup->begin(), sampleTypeIdLookup->end(),
      [sampleType]( AudioSampleTypeLookup::value_type const & v ) -> bool
      {
        return std::get<1>( v ) == sampleType;
      } );
    if( findIt == sampleTypeIdLookup->end() )
    {
      throw std::out_of_range( "Audio sample type does not match a Python data type." );
    }
    return std::get<0>( *findIt );
  },
  "Convert a VISR audio sample type into the corresponding Numpy dtype."
  );

  m.def( "dtypeToAudioSampleType", [sampleTypeIdLookup]( py::dtype pyType )
    {
      auto const findIt = std::find_if( sampleTypeIdLookup->begin(), sampleTypeIdLookup->end(),
        [pyType]( AudioSampleTypeLookup::value_type const & v ) -> bool
        {
          return pyType.is(std::get<0>( v ));
        } );
      if( findIt == sampleTypeIdLookup->end() )
      {
        throw std::out_of_range( "Python dtype does not match a registered audio sample type." );
      }
      return std::get<1>(*findIt);
  },
  "Convert a Numpy dtype into the corrponding VISR audio sample type id."
  );
}

} // namepace visr
} // namespace python
} // namespace visr
