/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "index_sequence.hpp"

#include <boost/bind/bind.hpp>
#include <boost/spirit/include/qi.hpp>

#include <algorithm>
#include <ciso646>

namespace visr
{
namespace rbbl
{

IndexSequence::IndexSequence()
{
}  

IndexSequence::IndexSequence( std::initializer_list<IndexType> const & val )
  : mIndices( val.begin(), val.end() )
{
}
 
IndexSequence::IndexSequence( std::string const & val )
{
  namespace qi = boost::spirit::qi;

  std::string::const_iterator first = val.begin(); 
  std::string::const_iterator last = val.end();

  /**
   * Internal object which is called by the spirit parser to hold the results of parsing.
   */
  struct ParseState
  {
  public:
    void push( int val )
    {
      mStack.push_back( val );
    }

    void finish()
    {
      switch( mStack.size() )
      {
      case 0:
        throw std::invalid_argument( "Internal parse error." );
      case 1:
      {
        int const val = mStack[0];
        if( val < 0 )
        {
          throw("IndexSequence: Indices must be greater or equal than zero.");
        }
        mContents.push_back( static_cast<IndexType>(val) );
        break;
      }
      case 2:
      {
        int const start = mStack[0];
        int const end = mStack[1];
        if( start < 0 or end < 0 )
        {
          throw("IndexSequence: The start and end values of a range must be greater or equal than zero.");
        }
        IndexType val = static_cast<IndexType>(mStack[0]);
        while( val <= static_cast<IndexType>(end) )
        {
          mContents.push_back( val++ );
        }
        break;
      }
      case 3:
      {
        int const start = mStack[0];
        int const inc = mStack[1];
        int const end = mStack[2];
        if( start < 0 or end < 0 )
        {
          throw("IndexSequence: The start and end values of a range must be greater or equal than zero.");
        }
        if( inc == 0 )
        {
          throw("IndexSequence: The increment of a range must not be zero.");
        }
        int val = start;
        if( inc > 0 )
        {
          while( val <= end )
          {
            mContents.push_back( val );
            val += inc;
          }
        }
        else // inc < 0
        {
          while( val >= end )
          {
            mContents.push_back( val );
            val += inc;
          }
        }
        break;
      }
      default:
        throw std::invalid_argument( "IndexSequence: Sequence of more than three colon-separated values." );
      }
      mStack.clear();
    }

    std::vector<int> mStack;
    ContainerType mContents;
  };
  ParseState state;
  using namespace boost::placeholders;
  auto const atom = boost::proto::deep_copy( ((qi::int_[boost::bind( &ParseState::push, &state, _1 )] % qi::char_( ":" ))[boost::bind( &ParseState::finish, &state )] % (qi::char_( ',' ))) | qi::eps );

  bool const parseRet = qi::phrase_parse( first, last, atom,
    qi::ascii::space );

  if( (not parseRet) or( first != last ) )
  {
    throw std::invalid_argument( "IndexSequence: Parsing of initialiser string failed." );
  }

  mIndices.swap( state.mContents );
}

void IndexSequence::clear()
{
  mIndices.clear();
}

} // namespace rbbl
} // namespace visr
