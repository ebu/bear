/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "vector_parameter.hpp"

#include <libvisr/constants.hpp>
#include <libvisr/parameter_factory.hpp>

#include <libefl/vector_functions.hpp>

#ifdef VISR_PML_USE_SNDFILE_LIBRARY
#include <sndfile.h>
#endif

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <fstream>

namespace visr
{
namespace pml
{

template< typename ElementType >
VectorParameter<ElementType>::VectorParameter( ParameterConfigBase const & config )
: VectorParameter( dynamic_cast<VectorParameterConfig const &>(config) )
{
}

template< typename ElementType >
VectorParameter<ElementType>::VectorParameter( VectorParameterConfig const & config )
: efl::BasicVector<ElementType>( config.numberOfElements(), cVectorAlignmentSamples ) // Use standard alignment
{
}

template< typename ElementType >
VectorParameter<ElementType>::VectorParameter( VectorParameter<ElementType> const & rhs )
  : VectorParameter<ElementType>( rhs.size(), rhs.alignmentElements() )
{
  if( efl::vectorCopy( rhs.data(), this->data(), this->size(), this->alignmentElements() ) != efl::noError )
  {
    throw std::runtime_error( "Copying of initial values failed." );
  }
}

template< typename ElementType >
VectorParameter<ElementType>::~VectorParameter() = default;

template< typename ElementType >
VectorParameter<ElementType>& VectorParameter<ElementType>::operator=(VectorParameter<ElementType> const & rhs)
{
  if( this->size() != rhs.size() )
  {
    throw std::out_of_range( "VectorParameter assignment: Sizes are different." );
  }
  efl::BasicVector<ElementType>::copy( rhs );
  return *this;
}

  
namespace // unnamed
{

  /**
  * Template to provide type translations for the types used in the stream parsing function.
  * By default the type is passed unaltered.
  * For complex floating point types, the template is specialised to translate to the corresponding real type.
  * @todo Check whether it is possible to use a partial template specification to handle all complex types in one specialisation.
  */
  template< typename T>
  struct TranslateType
  {
    using type = T;
  };

  /**
  * Specialization for complex<float>
  */
  template<>
  struct TranslateType<std::complex<float> >
  {
    using type = float;
  };

  /**
  * Specialization for complex<double>
  */
  template<>
  struct TranslateType<std::complex<double> >
  {
    using type = double;
  };

}

template< typename ElementType >
/*static*/ VectorParameter<ElementType>
VectorParameter<ElementType>::fromString( std::string const & initStr, std::size_t alignment /*= 0*/ )
{
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;

  std::vector<ElementType> v;
  // Have to convert the indices returned by find_first_of into iterators
  std::string::const_iterator startIt = initStr.begin();
  std::string::const_iterator endIt = initStr.end();
  bool parseRes = qi::phrase_parse( startIt, endIt,
#if !defined(__clang__) && __GNUC__ <= 4 && __GNUC_MINOR__ < 9
    // NOTE: the additional pair of parentheses around
    // qi::real_parser<ElementType>() is to prevent a GCC
    // parsing bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55535
    // which has been reported to be fixed in GCC 4.9
    // Ironically, this workaround triggers an error in llvm/clang used in XCode, which also claimes to be __GNUC__.
    // TODO: Remove conditional code after minimum compiler
    // requirement is GCC >= 4.9. 
(*((qi::real_parser<typename TranslateType< ElementType>::type>())[phoenix::push_back( phoenix::ref( v ), qi::_1 )]
#else
(*(qi::real_parser<typename TranslateType< ElementType>::type >()[phoenix::push_back( phoenix::ref( v ), qi::_1 )]
#endif
  % -qi::char_( ',' )) >> *(qi::char_( '%' ) >> *qi::char_)),
  boost::spirit::ascii::space );
  if( !parseRes or (startIt != endIt) )
  {
    throw std::invalid_argument( "VectorParameter::fromStream(): Error while parsing line." );
  }
  std::size_t const numElements = v.size();
  VectorParameter<ElementType> result( numElements, alignment );
  if( numElements > 0 )
  {
    std::copy( &v[0], &v[0] + v.size(), result.data() );
  }
  return result;
}

template< typename ElementType >
/*static*/ VectorParameter<ElementType>
VectorParameter<ElementType>::fromStream( std::istream & stream, std::size_t alignment /*= 0*/ )
{
  std::ostringstream os;
  os <<stream.rdbuf();
  return VectorParameter<ElementType>::fromString( os.str(), alignment );
}

#ifdef VISR_PML_USE_SNDFILE_LIBRARY
namespace // unnamed
{
  /**
  * Template function for reading from a sndfile to a specific data
  * format.
  * Specialisations exist for double and float to call the correct
  * version of sf_read_<datatype>.
  * The template itself has no implementation, so instantiating it
  * with a type other than float or double will cause a compilation
  * error.
  */
  template<typename ElementType>

  /**
  * Specialisation for float.
  */
  sf_count_t read_samples( SNDFILE * file, VectorParameter<ElementType> & result, std::size_t numElements );
  template<> sf_count_t read_samples<float>( SNDFILE * file, VectorParameter<float> & result, std::size_t numElements )
  {
    return sf_read_float( file, result.data(), static_cast<sf_count_t>(numElements) );
  }

  /**
  * Specialisation for double.
  */
  template<> sf_count_t read_samples<double>( SNDFILE * file, VectorParameter<double> & result, std::size_t numElements )
  {
    return sf_read_double( file, result.data(), static_cast<sf_count_t>(numElements) );
  }

  /**
  * Implementation for complex types, to be used in the template specialisations.
  * Sample data is interpreted as real-only float sequences.
  */
  template<typename T>
  sf_count_t read_samples_complex( SNDFILE * file, VectorParameter<std::complex<T> > & result, std::size_t numElements )
  {
    VectorParameter<T> tmp( result.size(), result.alignmentElements() );
    sf_count_t numEl = read_samples<T>( file, tmp, static_cast<sf_count_t>(numElements) );
    std::copy( tmp.data(), tmp.data() + tmp.size(), result.data() );
    return numEl;
  }

  template<>
  sf_count_t read_samples<std::complex<float> >( SNDFILE * file, VectorParameter<std::complex<float> > & result, std::size_t numElements )
  {
    return read_samples_complex( file, result, numElements );
  }

  template<>
  sf_count_t read_samples<std::complex<double> >( SNDFILE * file, VectorParameter<std::complex<double> > & result, std::size_t numElements )
  {
    return read_samples_complex( file, result, numElements );
  }

} // unnamed namespace

template< typename ElementType >
/*static*/ VectorParameter<ElementType>
VectorParameter<ElementType>::fromAudioFile( std::string const & fileName, std::size_t alignment /*= 0*/ )
{
  boost::filesystem::path file( fileName );
  if( not exists( file ) or is_directory( file ) )
  {
    throw std::invalid_argument( "VectorParameter::fromAudioFile(): The specified file name does not exist." );
  }
  SF_INFO fileInfo;
  SNDFILE* fileHandle = nullptr;
  fileHandle = sf_open( file.string().c_str(), SFM_READ, &fileInfo );
  if( not fileHandle )
  {
    throw std::invalid_argument( "VectorParameter::fromAudioFile(): The specified file name could not be opened." );
  }
  try
  {
    std::size_t const numRows = static_cast<std::size_t>(fileInfo.channels);
    if( numRows != 1 )
    {
      throw std::invalid_argument( "VectorParameter::fromAudioFile(): Number of channels differs from 1." );
    }
    std::size_t numElements = static_cast<std::size_t>(fileInfo.frames);
    VectorParameter<ElementType> resultVec( numElements, alignment );

    sf_count_t const numReadRaw = read_samples<ElementType>( fileHandle, resultVec, numElements );
    if( numReadRaw < 0 )
    {
      std::invalid_argument( "VectorParameter::fromAudioFile(): Reading the audio data failed." );
    }
    std::size_t const numRead = static_cast<std::size_t>(numReadRaw);
    if( numRead != numElements )
    {
      std::invalid_argument( "VectorParameter::fromAudioFile(): Reading the audio data returned a wrong number of arguments." );
    }
    sf_close( fileHandle );
    fileHandle = nullptr;
    return resultVec;
  }
  catch( std::exception const & /*ex*/ )
  {
    sf_close( fileHandle );
    fileHandle = nullptr;
    throw; // rethrow, i.e., pass the exception to the next level.
  }
}
#endif

template< typename ElementType >
/*static*/ VectorParameter<ElementType>
VectorParameter<ElementType>::fromTextFile( std::string const & fileName, std::size_t alignment /*= 0*/ )
{
  boost::filesystem::path file( fileName );
  if( not exists( file ) or is_directory( file ) )
  {
    throw std::invalid_argument( "VectorParameter::fromTextFile(): The specified file name does not exist." );
  }
  std::ifstream stream( file.string(), std::ios_base::in );
  if( not stream )
  {
    throw std::invalid_argument( "VectorParameter::fromTextFile(): Opening file for reading failed." );
  }
  return fromStream( stream, alignment );
}


// Explicit instantiations for element types float and double and the corresponding complex types.
template class VectorParameter<float>;
template class VectorParameter<double>;
template class VectorParameter<std::complex<float> >;
template class VectorParameter<std::complex<double> >;

} // namespace pml
} // namespace visr
