/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "filter_routing.hpp"

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

#ifndef _MSC_VER
  /**
  * Provide definition for the static const class member in order to allow their address to be taken.
  * The value is taken from their declaration within the class.
  * @note: Microsoft Visual Studio neither allows or requires this standard-compliant explicit definition.
  */
  /*static*/ const FilterRouting::IndexType FilterRouting::cInvalidIndex;
#endif

FilterRoutingList::FilterRoutingList( std::initializer_list<FilterRouting> const & entries )
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

/*static*/ FilterRoutingList const FilterRoutingList::fromJson( std::string const & initString )
{
  std::stringstream stream( initString );
  return FilterRoutingList::fromJson( stream );
}

/*static*/ FilterRoutingList const FilterRoutingList::fromJson( std::istream & initStream )
{
  FilterRoutingList newList;

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
    throw std::invalid_argument( std::string( "FilterRoutingList::parseJson(): Error while reading JSON data:" ) + ex.what( ) );
  }
  // boost::property_map translates elements of arrays into children with empty names.
  for( auto v : propTree.get_child( "" ) )
  {
    ptree const & routingNode = v.second;
    std::string const inIdxStr = routingNode.get<std::string>( "input" );
    std::string const outIdxStr = routingNode.get<std::string>( "output" );
    std::string const filterIdxStr = routingNode.get<std::string>( "filter" );
    std::string const gain = routingNode.get<std::string>( "gain", "1.0" );

    IndexSequence const inIndices( inIdxStr );
    IndexSequence const outIndices( outIdxStr );
    IndexSequence const filterIndices( filterIdxStr );
    FloatSequence<FilterRouting::GainType> const gainVector( gain );

    std::size_t numEntries = inIndices.size( );
    if( outIndices.size( ) != numEntries )
    {
      if( numEntries == 1 )
      {
        numEntries = outIndices.size( );
      }
      else if( outIndices.size( ) != 1 )
      {
        throw std::invalid_argument( "FilterRoutingList:parseJson: All non-scalar entries must have the same number of indices" );
      }
    }
    // Special case: If both input and output indices are scalar, the filter cpec cannot be non-scalar, because this would mean multiple
    // routings for the same (input,output) combination.
    if( filterIndices.size( ) != numEntries )
    {
      if( (numEntries == 1) and( filterIndices.size( ) != 1 ) )
      {
        throw std::invalid_argument( "FilterRoutingList:parseJson: All non-scalar entries must have the same number of indices" );
      }
    }
    // The gainVector entry cannot change the width of the filter routing definition, because that would mean that multiple
    // routings with the same {in,out,filter} but potentially different gains would be created.
    if( (gainVector.size( ) != 1) and( gainVector.size( ) != numEntries ) )
    {
      throw std::invalid_argument( "FilterRoutingList:parseJson: If the \"input\" and/or the \"output\" entry of a routing is non-scalar, the \"gain\" entry must have the same number of indices" );
    }
    for( std::size_t entryIdx( 0 ); entryIdx < numEntries; ++entryIdx )
    {
      FilterRouting::IndexType const inIdx = inIndices.size( ) == 1 ? inIndices[0] : inIndices[entryIdx];
      FilterRouting::IndexType const outIdx = outIndices.size( ) == 1 ? outIndices[0] : outIndices[entryIdx];
      FilterRouting::IndexType const filterIdx = filterIndices.size( ) == 1 ? filterIndices[0] : filterIndices[entryIdx];
      FilterRouting::GainType const gain = gainVector.size( ) == 1 ? gainVector[0] : gainVector[entryIdx];
      newList.addRouting( inIdx, outIdx, filterIdx, gain );
    }
  }
  return newList;
}

void FilterRoutingList::swap( FilterRoutingList& rhs )
{
  mRoutings.swap( rhs.mRoutings );
}

void FilterRoutingList::addRouting( FilterRouting const & newEntry )
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
    throw std::logic_error( "FilterRoutingList::addRouting(): Error inserting element: " );
  }
}

bool FilterRoutingList::removeRouting( FilterRouting const & entry )
{
  RoutingsType::const_iterator const findIt = mRoutings.find( entry );
  if( findIt != mRoutings.end() )
  {
    mRoutings.erase( findIt );
    return true;
  }
  return false;
}

bool FilterRoutingList::removeRouting( FilterRouting::IndexType inputIdx, FilterRouting::IndexType outputIdx )
{
  RoutingsType::const_iterator const findIt = mRoutings.find( FilterRouting( inputIdx, outputIdx, FilterRouting::cInvalidIndex ) );
  if( findIt != mRoutings.end() )
  {
    mRoutings.erase( findIt );
    return true;
  }
  return false;
}

void FilterRoutingList::parseJson( std::string const & encoded )
{
  std::stringstream strStr( encoded );
  parseJson( strStr );
}

void FilterRoutingList::parseJson( std::istream & encoded )
{
  FilterRoutingList newList = FilterRoutingList::fromJson( encoded );
  swap( newList ); // Ensure strong exception safety.
}

} // namespace rbbl
} // namespace visr
