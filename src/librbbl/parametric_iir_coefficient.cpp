/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "parametric_iir_coefficient.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <cmath>
#include <istream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace visr
{
namespace rbbl
{

class ParametricIirCoefficientBase::TypeIdTranslator
{
private:
  using ForwardMap = std::map<Type, std::string>;
  using ReverseMap = std::map<std::string, Type >;

  ForwardMap mFwdMap;

  ReverseMap mReverseMap;

public:
  TypeIdTranslator();

  /**
   * @throw std::invalid_argument id id is not a valid typename.
   */
  Type idToType( const std::string & id ) const
  {
    ReverseMap::const_iterator findIt = mReverseMap.find( id );
    if( findIt == mReverseMap.end() )
    {
      throw std::invalid_argument( "Invalid IIR parameter string type" );
    }
    return findIt->second;
  }

  std::string const & typeToId( Type typeStr ) const
  {
    ForwardMap::const_iterator findIt = mFwdMap.find( typeStr );
    if( findIt == mFwdMap.end() )
    {
      // Should not happen because of enum class 
      throw std::invalid_argument( "Invalid IIR parameter type." );
    }
    return findIt->second;
  }
};

  ParametricIirCoefficientBase::TypeIdTranslator::TypeIdTranslator()
    : mFwdMap( { { Type::lowpass, "lowpass" },{ Type::highpass, "highpass" },
    { Type::bandpass, "bandpass" },{ Type::bandstop, "bandstop" },
    { Type::peak, "peak" },
    { Type::lowshelf, "lowshelf" },{ Type::highshelf, "highshelf" },
    { Type::allpass, "allpass" } } )
  {
    for( ForwardMap::value_type const & v : mFwdMap )
    {
      mReverseMap.insert( std::make_pair( v.second, v.first ) );
    }
    // Todo: Add abbreviations
  }

  /*static*/ ParametricIirCoefficientBase::TypeIdTranslator const &
    ParametricIirCoefficientBase::translator()
  {
    static const TypeIdTranslator sTranslator;
    return sTranslator;
  }

  /*static*/ std::string const &
    ParametricIirCoefficientBase::typeIdToString( ParametricIirCoefficientBase::Type typeId )
  {
    return translator().typeToId( typeId );
  }

  /*static*/ ParametricIirCoefficientBase::Type
    ParametricIirCoefficientBase::stringToTypeId( std::string const & name )
  {
    return translator().idToType( name );
  }


template< typename CoefficientType >
ParametricIirCoefficient< CoefficientType >::ParametricIirCoefficient()
  : mType( Type::allpass )
  , mFrequency( static_cast<CoefficientType>(1.0) )
  , mQuality( static_cast<CoefficientType>(1.0) )
  , mGain( static_cast<CoefficientType>(0.0) )
{
}

template< typename CoefficientType >
ParametricIirCoefficient< CoefficientType >::ParametricIirCoefficient( ParametricIirCoefficient const & rhs ) = default;

template< typename CoefficientType >
ParametricIirCoefficient< CoefficientType >::ParametricIirCoefficient( ParametricIirCoefficient && rhs ) = default;

template< typename CoefficientType >
ParametricIirCoefficient< CoefficientType >::ParametricIirCoefficient( Type typeId,
                                                                       CoefficientType centerFrequency,
                                                                       CoefficientType quality,
                                                                       CoefficientType gain )
 : mType( typeId )
 , mFrequency( centerFrequency )
 , mQuality( quality )
 , mGain( gain )
{
}

template< typename CoefficientType > 
void ParametricIirCoefficient< CoefficientType >::setType( Type newType )
{
  mType = newType;
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::setFrequency( CoefficientType newFrequency )
{
  mFrequency = newFrequency;
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::setQuality( CoefficientType newQuality )
{
  mQuality = newQuality;
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::setGain( CoefficientType newGain )
{
  mGain = newGain;
}

template< typename CoefficientType >
/*static*/ ParametricIirCoefficient< CoefficientType >
ParametricIirCoefficient< CoefficientType >::fromJson( boost::property_tree::ptree const & tree )
{
  ParametricIirCoefficient< CoefficientType > res;
  res.loadJson( tree );
  return res;
}

template< typename CoefficientType >
/*static*/ ParametricIirCoefficient< CoefficientType >
ParametricIirCoefficient< CoefficientType >::fromJson( std::basic_istream<char> & stream )
{
  ParametricIirCoefficient< CoefficientType > res;
  res.loadJson( stream );
  return res;
}

template< typename CoefficientType >
/*static*/ ParametricIirCoefficient< CoefficientType >
ParametricIirCoefficient< CoefficientType >::fromJson( std::string const & str )
{
  ParametricIirCoefficient< CoefficientType > res;
  res.loadJson( str );
  return res;
}

template< typename CoefficientType >
/*static*/ ParametricIirCoefficient< CoefficientType >
ParametricIirCoefficient< CoefficientType >::fromXml( boost::property_tree::ptree const & tree )
{
  ParametricIirCoefficient< CoefficientType > res;
  res.loadXml( tree );
  return res;
}

template< typename CoefficientType >
/*static*/ ParametricIirCoefficient< CoefficientType >
ParametricIirCoefficient< CoefficientType >::fromXml( std::basic_istream<char> & stream )
{
  ParametricIirCoefficient< CoefficientType > res;
  res.loadXml( stream );
  return res;
}

template< typename CoefficientType >
/*static*/ ParametricIirCoefficient< CoefficientType >
ParametricIirCoefficient< CoefficientType >::fromXml( std::string const & str )
{
  ParametricIirCoefficient< CoefficientType > res;
  res.loadXml( str );
  return res;
}


template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::loadJson( boost::property_tree::ptree const & tree )
{
  std::string const typeStr = tree.get<std::string>( "type" );
  CoefficientType const frequency = tree.get<CoefficientType>( "f" );
  CoefficientType const quality = tree.get<CoefficientType>( "q" );
  boost::optional<CoefficientType> const gainOpt = tree.get_optional<CoefficientType>( "gain" );
  Type const typeId = stringToTypeId( typeStr ); // might throw
  setType( typeId );
  setFrequency( frequency );
  setQuality( quality );
  setGain( gainOpt ? *gainOpt : static_cast<CoefficientType >(0.0) );
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::loadJson( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a json ParametricIirCoefficient node: " ) + ex.what( ) );
  }
  loadJson( tree );
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::loadJson( std::string const & str )
{
  std::stringstream stream(str);
  loadJson( stream );
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::loadXml( boost::property_tree::ptree const & tree )
{
  boost::property_tree::write_xml( std::cout, tree );

  std::string const & typeStr = tree.get<std::string>( "<xmlattr>.type" );
  Type const typeId  = stringToTypeId( typeStr );
  CoefficientType const frequency = tree.get<CoefficientType>( "<xmlattr>.f" );
  CoefficientType const quality = tree.get<CoefficientType>( "<xmlattr>.q" );
  boost::optional<CoefficientType> const gainOpt = tree.get_optional<CoefficientType>( "<xmlattr>.gain" );
  setType( typeId );
  setFrequency( frequency );
  setQuality( quality );
  setGain( gainOpt ? *gainOpt : static_cast<CoefficientType>(0.0) );
}

template< typename CoefficientType >
void ParametricIirCoefficient<CoefficientType>::loadXml( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_xml( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a XML ParametricIirCoefficient node: " ) + ex.what( ) );
  }
  loadXml( tree );
}

template< typename CoefficientType >
void ParametricIirCoefficient<CoefficientType>::loadXml( std::string const & str )
{
  boost::property_tree::ptree tree;
  try
  {
    read_xml( str, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a XML ParametricIirCoefficient node: " ) + ex.what() );
  }
  loadXml( tree );
}

template< typename CoefficientType >
void ParametricIirCoefficient<CoefficientType>::writeJson( boost::property_tree::ptree & tree ) const
{
  tree.put( "type", typeIdToString( type() ) );
  tree.put( "f", frequency() );
  tree.put( "q", quality( ) );
  if( std::abs( gain() ) >= std::numeric_limits<CoefficientType>::epsilon() )
  {
    tree.put( "gain", gain() );
  }
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::writeJson( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeJson( tree );
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::writeJson( std::string & str ) const
{
  std::stringstream stream;
  writeJson( stream );
  str = stream.str();
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::writeXml( boost::property_tree::ptree & tree ) const
{
  tree.put( "<xmlattr>.type", typeIdToString( type() ) );
  tree.put( "<xmlattr>.f", frequency() );
  tree.put( "<xmlattr>.q", quality() );
  if( std::abs( gain() ) >= std::numeric_limits<CoefficientType>::epsilon() )
  {
    tree.put( "<xmlattr>.gain", gain() );
  }
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::writeXml( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree tree;
  writeXml( tree );
  boost::property_tree::write_xml( stream, tree );
}

template< typename CoefficientType >
void ParametricIirCoefficient< CoefficientType >::writeXml( std::string & str ) const
{
  std::stringstream stream;
  writeXml( stream );
  str = stream.str();
}

// Explicit instantiations
template class ParametricIirCoefficient< float >;
template class ParametricIirCoefficient< double >;

///////////////////////////////////////////////////////////////////////////////

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::loadJson( boost::property_tree::ptree const & tree )
{
  Container newCoeffs;
  newCoeffs.reserve( tree.size() );
  for( auto node : tree )
  {
    Element newEl = Element::fromJson( node.second );
    newCoeffs.push_back( std::move(newEl) );
  }
  mCoeffs.swap( newCoeffs );
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::loadJson( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_json( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing a json ParametricIirCoefficientList node: " ) + ex.what() );
  }
  loadJson( tree );
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::loadJson( std::string const & str )
{
  std::stringstream stream( str );
  loadJson( stream );
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::loadXml( boost::property_tree::ptree const & tree )
{
  Container newCoeffs;
  auto eqNodes = tree.equal_range( "eq" );
  newCoeffs.reserve( std::distance( eqNodes.first, eqNodes.second ) );
  for( auto eqNodeIt( eqNodes.first ); eqNodeIt != eqNodes.second; ++eqNodeIt )
  {
    Element newEl = Element::fromXml( eqNodeIt->second );
    newCoeffs.push_back( std::move( newEl ) );
  }
  mCoeffs.swap( newCoeffs );
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::loadXml( std::basic_istream<char> & stream )
{
  boost::property_tree::ptree tree;
  try
  {
    read_xml( stream, tree );
  }
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing an Xml ParametricIirCoefficientList node: " ) + ex.what() );
  }
  loadXml( tree );
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::loadXml( std::string const & str )
{
  std::stringstream stream( str );
  loadXml( stream );
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::writeJson( boost::property_tree::ptree & tree ) const
{
  for( Element const & el : *this )
  {
    boost::property_tree::ptree child;
    el.writeJson( child );
    tree.add_child("", child ); // Adding unnamed childs creates a JSON array.
  }
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::writeJson( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree node;
  writeJson( node );
  try
  {
    boost::property_tree::write_json( stream, node );
  }
  catch( std::exception const & ex )
  {
    throw( std::invalid_argument( std::string("Serialisation of EQ parameters failed: ") + ex.what() ) );
  }
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::writeJson( std::string & str ) const
{
  std::stringstream stream;
  writeJson( stream );
  str = stream.str();
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::writeXml( boost::property_tree::ptree & tree ) const
{
  for( Element const & el : *this )
  {
    boost::property_tree::ptree child;
    el.writeJson( child );
    tree.add_child( "eq", child ); // XML needs named nodes
  }
}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::writeXml( std::basic_ostream<char> & stream ) const
{
  boost::property_tree::ptree node;
  writeXml( node );
  try
  {
    boost::property_tree::write_xml( stream, node );
  }
  catch( std::exception const & ex )
  {
    throw(std::invalid_argument( std::string("Serialisation of EQ parameters failed: ") + ex.what() ));
  }

}

template< typename CoefficientType >
void ParametricIirCoefficientList< CoefficientType >::writeXml( std::string & str ) const
{
  std::stringstream stream;
  writeXml( stream );
  str = stream.str();
}

// Explicit instantiations
template class ParametricIirCoefficientList< float >;
template class ParametricIirCoefficientList< double >;


} // namespace rbbl
} // namespace visr
