//
//  LoudspeakerArray.cpp
//
//  Created by Dylan Menzies on 18/11/2014.
//  Copyright (c) ISVR, University of Southampton. All rights reserved.
//

#include "LoudspeakerArray.h"

#include <libefl/degree_radian_conversion.hpp>
#include <libefl/cartesian_spherical_conversion.hpp>
#include <libefl/db_linear_conversion.hpp>


#include <librbbl/biquad_coefficient.hpp>

#include <librbbl/float_sequence.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/find_format.hpp>
#include <boost/algorithm/string/formatter.hpp>
#include <boost/algorithm/string/classification.hpp>

#ifdef VISR_USE_BOOST_REGEX
#include <boost/regex.hpp>
#else
#include <regex>
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <ciso646>
#include <cstdio>
#include <iterator>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

namespace visr
{
namespace panning
{
  LoudspeakerArray::LoudspeakerArray()
  : m_is2D( false )
  , m_isInfinite( false )
  {
  }
  
  LoudspeakerArray::LoudspeakerArray( std::string const & xmlConfiguration )
  {
    loadXmlFile( xmlConfiguration );
  }
  
  LoudspeakerArray const &  LoudspeakerArray::operator=( LoudspeakerArray const & rhs )
  {
    m_is2D = rhs.m_is2D;
    m_isInfinite = rhs.m_isInfinite;
    m_position = rhs.m_position;
    m_triplet = rhs.m_triplet;
    m_channel = rhs.m_channel;
    m_id = rhs.m_id;
    m_reRoutingCoeff.resize( rhs.m_reRoutingCoeff.numberOfRows(), rhs.m_reRoutingCoeff.numberOfColumns() );
    m_reRoutingCoeff.copy( rhs.m_reRoutingCoeff );
    m_subwooferChannels = rhs.m_subwooferChannels;
    m_subwooferGains.resize( rhs.m_subwooferGains.numberOfRows(), rhs.m_subwooferGains.numberOfColumns() );
    m_subwooferGains.copy( rhs.m_subwooferGains );
    m_gainAdjustment.assign( rhs.m_gainAdjustment );
    m_delayAdjustment.assign( rhs.m_delayAdjustment );
    return *this;
  }
  
  LoudspeakerArray::~LoudspeakerArray() = default;
  
  namespace // unnamed
  {
    
    /**
     * Local function to parse the coordinate of either a normal or a virtual loudspeaker.
     * @param node The speaker node
     * @param isInfinite The global infinity flag for the array, i.e., whether loudspeakers are considered to emit plane waves.
     * @param is2D Whether the loudspeaker config is considered as 2D or 3D. In the latter case, the coordiante attributes 
     * "z" and "el" are optional and default to zero.
     * @return Reference to return the parsed position.
     * @throw std::invalid_argument If parsing fails.
     */
    XYZ parseCoordNode( boost::property_tree::ptree const & node, bool isInfinite, bool is2D )
    {
      XYZ pos( 0.0f, 0.0f, 0.0f, isInfinite );
      using namespace boost::property_tree;
      std::size_t numCartCoords = node.count( "cart" );
      std::size_t numPolarCoords = node.count( "polar" );
      
      if( numCartCoords > 1 or numPolarCoords > 1 or not((numCartCoords == 1) xor (numPolarCoords == 1)) )
      {
        throw std::invalid_argument( "LoudspeakerArray::loadXml(): For each speaker, exactly one \"cart\" or \"polar\" node must exist.`" );
      }
      if( numCartCoords == 1 )
      {
        ptree::const_assoc_iterator cartIt = node.find( "cart" );
        assert( cartIt != node.not_found() );
        ptree const coordNode = cartIt->second;
        pos.x = coordNode.get<SampleType>( "<xmlattr>.x" );
        pos.y = coordNode.get<SampleType>( "<xmlattr>.y" );
        if( is2D )
        {
          auto const zOpt = coordNode.get_optional<SampleType>("<xmlattr>.z" );
          pos.z = zOpt ? *zOpt : static_cast<SampleType>(0.0);
        }
        else
        {
          pos.z = coordNode.get<SampleType>("<xmlattr>.z");
        }
      }
      else
      {
        ptree::const_assoc_iterator polarIt = node.find( "polar" );
        assert( polarIt != node.not_found() );
        ptree const coordNode = polarIt->second;
        SampleType const az = coordNode.get<SampleType>( "<xmlattr>.az" );
        SampleType el{ static_cast<SampleType>(0.0) };
        if( is2D )
        {
          auto const elOpt = coordNode.get_optional<SampleType>( "<xmlattr>.el" );
          el = elOpt ? *elOpt : static_cast<SampleType>(0.0);
        }
        else
        {
          el = coordNode.get<SampleType>("<xmlattr>.el");
        }
        SampleType const r = coordNode.get<SampleType>( "<xmlattr>.r" );
        std::tie( pos.x, pos.y, pos.z ) = efl::spherical2cartesian( efl::degree2radian( az ), efl::degree2radian( el ), r );
      }
      return pos;
    }
    
    /**
     * Local function to parse the gain and delay adjustment out of either a loudspeaker or a subwoofer node.
     * @param node The XML element containing the attributes
     * @param [out] gain The parsed gain value, linear scale.
     * @param [out] delay The parsed delay value, in seconds.
     * @throw std::invalid_argument if the parsing fails.
     */
    void parseGainDelayAdjustments( boost::property_tree::ptree const & node, SampleType & gain, SampleType & delay )
    {
      boost::optional<SampleType> const gainLinear = node.get_optional<SampleType>( "<xmlattr>.gain" );
      boost::optional<SampleType> const gainDB = node.get_optional<Afloat>( "<xmlattr>.gainDB" );
      if( gainLinear and gainDB )
      {
        throw std::invalid_argument( "ArrayConfiguration::loadXml(): The \"gain\" or \"gainDB\" attributes are mutually exclusive." );
      }
      else if( gainLinear )
      {
        gain = *gainLinear;
      }
      else if( gainDB )
      {
        gain = std::pow( 10.0f, *gainDB / 20.0f );
      }
      else // no gain specified, use default value.
      {
        gain = 1.0f;
      }
      boost::optional<Afloat> const delayOpt = node.get_optional<Afloat>( "<xmlattr>.delay" );
      delay = delayOpt ? *delayOpt : 0.0f;
    }
    
    using ChannelEqLookupTable = std::map<std::string, rbbl::BiquadCoefficientList<Afloat> >;
    
    /**
     * Local function to parse the optional node "outputEqConfiguration"
     * @param tree
     */
    void parseChannelLookup( boost::property_tree::ptree const & tree, ChannelEqLookupTable & table, std::size_t numEqSections )
    {
      using boost::property_tree::ptree;
      table.clear();
      auto const filterSpecNodes = tree.equal_range( "filterSpec" );
      for( ptree::const_assoc_iterator nodeIt( filterSpecNodes.first ); nodeIt != filterSpecNodes.second; ++nodeIt )
      {
        ptree const nodeTree = nodeIt->second;
        std::string const specName = nodeTree.get<std::string>( "<xmlattr>.name" );
        rbbl::BiquadCoefficientList<Afloat> biqList = rbbl::BiquadCoefficientList<Afloat>::fromXml( nodeTree );
        
        bool insertRes;
        std::tie( std::ignore, insertRes ) = table.insert( std::make_pair( specName, biqList ) );
      }
    }
    
    void parseEqFilter( boost::property_tree::ptree const & tree,
                       ChannelEqLookupTable const & table,
                       std::size_t channelIndex,
                       rbbl::BiquadCoefficientMatrix<Afloat> & eqMatrix )
    {
      boost::optional<std::string> const eqId = tree.get_optional<std::string>( "<xmlattr>.eq" );
      if( eqId )
      {
        ChannelEqLookupTable::const_iterator findEq = table.find( *eqId );
        if( findEq == table.end() )
        {
          throw std::invalid_argument( "LoudspeakerArray::loadXml(): ." );
        }
        eqMatrix.setFilter( channelIndex, findEq->second );
      }
      else
      {
        eqMatrix.setFilter( channelIndex, rbbl::BiquadCoefficientList<Afloat>() );
      }
    }
    
  } // unnamed namespace
  
  
  void LoudspeakerArray::loadXmlFile( std::string const & filePath )
  {
    boost::filesystem::path path( filePath );
    if( not exists( path ) or is_directory( path ) )
    {
      throw std::invalid_argument( "ArrayConfiguration::loadXml(): File does not exist." );
    }
    std::ifstream fileStream( path.string() );
    if( not fileStream )
    {
      throw std::invalid_argument( "ArrayConfiguration::loadXml(): Invalid file path." );
    }
    loadXmlStream( fileStream );
  }
  
  void LoudspeakerArray::loadXmlStream( std::istream & fileStream )
  {
    using namespace boost::property_tree;
    boost::property_tree::ptree parseTree;
    boost::property_tree::read_xml( fileStream, parseTree );
    
    boost::property_tree::ptree treeRoot = parseTree.get_child( "panningConfiguration" );
    
    boost::optional<std::string> const dimension = treeRoot.get_optional<std::string>( "<xmlattr>.dimension" );
    if( dimension )
    {
      if( *dimension == "2" )
      {
        m_is2D = true;
      }
      else if( *dimension == "3" )
      {
        m_is2D = false;
      }
      else
      {
        throw std::invalid_argument( "LoudspeakerArray::loadXml(): If given, the \"dimension\" atttribute must be either \"2\" or \"3\"." );
      }
    }
    else
    {
      m_is2D = false;
    }
    boost::optional<bool> const infinity = treeRoot.get_optional<bool>( "<xmlattr>.infinite" );
    m_isInfinite = infinity ? *infinity : false;
    
    auto const speakerNodes = treeRoot.equal_range( "loudspeaker" );
    auto const virtualSpeakerNodes = treeRoot.equal_range( "virtualspeaker" );
    auto const subwooferNodes = treeRoot.equal_range( "subwoofer" );
    
    std::size_t const numRegularSpeakers = std::distance( speakerNodes.first, speakerNodes.second );
    std::size_t const numVirtualSpeakers = std::distance( virtualSpeakerNodes.first, virtualSpeakerNodes.second );
    std::size_t const numTotalSpeakers = numRegularSpeakers + numVirtualSpeakers;
    std::size_t const numSubwoofers = std::distance( subwooferNodes.first, subwooferNodes.second );
    
    m_position.resize( numTotalSpeakers );
    m_channel.resize( numRegularSpeakers );
    
    m_gainAdjustment.resize( numRegularSpeakers + numSubwoofers );
    m_delayAdjustment.resize( numRegularSpeakers + numSubwoofers );
    
    m_reRoutingCoeff.resize( numVirtualSpeakers, numRegularSpeakers );
    m_reRoutingCoeff.zeroFill();
    
    const ChannelIndex cInvalidChannel = std::numeric_limits<ChannelIndex>::max();
    std::fill( m_channel.begin(), m_channel.end(), cInvalidChannel ); // assign special value to check afterwards if every speaker index has been assigned.
    
    std::size_t const numEqConfigs = treeRoot.count( "outputEqConfiguration" );
    if( numEqConfigs >= 2 )
    {
      throw std::invalid_argument( "LoudspeakerArray::loadXml(): Either zero or one \"outputEqConfiguration\" nodes are admissible." );
    }
    bool const eqConfigPresent = (numEqConfigs == 1);
    ChannelEqLookupTable eqLookupTable;
    std::size_t numEqSections = 0;
    if( eqConfigPresent )
    {
      ptree const eqNode = treeRoot.get_child( "outputEqConfiguration" );
      numEqSections = eqNode.get<std::size_t>( "<xmlattr>.numberOfBiquads" );
      parseChannelLookup( treeRoot.get_child( "outputEqConfiguration" ), eqLookupTable, numEqSections );
      mOutputEqs.reset( new rbbl::BiquadCoefficientMatrix<Afloat>( numRegularSpeakers + numSubwoofers, numEqSections ) );
    }
    else
    {
      mOutputEqs.reset(); // Make it a null ptr to flag 'no EQ config present'
    }
    // TODO: If we leave the instantiation here, the EQ configuration
    // will always be present, albeit with zero biquad sections. However, the
    // renderer should work in this case as well (and currently does).
    // The current setting distinguished between the cases.
    // mOutputEqs.reset( new rbbl::BiquadCoefficientMatrix<Afloat>( numRegularSpeakers, numEqSections ) );
    

#ifdef VISR_USE_BOOST_REGEX
    static const boost::regex idExpression{ "^[A-za-z0-9@&\\(\\)\\+/:_-]+$" };
#else
    // Put the hyphen (minus) last in the character class to avoid quoting issues.
    static const std::regex idExpression{ "^[[:alnum:]@&\\(\\)\\+/:_-]+$", std::regex::ECMAScript};
#endif
    // parsing the loudspeaker ids and channels
    std::size_t i = 0;
    for( ptree::const_assoc_iterator treeIt( speakerNodes.first ); treeIt != speakerNodes.second; ++treeIt, i++ )
    {
      ptree const childTree = treeIt->second;
      std::string id = childTree.get<std::string>( "<xmlattr>.id" );
      boost::trim_if( id, boost::is_any_of( "\t " ) );
#ifdef VISR_USE_BOOST_REGEX
      if( (not boost::regex_match( id, idExpression )) or (m_id.find( id ) != m_id.end()))
#else
      if( (not std::regex_match( id, idExpression )) or (m_id.find( id ) != m_id.end()) )
#endif
      {
        throw std::invalid_argument( "LoudspeakerArray::loadXml(): The loudspeaker id must be unique and must only contain alphanumeric characters or \" @ & ( ) + / : - _ \" " );
      }
      std::size_t idZeroOffset = i;
      if( m_channel[idZeroOffset] != cInvalidChannel )
      {
        throw std::invalid_argument( "LoudspeakerArray::loadXml(): Each channel id must be used exactly once." );
      }
      ChannelIndex const chIdx = childTree.get<ChannelIndex>( "<xmlattr>.channel" );
      if( chIdx < 1 )
      {
        throw std::invalid_argument( "LoudspeakerArray::loadXml(): The channel id must be greater or equal than one." );
      }
      m_id[id] = 0; //just to have the id as a key in the map, to search for duplicates
      m_channel[i] = chIdx;
      
    }
    
    // Reordering by progressive channel number
    std::sort( m_channel.begin(), m_channel.end() );
    
    
    // reparsing the xml with ordered channels
    
    std::size_t w=0;
    for( ptree::const_assoc_iterator treeIt( speakerNodes.first ); treeIt != speakerNodes.second; ++treeIt, ++w )
    {
      ptree const childTree = treeIt->second;
      std::string id = childTree.get<std::string>( "<xmlattr>.id" );
      //      if(std::find(m_channel.begin(),m_channel.end(),m_channel[w]) == m_channel.end()) std::cout<<"NOTFIND"<<std::endl;
      ChannelIndex const chIdx = childTree.get<ChannelIndex>( "<xmlattr>.channel" );
      size_t offset = std::distance(m_channel.begin(),std::find(m_channel.begin(),m_channel.end(),chIdx));
      
      m_id[id] = offset;
      
      
      m_position[m_id[id]] = parseCoordNode( childTree, m_isInfinite, m_is2D );
      
      parseGainDelayAdjustments( childTree, m_gainAdjustment[m_id[id]], m_delayAdjustment[m_id[id]] );
      
      if( eqConfigPresent )
      {
        parseEqFilter( childTree, eqLookupTable, m_id[id], *mOutputEqs );
      }
    }
    
    
    // Same for the virtual speaker nodes, except there is no 'channel' field and no gain/delay adjustments.
    for( ptree::const_assoc_iterator treeIt( virtualSpeakerNodes.first ); treeIt != virtualSpeakerNodes.second; ++treeIt, i++ )
    {
      ptree const childTree = treeIt->second;
      std::string id = childTree.get<std::string>( "<xmlattr>.id" );
      boost::trim_if( id, boost::is_any_of( "\t " ) );
#ifdef VISR_USE_BOOST_REGEX
      if( (not boost::regex_match( id, idExpression) ) or (m_id.find( id ) != m_id.end()) )
#else
      if( (not std::regex_match( id, idExpression )) or (m_id.find( id ) != m_id.end()) )
#endif
      {
        throw std::invalid_argument( "LoudspeakerArray::loadXml(): The virtual loudspeaker id must be unique and must only contain alphanumeric characters or \" @ & ( ) + / : - _ \" " );
      }
      m_id[id] = i;
      
      auto const routeNodes = childTree.equal_range( "route" );
      
      std::size_t w = 0;
      for( ptree::const_assoc_iterator routeIt( routeNodes.first ); routeIt != routeNodes.second; ++routeIt, w++ )
      {
        ptree const childTree = routeIt->second;
        std::string lspId = childTree.get<std::string>( "<xmlattr>.lspId" );
        SampleType const gainDB = childTree.get<SampleType>( "<xmlattr>.gainDB" );
        boost::trim_if( lspId, boost::is_any_of( "\t " ) );
        
        if( m_id.find( lspId ) == m_id.end() or lspId.empty() )
        {
          throw std::invalid_argument( "LoudspeakerArray::loadXml(): Cannot find loudspeaker id in virtual speaker routing." );
        }
        //std::cout <<"RER: "<< i - numRegularSpeakers << " => " << m_id[lspId] << '\n';
        m_reRoutingCoeff( i - numRegularSpeakers, m_id[lspId] ) = efl::dB2linear( gainDB );
      }
      /**************print rerouting matrix*******************/
      /*
       size_t row, columns;
       for( size_t row = 0; row < numVirtualSpeakers; row++ )
       {
       for( int columns = 0; columns < numRegularSpeakers; columns++ )
       std::cout << m_reRoutingCoeff.at( row, columns ) << "\t";
       std::cout << std::endl;
       }
       */
      m_position[i] = parseCoordNode( childTree, m_isInfinite, m_is2D );
    }
    // The checks above (all speaker indices are between 1 and numTotalSpeakers && the indices are unique) are
    // sufficient to ensure that the speaker indices are consecutive.
    
    
    // parse the triplet config
    auto const  tripletNodes = treeRoot.equal_range( "triplet" );
    std::size_t const numTriplets = std::distance( tripletNodes.first, tripletNodes.second );
    m_triplet.clear();
    m_triplet.reserve( numTriplets );
    
    for( ptree::const_assoc_iterator tripletIt( tripletNodes.first ); tripletIt != tripletNodes.second; ++tripletIt )
    {
      ptree const childTree = tripletIt->second;
      std::array<LoudspeakerIndexType, 3> triplet;
      std::string tid1 = childTree.get<std::string>( "<xmlattr>.l1" );
      std::string tid2 = childTree.get<std::string>( "<xmlattr>.l2" );
      boost::trim_if( tid1, boost::is_any_of( "\t " ) );
      boost::trim_if( tid2, boost::is_any_of( "\t " ) );
      if( m_id.find( tid1 ) == m_id.end()
         or m_id.find( tid2 ) == m_id.end()
         or tid1.empty() or tid2.empty() )
      {
        throw std::invalid_argument( "LoudspeakerArray::loadXml(): Cannot find triplet loudspeaker id." );
      }
      
      
      triplet[0] = m_id[tid1];
      triplet[1] = m_id[tid2];
      if( m_is2D )
      {
        triplet[2] = -1; // special value reserved for 'unused'
      }
      else
      {
        std::string tid3 = childTree.get<std::string>( "<xmlattr>.l3" );
        boost::trim_if( tid3, boost::is_any_of( "\t " ) );
        if( m_id.find( tid3 ) == m_id.end() or tid3.empty() )
        {
          throw std::invalid_argument( "LoudspeakerArray::loadXml(): Cannot find triplet loudspeaker id." );
        }
        triplet[2] = m_id[tid3];
        
      }
      m_triplet.push_back( triplet );
    }
    assert( m_triplet.size() == numTriplets );
    
    std::cout<<std::endl;
    
    // Subwoofer configuration
    m_subwooferChannels.resize( numSubwoofers );
    m_subwooferGains.resize( numSubwoofers, numRegularSpeakers );
    std::size_t subIdx( 0 );
    for( ptree::const_assoc_iterator subIt( subwooferNodes.first ); subIt != subwooferNodes.second; ++subIt, ++subIdx )
    {
      ptree const subNode = subIt->second;
      ChannelIndex const subChannel = subNode.get<std::size_t>( "<xmlattr>.channel" );
      m_subwooferChannels[subIdx] = subChannel;
      
      std::string speakerIndicesStr = subNode.get<std::string>( "<xmlattr>.assignedLoudspeakers" );
      std::vector<std::string> speakerIndices;
      boost::find_format_all( speakerIndicesStr, boost::token_finder( ::isspace ), boost::const_formatter( "" ) );
      boost::split( speakerIndices, speakerIndicesStr, boost::is_any_of( "," ), boost::token_compress_on );
      if( std::find_if( speakerIndices.begin(), speakerIndices.end(),
                       [&]( std::string val ) { return (m_id.find( val ) == m_id.end() or (val.empty())); } ) != speakerIndices.end() )
      {
        throw std::invalid_argument( "At least one of the specified loudspeaker index in subwoofer's list does not exist." );
      }
      
      boost::optional<std::string> const weightStr = subNode.get_optional<std::string>( "<xmlattr>.weights" );
      if( weightStr )
      {
        rbbl::FloatSequence<SampleType> speakerWeights( *weightStr );
        if( speakerWeights.size() != speakerIndices.size() )
        {
          throw std::invalid_argument( "The loudspeaker index list and the weight vector must have the same length." );
        }
        
        for( std::size_t entryIdx( 0 ); entryIdx < speakerIndices.size(); ++entryIdx )
        {
          m_subwooferGains( subIdx, m_id[speakerIndices[entryIdx]] ) = speakerWeights[entryIdx];
        }
      }
      else // No weight vector given, assign ones to all channels indexed by the assignedLoudspeakers string.
      {
        for( std::size_t entryIdx( 0 ); entryIdx < speakerIndices.size(); ++entryIdx )
        {
          m_subwooferGains( subIdx, m_id[speakerIndices[entryIdx]] ) = 1.0f;
        }
      }
      
      parseGainDelayAdjustments( subNode, m_gainAdjustment[numRegularSpeakers + subIdx],
                                m_delayAdjustment[numRegularSpeakers + subIdx] );
      
      if( eqConfigPresent )
      {
        parseEqFilter( subNode, eqLookupTable, numRegularSpeakers + subIdx, *mOutputEqs );
      }
    }
    // Check the subwoofer and the subwoofer indices whether all indices are unique.
    std::vector<ChannelIndex> spkIndicesSorted( m_channel );
    std::vector<ChannelIndex> subIndicesSorted( m_subwooferChannels );
    std::sort( spkIndicesSorted.begin(), spkIndicesSorted.end() );
    std::sort( subIndicesSorted.begin(), subIndicesSorted.end() );
    if( std::adjacent_find( spkIndicesSorted.begin(), spkIndicesSorted.end() ) != spkIndicesSorted.end() )
    {
      // Technically, this isn't possible, as the implementation above allows only consecutive ranges
      // from 1...numRegularSpeakers.
      throw std::invalid_argument( "LoudspeakerArray::loadXml(): Duplicated loudspeaker channel indices." );
    }
    if( std::adjacent_find( subIndicesSorted.begin(), subIndicesSorted.end() ) != subIndicesSorted.end() )
    {
      throw std::invalid_argument( "LoudspeakerArray::loadXml(): Duplicated subwoofer channel indices." );
    }
    std::vector<ChannelIndex> collidingChannelIndices;
    std::set_intersection( spkIndicesSorted.begin(), spkIndicesSorted.end(),
                          subIndicesSorted.begin(), subIndicesSorted.end(),
                          std::back_inserter( collidingChannelIndices ) );
    
    if( not collidingChannelIndices.empty() )
    {
      throw std::invalid_argument( "LoudspeakerArray::loadXml(): Loudspeaker and subwoofer channels indices are not exclusive." );
    }
  }
  
  void LoudspeakerArray::loadXmlString( std::string const & xmlString ){
    std::istringstream iss( xmlString );
    loadXmlStream(iss);
  }
  
  bool LoudspeakerArray::outputEqualisationPresent() const
  {
    return bool( mOutputEqs ); // Operator bool checks whether pointer is assigned.
  }
  
  /**
   * Query the number of biquads per output channel.
   * Returns 0 if no equalisation configuration is present.
   */
  std::size_t LoudspeakerArray::outputEqualisationNumberOfBiquads() const
  {
    return outputEqualisationPresent() ? mOutputEqs->numberOfSections() : 0;
  }
  
  /**
   * Return a matrix containing the biquad parameters for all output sections.
   * The dimension of the matrix is
   * \p (numRegularSpeakers()+getNumSubwoofers()) x outputEqualisationNumberOfBiquads()
   * @throw std::logic_error if no output EQ configuration is present, i.e., outputEqualisationPresent() is \b false.
   */
  rbbl::BiquadCoefficientMatrix<SampleType> const &
  LoudspeakerArray::outputEqualisationBiquads() const
  {
    if( not outputEqualisationPresent() )
    {
      throw std::logic_error( "LoudspeakerArray::::outputEqualisationBiquads(): No EQ configuration present." );
    }
    return *mOutputEqs;
  }
  
  LoudspeakerArray::LoudspeakerIdType const &
  LoudspeakerArray::loudspeakerId( LoudspeakerIndexType const & index ) const
  {
    
    auto const findIt  = std::find_if( m_id.begin(), m_id.end(), [index](decltype(m_id)::value_type const & val ){ return val.second == index; } );
    if( findIt == m_id.end() )
    {
      std::stringstream msg;
      msg << "LoudspeakerArray::loudspeakerId(): The given numeric loudspeaker id \"" << index << "\" is outside the admissible range.";
      throw std::out_of_range( msg.str() );
    }
    return findIt->first;
  }

  LoudspeakerArray::LoudspeakerIndexType LoudspeakerArray::getSpeakerIndexFromId( LoudspeakerIdType const & id ) const {
    return m_id.at(id);
  }
  
} // namespace panning
} // namespace visr

