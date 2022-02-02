/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "biquad_coefficient.hpp"

#include <libvisr/detail/compose_message_string.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <cassert>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>

namespace visr
{
namespace rbbl
{

template< typename CoeffType >
/*static*/ BiquadCoefficient<CoeffType>
BiquadCoefficient<CoeffType>::fromJson( boost::property_tree::ptree const & tree )
{
  BiquadCoefficient<CoeffType> res;
  res.loadJson( tree );
  return res;
}

template< typename CoeffType >
/*static*/ BiquadCoefficient<CoeffType>
BiquadCoefficient<CoeffType>::fromJson( std::basic_istream<char> & stream )
{
  BiquadCoefficient<CoeffType> res;
  res.loadJson( stream );
  return res;
}

template< typename CoeffType >
/*static*/ BiquadCoefficient<CoeffType>
BiquadCoefficient<CoeffType>::fromJson( std::string const & str )
{
  BiquadCoefficient<CoeffType> ret;
  ret.loadJson( str );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficient<CoeffType>
BiquadCoefficient<CoeffType>::fromXml( boost::property_tree::ptree const & tree )
{
  BiquadCoefficient<CoeffType> res;
  res.loadXml( tree );
  return res;
}

template< typename CoeffType >
/*static*/ BiquadCoefficient<CoeffType>
BiquadCoefficient<CoeffType>::fromXml( std::basic_istream<char> & stream )
{
  BiquadCoefficient<CoeffType> res;
  res.loadXml( stream );
  return res;
}

template< typename CoeffType >
/*static*/ BiquadCoefficient<CoeffType>
BiquadCoefficient<CoeffType>::fromXml( std::string const & str )
{
  BiquadCoefficient<CoeffType> ret;
  ret.loadXml( str );
  return ret;
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::loadJson( boost::property_tree::ptree const & tree )
{
  // boost::property_tree::ptree const biquadTree = tree.get_child( "biquad" );

  CoeffType const a0 = tree.get<CoeffType>( "a0", static_cast<CoeffType>(1.0f) );
  at( 0 ) = tree.get<CoeffType>( "b0" ) / a0;
  at( 1 ) = tree.get<CoeffType>( "b1" ) / a0;
  at( 2 ) = tree.get<CoeffType>( "b2" ) / a0;
  at( 3 ) = tree.get<CoeffType>( "a1" ) / a0;
  at( 4 ) = tree.get<CoeffType>( "a2" ) / a0;
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::loadJson( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( detail::composeMessageString( "Error while parsing a json BiquadCoefficient node: " , ex.what( )) );
  }
  loadJson( tree );
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::loadJson( std::string const & str )
{
  std::stringstream stream( str );
  loadJson( stream );
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::loadXml( boost::property_tree::ptree const & tree )
{
  // Hack: Depending from where this method is called, we don't know
  boost::property_tree::ptree const biquadTree = tree.count( "biquad" ) == 0
    ? tree : tree.get_child( "biquad" );

  boost::optional<CoeffType> const a0Optional = biquadTree.get_optional<CoeffType>( "<xmlattr>.a0" );
  CoeffType const a0 = a0Optional ? *a0Optional : static_cast<CoeffType>(1.0f);
  CoeffType const a1 = biquadTree.get<CoeffType>( "<xmlattr>.a1" );
  CoeffType const a2 = biquadTree.get<CoeffType>( "<xmlattr>.a2" );
  CoeffType const b0 = biquadTree.get<CoeffType>( "<xmlattr>.b0" );
  CoeffType const b1 = biquadTree.get<CoeffType>( "<xmlattr>.b1" );
  CoeffType const b2 = biquadTree.get<CoeffType>( "<xmlattr>.b2" );

  at( 0 ) = b0 / a0;
  at( 1 ) = b1 / a0;
  at( 2 ) = b2 / a0;
  at( 3 ) = a1 / a0;
  at( 4 ) = a2 / a0;
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::loadXml( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_xml( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a XML BiquadCoefficient node: " ) + ex.what( ) );
  }
  loadXml( tree );
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::loadXml( std::string const & str )
{
  std::stringstream stream( str );
  loadXml( stream );
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::writeJson( boost::property_tree::ptree & tree ) const
{
  tree.put( "b0", b0() );
  tree.put( "b1", b1() );
  tree.put( "b2", b2() );
  tree.put( "a1", a1() );
  tree.put( "a2", a2() );
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::writeJson( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeJson( tree );
  try
  {
    write_json( stream, tree);
  } catch (std::exception const & ex)
  {
    throw( detail::composeMessageString( "Error while writing BiquadCoefficient message to JSON: ", ex.what() ) );
  }
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::writeJson( std::string & str ) const
{
  std::stringstream stream;
  writeJson( stream );
  str = stream.str();
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::writeXml( boost::property_tree::ptree & tree ) const
{
  // TODO: implement me!
  assert( false );
}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::writeXml( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeJson( tree );

}

template< typename CoeffType >
void BiquadCoefficient<CoeffType>::writeXml( std::string & str ) const
{
}

///////////////////////////////////////////////////////////////////////////////

template< typename CoeffType >
/*static*/ BiquadCoefficientList<CoeffType>
BiquadCoefficientList<CoeffType>::fromJson( boost::property_tree::ptree const & tree )
{
  BiquadCoefficientList<CoeffType> ret;
  ret.loadJson( tree );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientList<CoeffType>
BiquadCoefficientList<CoeffType>::fromJson( std::basic_istream<char> & stream )
{
  BiquadCoefficientList<CoeffType> ret;
  ret.loadJson( stream );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientList<CoeffType>
BiquadCoefficientList<CoeffType>::fromJson( std::string const & str )
{
  BiquadCoefficientList<CoeffType> ret;
  ret.loadJson( str );
  return ret;

}

template< typename CoeffType >
/*static*/ BiquadCoefficientList<CoeffType>
BiquadCoefficientList<CoeffType>::fromXml( boost::property_tree::ptree const & tree )
{
  BiquadCoefficientList<CoeffType> ret;
  ret.loadXml( tree );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientList<CoeffType>
BiquadCoefficientList<CoeffType>::fromXml( std::basic_istream<char> & stream )
{
  BiquadCoefficientList<CoeffType> ret;
  ret.loadXml( stream );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientList<CoeffType>
BiquadCoefficientList<CoeffType>::fromXml( std::string const & str )
{
  BiquadCoefficientList<CoeffType> ret;
  ret.loadXml( str );
  return ret;
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::loadJson( boost::property_tree::ptree const & tree )
{
  auto const biquadNodes = tree.equal_range( "" );
  std::size_t const numBiquads = std::distance( biquadNodes.first, biquadNodes.second );
  resize( numBiquads );
  std::size_t biqIndex = 0;
  for( boost::property_tree::ptree::const_assoc_iterator treeIt( biquadNodes.first ); treeIt != biquadNodes.second; ++treeIt, ++biqIndex )
  {
    boost::property_tree::ptree const childTree = treeIt->second;
    at( biqIndex ).loadJson( childTree );
  }
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::loadJson( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a JSON BiquadCoefficientList representation: " ) + ex.what( ) );
  }
  loadJson( tree );
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::loadJson( std::string const & str )
{
  std::stringstream stream( str );
  loadJson( stream );
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::loadXml( boost::property_tree::ptree const & tree )
{
  // Hack: Depending from where this method is called, we don't know
  boost::property_tree::ptree const specNode = tree.count( "filterSpec" ) == 0
    ? tree : tree.get_child( "filterSpec" );

  auto const biquadNodes = specNode.equal_range( "biquad" );
  std::size_t const numBiquads = std::distance( biquadNodes.first, biquadNodes.second );
  resize( numBiquads );
  std::size_t biqIndex = 0;
  for( boost::property_tree::ptree::const_assoc_iterator treeIt( biquadNodes.first ); treeIt != biquadNodes.second; ++treeIt, ++biqIndex )
  {
    boost::property_tree::ptree const childTree = treeIt->second;
    at( biqIndex ).loadXml( childTree );
  }
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::loadXml( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_xml( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a XML BiquadCoefficient node: " ) + ex.what( ) );
  }
  loadXml( tree );
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::loadXml( std::string const & str )
{
  std::stringstream stream( str );
  loadXml( stream );
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::writeJson( boost::property_tree::ptree & tree ) const
{
  for( std::size_t idx( 0 ); idx < size(); ++idx )
  {
    boost::property_tree::ptree child;
    at( idx ).writeJson( child );
    tree.push_back( std::make_pair( "", child ) );
  }
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::writeJson( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeJson( tree );
  try
  {
    write_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while writing a BiquadCoefficientList to JSON: " ) + ex.what( ) );
  }
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::writeJson( std::string & str ) const
{
  std::stringstream stream;
  writeJson( stream );
  str = stream.str();
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::writeXml( boost::property_tree::ptree & tree ) const
{
  // TODO: implement me!
  assert( false );
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::writeXml( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeXml( tree );
  try
  {
    write_xml( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while writing BiquadCoefficientList to XML: " ) + ex.what( ) );
  }
}

template< typename CoeffType >
void BiquadCoefficientList<CoeffType>::writeXml( std::string & str ) const
{
  std::stringstream stream;
  writeXml( stream );
  str = stream.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename CoeffType>
BiquadCoefficientMatrix<CoeffType>::BiquadCoefficientMatrix( std::size_t numberOfFilters, std::size_t numberOfBiquads )
{
  resize( numberOfFilters, numberOfBiquads );
}

template<typename CoeffType>
BiquadCoefficientMatrix<CoeffType>::
BiquadCoefficientMatrix( std::initializer_list<
                                  std::initializer_list<
                                  BiquadCoefficient<CoeffType> > > const & initList )
{
  std::size_t const numRows{ initList.size() };
  if( numRows > 0 )
  {
    auto rowIt{ initList.begin() };
    assert( rowIt != initList.end() );
    std::size_t numCols{ rowIt->size() };
    resize( numRows, numCols );
    for( std::size_t rowIdx{0}; rowIt != initList.end(); ++rowIt, ++rowIdx )
    {
      if( rowIt->size() != numCols )
      {
        throw std::invalid_argument( "Matrix row sizes are inconsistent." );
      }
      std::size_t colIdx{0};
      for( auto colIt{ rowIt->begin() }; colIt != rowIt->end(); ++ colIt, ++colIdx )
      {
        operator()( rowIdx, colIdx ) = * colIt;
      }
    }
  }
  else
  {
    resize( 0, 0 );
  }
}

template<typename CoeffType>
BiquadCoefficientMatrix<CoeffType>::~BiquadCoefficientMatrix() = default;

template<typename CoeffType>
void BiquadCoefficientMatrix<CoeffType>::resize( std::size_t numberOfFilters, std::size_t numberOfBiquads )
{
  mRows.clear(); // Needed to trigger the re-initialisation of all rows even if the size decreases or stayed constant.
  // Define the initial entry for all filter rows (numberOfBiquads 'default' biquad specs)
  BiquadCoefficientList<CoeffType> templ( numberOfBiquads );
  mRows.resize( numberOfFilters, templ );
}

template<typename CoeffType>
void BiquadCoefficientMatrix<CoeffType>::setFilter( std::size_t filterIdx, BiquadCoefficientList<CoeffType> const & newFilter )
{
  if( filterIdx >= numberOfFilters() )
  {
    throw std::out_of_range( "BiquadCoefficientMatrix::setFilter(): Filter index exceeds maximum admissible value." );
  }
  if( newFilter.size() > numberOfSections() )
  {
    throw std::invalid_argument( "BiquadCoefficientMatrix::setFilter(): The number index exceeds maximum admissible value." );
  }
  BiquadCoefficientList<CoeffType> & row = mRows[filterIdx];
  std::copy( &newFilter[0], &newFilter[0] + newFilter.size(), &row[0] );
  std::fill_n( &row[0] + newFilter.size(),
    numberOfSections()
    - newFilter.size(), BiquadCoefficient<CoeffType>() );
}

template< typename CoeffType >
/*static*/ BiquadCoefficientMatrix<CoeffType>
BiquadCoefficientMatrix<CoeffType>::fromJson( boost::property_tree::ptree const & tree )
{
  BiquadCoefficientMatrix<CoeffType> ret{0,0};
  ret.loadJson( tree );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientMatrix<CoeffType>
BiquadCoefficientMatrix<CoeffType>::fromJson( std::basic_istream<char> & stream )
{
  BiquadCoefficientMatrix<CoeffType> ret{0,0};
  ret.loadJson( stream );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientMatrix<CoeffType>
BiquadCoefficientMatrix<CoeffType>::fromJson( std::string const & str )
{
  BiquadCoefficientMatrix<CoeffType> ret{0,0};
  ret.loadJson( str );
  return ret;

}

template< typename CoeffType >
/*static*/ BiquadCoefficientMatrix<CoeffType>
BiquadCoefficientMatrix<CoeffType>::fromXml( boost::property_tree::ptree const & tree )
{
  BiquadCoefficientMatrix<CoeffType> ret{0,0};
  ret.loadXml( tree );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientMatrix<CoeffType>
BiquadCoefficientMatrix<CoeffType>::fromXml( std::basic_istream<char> & stream )
{
  BiquadCoefficientMatrix<CoeffType> ret{0,0};
  ret.loadXml( stream );
  return ret;
}

template< typename CoeffType >
/*static*/ BiquadCoefficientMatrix<CoeffType>
BiquadCoefficientMatrix<CoeffType>::fromXml( std::string const & str )
{
  BiquadCoefficientMatrix<CoeffType> ret{0,0};
  ret.loadXml( str );
  return ret;
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::loadJson( boost::property_tree::ptree const & tree )
{
  auto const channelNodes = tree.equal_range( "" );
  std::size_t const numChannels = std::distance( channelNodes.first, channelNodes.second );
  if( numChannels == 0 )
  {
    resize( 0, 0 ); // return empty matrix
  }
  std::size_t static const unInitMarker = std::numeric_limits<std::size_t>::max();
  std::size_t numBiquads = unInitMarker;
  std::size_t rowIdx{ 0 };
  for( boost::property_tree::ptree::const_assoc_iterator channelIt( channelNodes.first ); channelIt != channelNodes.second; ++channelIt, ++rowIdx )
  {
    boost::property_tree::ptree const channelTree = channelIt->second;
    BiquadCoefficientList<CoeffType> const rowFilters = BiquadCoefficientList<CoeffType>::fromJson( channelTree );
    if( rowIdx == 0 )
    {
      numBiquads = rowFilters.size();
      resize( numChannels, numBiquads );
    }
    assert( numBiquads != unInitMarker ); // Check that initialisation is performed during the first run.
    if( rowFilters.size() != numBiquads )
    {
      std::invalid_argument( "BiquadCoefficientMatrix::loadJSON(): Encountered matrix rows with differing sizes." );
    }
    setFilter( rowIdx, rowFilters );
  }
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::loadJson( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a JSON BiquadCoefficientMatrix representation: " ) + ex.what() );
  }
  loadJson( tree );
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::loadJson( std::string const & str )
{
  std::stringstream stream( str );
  loadJson( stream );
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::loadXml( boost::property_tree::ptree const & tree )
{
#if 0
  // Hack: Depending from where this method is called, we don't know
  boost::property_tree::ptree const specNode = tree.count( "filterSpec" ) == 0
    ? tree : tree.get_child( "filterSpec" );

  auto const biquadNodes = specNode.equal_range( "biquad" );
  std::size_t const numBiquads = std::distance( biquadNodes.first, biquadNodes.second );
  resize( numBiquads );
  std::size_t biqIndex = 0;
  for( boost::property_tree::ptree::const_assoc_iterator treeIt( biquadNodes.first ); treeIt != biquadNodes.second; ++treeIt, ++biqIndex )
  {
    boost::property_tree::ptree const childTree = treeIt->second;
    at( biqIndex ).loadXml( childTree );
  }
#endif
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::loadXml( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_xml( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a XML BiquadCoefficient node: " ) + ex.what() );
  }
  loadXml( tree );
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::loadXml( std::string const & str )
{
  std::stringstream stream( str );
  loadXml( stream );
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::writeJson( boost::property_tree::ptree & tree ) const
{
#if 0
  for( std::size_t idx( 0 ); idx < size(); ++idx )
  {
    boost::property_tree::ptree child;
    at( idx ).writeJson( child );
    tree.push_back( std::make_pair( "", child ) );
  }
#endif
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::writeJson( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeJson( tree );
  try
  {
    write_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while writing a BiquadCoefficientMatrix to JSON: " ) + ex.what() );
  }
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::writeJson( std::string & str ) const
{
  std::stringstream stream;
  writeJson( stream );
  str = stream.str();
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::writeXml( boost::property_tree::ptree & tree ) const
{
  // TODO: implement me!
  assert( false );
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::writeXml( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeXml( tree );
  try
  {
    write_xml( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while writing BiquadCoefficientMatrix to XML: " ) + ex.what() );
  }
}

template< typename CoeffType >
void BiquadCoefficientMatrix<CoeffType>::writeXml( std::string & str ) const
{
  std::stringstream stream;
  writeXml( stream );
  str = stream.str();
}

// Explicit instantiations
template class BiquadCoefficient<float>;
template class BiquadCoefficientList<float>;
template class BiquadCoefficientMatrix<float>;

template class BiquadCoefficient<double>;
template class BiquadCoefficientList<double>;
template class BiquadCoefficientMatrix<double>;

} // namespace rbbl
} // namespace visr
