/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "channel_object_parser.hpp"

#include "channel_object.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/bind/bind_function.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>

#include <algorithm>
#include <ciso646>
#include <iterator>
#include <functional>
#include <sstream>


namespace visr
{
namespace objectmodel
{

namespace // unnamed
{

struct ParserState
{
  void appendOutputChannel( ChannelObject::OutputChannelList const & val )
  {
    mChannels.push_back( val );
  }

  ChannelObject::OutputChannelContainer const & channels() { return mChannels; }

private:
  ChannelObject::OutputChannelContainer mChannels;
};

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

void pushLabel( ChannelObject::OutputChannelList & list, ChannelObject::OutputChannelId const & id )
{
  list.push_back( id );
}

template< typename IteratorType, typename Skipper >
class Parser: public qi::grammar<IteratorType, ParserState(), Skipper>
{
public:
  Parser():
   Parser::base_type( mTopLevelRule )
  {
    // auto _addLabel = phoenix::bind( &ParserState::appendOutputChannel, qi::_val );

    mLabelRule = +qi::char_("a-zA-Z0-9+-&@+_-");

    mOutputChannelRule = mLabelRule[phoenix::bind( &pushLabel, qi::_val, qi::_1 )] | 
      (qi::char_('[') >> -(mLabelRule[phoenix::bind( &pushLabel, qi::_val, qi::_1 )] % qi::char_( "," )) >> qi::char_( ']' ) );

    mTopLevelRule = -(mOutputChannelRule[phoenix::bind( &ParserState::appendOutputChannel, qi::_val, qi::_1 )] % qi::char_( "," ));
  }

private:
  qi::rule<IteratorType, ParserState(), Skipper> mTopLevelRule;

  qi::rule<IteratorType, std::string(), boost::spirit::ascii::space_type > mLabelRule;
  qi::rule<IteratorType, ChannelObject::OutputChannelList(), boost::spirit::ascii::space_type > mOutputChannelRule;
};

ChannelObject::OutputChannelContainer parseOutputChannels( std::string const strVal )
{
  ParserState state;
  using IteratorType = std::string::const_iterator;
  Parser<IteratorType, qi::ascii::space_type> const parser;
  IteratorType strStart = strVal.begin();
  IteratorType strStop = strVal.end();
  bool parsingOK = qi::phrase_parse( strStart, strStop, parser, qi::ascii::space, state );
  if( (not parsingOK) or (strStart != strStop) )
  {
    throw std::invalid_argument( "The outputChannels attribute is invalid." );
  }

  return state.channels();
}

} // Unnamed namespace

/*virtual*/ void ChannelObjectParser::
parse( boost::property_tree::ptree const & tree, Object & obj ) const
{
  try
  {
    // parse generic part of Object (parent of ChannelObject)
    ObjectParser::parse( tree, obj );

    ChannelObject & chObj = dynamic_cast<ChannelObject &>(obj);

    chObj.setDiffuseness( tree.get<LevelType>( "diffuseness", static_cast<LevelType>(0.0) ) );

    std::string const outChannelStr = tree.get<std::string>( "outputChannels" );
    ChannelObject::OutputChannelContainer const channelIds = parseOutputChannels( outChannelStr );

    if( channelIds.size() != obj.numberOfChannels())
    {
      throw std::invalid_argument("ChannelObjectParser::parse() Number of output channels does not match the number of object channels.");
    }
    chObj.setOutputChannels( channelIds );
  }
  // TODO: distinguish between boost property_tree parse errors and bad dynamic casts.
  catch( std::exception const & ex )
  {
    throw std::invalid_argument( std::string( "Error while parsing channel object: ") + ex.what() );
  }
}

namespace // unnamed
{
/***
 */
//@{
std::ostream & operator<<( std::ostream & str, ChannelObject::OutputChannelList const & channels )
{
  if( channels.empty() )
  {
    str << "[]";
  }
  else if( channels.size() == 1 )
  {
    str << channels[0];
  }
  else
  {
    str << "[ " << channels[0];
    for( ChannelObject::OutputChannelList::const_iterator runIt( channels.begin()+1);
         runIt != channels.end(); ++runIt )
    {
      str << ", " << *runIt;
    }
    str << " ]";
  }
  return str;
}

std::ostream & operator<<( std::ostream & str, ChannelObject::OutputChannelContainer const & outChannels )
{
  if( not outChannels.empty() )
  {
    str << outChannels[0];
  }
  for( ChannelObject::OutputChannelContainer::const_iterator runIt( outChannels.begin() + 1 );
    runIt != outChannels.end(); ++runIt )
  {
    str << ", " << *runIt;
  }
  return str;
}

} // unnamed namespace

/*virtual*/ void ChannelObjectParser
::write( Object const & obj, boost::property_tree::ptree & tree ) const
{
  ObjectParser::write( obj, tree );

  ChannelObject const & chObj = dynamic_cast<ChannelObject const &>(obj);

  // Check object invariant before writing
  if( chObj.numberOfChannels() != chObj.outputChannels().size())
  {
    throw std::invalid_argument( "ChannelObjectParser::write(): Object is inconsistent, because the number of output channels does not the number of object channels." );
  }

  if( chObj.diffuseness() != static_cast<LevelType>(0.0) )
  {
    tree.put( "diffuseness", chObj.diffuseness() );
  }
  std::stringstream outChannelStr;
  outChannelStr << chObj.outputChannels();
  tree.put( "outputChannels", outChannelStr.str() );
}
//@}
} // namespace objectmodel
} // namespace visr
