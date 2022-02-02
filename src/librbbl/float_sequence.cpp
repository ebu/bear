/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "float_sequence.hpp"

#include <boost/bind/bind.hpp>
#include <boost/spirit/include/qi.hpp>

#include <algorithm>
#include <ciso646>
#include <cmath>
#include <limits>

namespace visr
{
namespace rbbl
{

template< typename ElementType>
FloatSequence<ElementType>::FloatSequence()
{
}  

template< typename ElementType>
FloatSequence<ElementType>::FloatSequence( ElementType val, std::size_t num /*= 1*/ )
: mValues( num, val ) // Relies on mValues being a std::vector.
{
}

template< typename ElementType>
FloatSequence<ElementType>::FloatSequence( ElementType const * const val, std::size_t numValues )
 : mValues( val, val + numValues )
{
}



template< typename ElementType>
FloatSequence<ElementType>::FloatSequence( std::initializer_list<ElementType> const & val )
  : mValues( val.begin(), val.end() )
{
}
 
template< typename ElementType>
FloatSequence<ElementType>::FloatSequence( std::string const & val )
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
    void push( ElementType val )
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
        ElementType const val = mStack[0];
        mContents.push_back( val );
        break;
      }
      case 2:
      {
        ElementType val = mStack[0];
        ElementType const end = mStack[1];
        while( val <= end )
        {
          mContents.push_back( val++ );
        }
        break;
      }
      case 3:
      {
        ElementType const start = mStack[0];
        ElementType const inc = mStack[1];
        ElementType const end = mStack[2];

        if( std::abs(inc) < std::numeric_limits<ElementType>::epsilon() )
        {
          throw("FloatSequence: The increment of a range must not be zero.");
        }
        ElementType val = start;
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
        throw std::invalid_argument( "FloatSequence: Sequence of more than three colon-separated values." );
      }
      mStack.clear();
    }

    std::vector<ElementType> mStack;
    ContainerType mContents;
  };
  ParseState state;

  using namespace boost::placeholders;
  // TODO: find a workaround around this likely GCC compiler bug which
  // prevents us from using the completely type-correct version.
  // auto const atom = ((qi::real_parser<typename ElementType>()[boost::bind( &ParseState::push, &state, _1 )] % qi::char_( ":" ))[boost::bind( &ParseState::finish, &state )] % (qi::char_( ',' )));
  auto const atom = boost::proto::deep_copy(((qi::float_[boost::bind( &ParseState::push, &state, _1 )] % qi::char_( ":" ))[boost::bind( &ParseState::finish, &state )] % (qi::char_( ',' ))));
  bool const parseRet = qi::phrase_parse( first, last, atom,
    qi::ascii::space );

  if( (not parseRet) or( first != last ) )
  {
    throw std::invalid_argument( "SignalList: Parsing of initialiser string failed." );
  }

  mValues.swap( state.mContents );
}

template< typename ElementType>
void FloatSequence<ElementType>::clear( )
{
  mValues.clear();
}

template< typename ElementType>
std::string FloatSequence<ElementType>::toString( std::string const & separator /*= std::string( ", " )*/ ) const
{
  std::stringstream strStr;
  // std::vector<ElementType> tmp;
  std::copy( mValues.begin(), mValues.end(), std::ostream_iterator<ElementType>( strStr, separator.c_str() ) );
  std::string const str( strStr.str() );
  // Remove the trailing separator added by the ostream_iterator
  return str.substr( 0, str.size() - separator.size() );
}

// Explicit instantiations
template class FloatSequence<float>;
template class FloatSequence<double>;

} // namespace rbbl
} // namespace visr
