/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "object_type.hpp"

#include <map>
#include <stdexcept>
#include <tuple>

namespace visr
{
namespace objectmodel
{

namespace // unnamed
{

using ObjectToString = std::map< ObjectTypeId, std::string >;

using StringToObject = std::map< std::string, ObjectTypeId >;

static const ObjectToString objectToStringLookup =
{
  { ObjectTypeId::PointSource, "point" },
  { ObjectTypeId::PointSourceExtent, "pointextent" },
  { ObjectTypeId::PlaneWave, "plane" },
  { ObjectTypeId::DiffuseSource, "diffuse" },
  { ObjectTypeId::PointSourceWithDiffuseness, "pointdiffuse" },
  { ObjectTypeId::ExtendedSource, "extended" },
  { ObjectTypeId::HoaSource, "hoa" },
  { ObjectTypeId::ChannelObject, "channel" },
  { ObjectTypeId::PointSourceWithReverb, "pointreverb" }
};

// TODO: This would be nice place for the move constructor idiom
// (avoid the copy construction upon returning), although the 
// impact on the execution time is negligible.
StringToObject const createReverseLookup( ObjectToString const & fwdTable )
{
  StringToObject reverseTable;
  bool insertRes;
  for( auto val : fwdTable )
  {
    std::tie( std::ignore, insertRes ) = reverseTable.insert( std::make_pair( val.second, val.first ) );
    if( !insertRes )
    {
      throw std::logic_error( "Internal logic error: type name string duplivated in forward lookup table" );
    }
  }
  return reverseTable;
}

static const StringToObject stringToObjectLookup( createReverseLookup( objectToStringLookup ) );

} // unnamed namespace

const std::string& objectTypeToString( ObjectTypeId type )
{
  auto findIt = objectToStringLookup.find( type );
  if( findIt == objectToStringLookup.end() )
  {
    throw std::logic_error( "Internal logic error: No string representation found for an existing source type." );
  }
  return findIt->second;
}

ObjectTypeId stringToObjectType( std::string const & typeString )
{
  auto findIt = stringToObjectLookup.find( typeString );
  if( findIt == stringToObjectLookup.end( ) )
  {
    throw std::logic_error( " The given string representation does not match an existing source type." );
  }
  return findIt->second;
}

} // namespace objectmodel
} // namespace visr
