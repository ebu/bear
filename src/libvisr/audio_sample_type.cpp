/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "audio_sample_type.hpp"

#include <cstdint>
#include <initializer_list>
#include <map>
#include <utility>

namespace visr
{
namespace AudioSampleType
{

std::size_t typeSize( Id id )
{
  using TypeSizeMap = std::map<Id, std::size_t>;
  // static object is constructed on first use.
  static TypeSizeMap sSizeLookup{ 
    {floatId, sizeof(float)},
    {doubleId, sizeof( double ) },
    {longDoubleId, sizeof( long double )},
    {uint8Id, sizeof( uint8_t )},
    {int8Id, sizeof( int8_t ) },
    {uint16Id, sizeof( uint16_t) },
    {int16Id, sizeof( int16_t ) },
    {uint32Id, sizeof( uint32_t ) },
    {int32Id, sizeof( int32_t ) },
    {complexFloatId, sizeof( std::complex<float> ) },
    {complexDoubleId, sizeof( std::complex<double> ) }
  }; 
  return sSizeLookup.at(id);
}

} // namespace AudioSampleType
} // namespace visr
