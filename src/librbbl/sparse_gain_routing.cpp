/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "sparse_gain_routing.hpp"

#include <librbbl/float_sequence.hpp>
#include <librbbl/index_sequence.hpp>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <algorithm>
#include <ciso646>
#include <sstream>
#include <vector>

namespace visr
{
namespace rbbl
{

SparseGainRoutingList::SparseGainRoutingList() = default;

SparseGainRoutingList::SparseGainRoutingList( std::initializer_list<SparseGainRouting> const & entries )
{
  for( auto e : entries )
  {
    bool result;
    std::tie( std::ignore, result ) = mRoutings.insert( e );
    if( not result )
    {
      throw std::invalid_argument( "Duplicated output indices are not allowed." );
    }
  }
}

SparseGainRoutingList::SparseGainRoutingList( const SparseGainRoutingList & rhs ) = default;

SparseGainRoutingList& SparseGainRoutingList::operator=( const SparseGainRoutingList & rhs ) = default;


/*static*/ SparseGainRoutingList const SparseGainRoutingList::fromJson( std::string const & initString )
{
  std::stringstream stream( initString );
  return SparseGainRoutingList::fromJson( stream );
}

/*static*/ SparseGainRoutingList const SparseGainRoutingList::fromJson( std::istream & initStream )
{
  SparseGainRoutingList newList;

  using ptree = boost::property_tree::ptree;
  using rbbl::IndexSequence;
  using rbbl::FloatSequence;

  ptree propTree;
  try
  {
    read_json( initStream, propTree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "SparseGainRoutingList::parseJson(): Error while reading JSON data:" ) + ex.what( ) );
  }
  // boost::property_map translates elements of arrays into children with empty names.
  for( auto v : propTree.get_child( "" ) )
  {
    ptree const & routingNode = v.second;
    std::string const entryIdxStr = routingNode.get<std::string>( "index" );
    std::string const rowIdxStr = routingNode.get<std::string>( "row" );
    std::string const columnIdxStr = routingNode.get<std::string>( "column" );
    std::string const gain = routingNode.get<std::string>( "gain" );

    IndexSequence const entryIndices( entryIdxStr );
    IndexSequence const rowIndices( rowIdxStr );
    IndexSequence const columnIndices( columnIdxStr );
    FloatSequence<SparseGainRouting::GainType> const gainVector( gain );

    std::size_t numEntries = entryIndices.size( );
    if( rowIndices.size( ) != numEntries )
    {
      if( numEntries == 1 )
      {
        numEntries = entryIndices.size( );
      }
      else if( rowIndices.size( ) != 1 )
      {
        throw std::invalid_argument( "SparseGainRoutingList:parseJson: All non-scalar entries must have the same number of indices" );
      }
    }
    // Special case: If both input and output indices are scalar, the filter cpec cannot be non-scalar, because this would mean multiple
    // routings for the same (input,output) combination.
    if( columnIndices.size( ) != numEntries )
    {
      if( (numEntries == 1) and( columnIndices.size( ) != 1 ) )
      {
        throw std::invalid_argument( "SparseGainRoutingList:parseJson: All non-scalar entries must have the same number of indices" );
      }
    }
    // The gainVector entry cannot change the width of the filter routing definition, because that would mean that multiple
    // routings with the same {in,out,filter} but potentially different gains would be created.
    if( (gainVector.size( ) != 1) and( gainVector.size( ) != numEntries ) )
    {
      throw std::invalid_argument( "SparseGainRoutingList:parseJson: If the \"input\" and/or the \"output\" entry of a routing is non-scalar, the \"gain\" entry must have the same number of indices" );
    }
    for( std::size_t entryIdx( 0 ); entryIdx < numEntries; ++entryIdx )
    {
      SparseGainRouting::IndexType const entryId = entryIndices[entryIdx];
      SparseGainRouting::IndexType const rowIdx = rowIndices.size( ) == 1 ? rowIndices[0] : rowIndices[entryIdx];
      SparseGainRouting::IndexType const columnIdx = columnIndices.size( ) == 1 ? columnIndices[0] : columnIndices[entryIdx];
      SparseGainRouting::GainType const gain = gainVector.size( ) == 1 ? gainVector[0] : gainVector[entryIdx];
      newList.addRouting( entryId, rowIdx, columnIdx, gain );
    }
  }
  return newList;
}

void SparseGainRoutingList::clear( )
{
  mRoutings.clear();
}

void SparseGainRoutingList::swap( SparseGainRoutingList& rhs )
{
  mRoutings.swap( rhs.mRoutings );
}

void SparseGainRoutingList::addRouting( SparseGainRouting const & newEntry )
{
  RoutingsType::const_iterator const findIt = mRoutings.find( newEntry );
  if( findIt != mRoutings.end() )
  {
    mRoutings.erase( findIt );
  }
  bool res( false );
  std::tie(std::ignore, res ) = mRoutings.insert( newEntry );
  if( not res )
  {
    throw std::logic_error( "SparseGainRoutingList::addRouting(): Error inserting element: " );
  }
}

bool SparseGainRoutingList::removeRouting( SparseGainRouting const & entry )
{
  RoutingsType::const_iterator const findIt = mRoutings.find( entry );
  if( findIt != mRoutings.end() )
  {
    mRoutings.erase( findIt );
    return true;
  }
  return false;
}

bool SparseGainRoutingList::removeRouting( SparseGainRouting::IndexType inputIdx, SparseGainRouting::IndexType outputIdx )
{
  RoutingsType::const_iterator const findIt = mRoutings.find( SparseGainRouting( inputIdx, outputIdx, SparseGainRouting::cInvalidIndex ) );
  if( findIt != mRoutings.end() )
  {
    mRoutings.erase( findIt );
    return true;
  }
  return false;
}

void SparseGainRoutingList::parseJson( std::string const & encoded )
{
  std::stringstream strStr( encoded );
  parseJson( strStr );
}

void SparseGainRoutingList::parseJson( std::istream & encoded )
{
  SparseGainRoutingList newList = SparseGainRoutingList::fromJson( encoded );
  swap( newList ); // Ensure strong exception safety.
}

} // namespace rbbl
} // namespace visr
