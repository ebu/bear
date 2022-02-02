/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "matrix_parameter.hpp"

#include <libvisr/constants.hpp>
#include <libvisr/parameter_factory.hpp>
#include <libvisr/parameter_type.hpp>

#ifdef VISR_PML_USE_SNDFILE_LIBRARY
#include <sndfile.h>
#endif

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <ciso646>
#include <complex>
#include <fstream>
#include <limits>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <vector>

namespace visr
{
namespace pml
{

template< typename ElementType >
MatrixParameter<ElementType>::MatrixParameter( std::size_t alignment /*= 0*/ )
  : efl::BasicMatrix<ElementType>( alignment )
{
}

template< typename ElementType >
MatrixParameter<ElementType>::MatrixParameter( std::size_t numRows, std::size_t numColumns, std::size_t alignment /*= 0*/ )
  : efl::BasicMatrix<ElementType>( numRows, numColumns, alignment )
{
}

template< typename ElementType >
MatrixParameter<ElementType>::MatrixParameter( std::size_t numRows, std::size_t numColumns,
                                               std::initializer_list<std::initializer_list<ElementType> > const & initMtx,
                                               std::size_t alignment /*= 0*/ )
  : efl::BasicMatrix< ElementType >( numRows, numColumns, initMtx, alignment )
{
}

template< typename ElementType >
MatrixParameter<ElementType>::MatrixParameter( MatrixParameterConfig const & config )
: MatrixParameter<ElementType>( config.numberOfRows( ), config.numberOfColumns( ), cVectorAlignmentSamples )
{
}

template< typename ElementType >
MatrixParameter<ElementType>::MatrixParameter( ParameterConfigBase const & config )
: MatrixParameter( dynamic_cast<MatrixParameterConfig const &>(config) )
{
  // Todo: handle exceptions
}

template< typename ElementType >
MatrixParameter<ElementType>::MatrixParameter( MatrixParameter<ElementType> const & rhs )
: MatrixParameter( rhs.numberOfRows(), rhs.numberOfColumns(), rhs.alignmentElements() )
{
  efl::BasicMatrix<ElementType>::copy( rhs );
}

template< typename ElementType >
MatrixParameter<ElementType>::~MatrixParameter() = default;

template< typename ElementType >
MatrixParameter<ElementType>& MatrixParameter<ElementType>::operator=(MatrixParameter<ElementType> const & rhs)
{
  // TODO: Check compatible dimensions
  efl::BasicMatrix<ElementType>::copy( rhs );
  return *this;
}


template< typename ElementType >
void MatrixParameter<ElementType>::resize( std::size_t numRows, std::size_t numColumns )
{
  efl::BasicMatrix< ElementType >::resize( numRows, numColumns );
}


template< typename ElementType >
/*static*/ MatrixParameter<ElementType>
MatrixParameter<ElementType>::fromString( std::string const & textMatrix, std::size_t alignment /*= 0*/ )
{
  std::stringstream stream( textMatrix );
  return fromStream( stream, alignment );
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
/*static*/ MatrixParameter<ElementType>
MatrixParameter<ElementType>::fromStream( std::istream & stream, std::size_t alignment /*= 0*/ )
{
  namespace qi = boost::spirit::qi;
  namespace phoenix = boost::phoenix;

  std::vector< std::vector<ElementType> > tmpStorage;
  std::size_t minSize = std::numeric_limits<std::size_t>::max( );
  std::size_t maxSize = 0;
  while( not stream.eof() )
  {
    if( not stream.good( ) )
    {
      throw std::invalid_argument( "MatrixParameter::fromStream(): Error while reading line." );
    }
    std::string currLine;
    std::getline( stream, currLine );
    for( std::size_t strPos( 0 ); strPos != std::string::npos; /* strPos is updated in the loop.*/ )
    {
      std::size_t lineEndPos = currLine.find_first_of( std::string(";"), strPos );

      std::vector<ElementType> v;
      // Have to convert the indices returned by find_first_of into iterators
      std::string::const_iterator startIt = currLine.begin() + strPos;
      std::string::const_iterator endIt =
        (lineEndPos == std::string::npos) ? currLine.end() : currLine.begin( ) + lineEndPos;
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
        (*(qi::real_parser<typename TranslateType< ElementType>::type >( )[phoenix::push_back( phoenix::ref( v ), qi::_1 )]
#endif
        % -qi::char_( ',' )) >> *(qi::char_( '%' ) >> *qi::char_)),
        boost::spirit::ascii::space);
      if( !parseRes or( startIt != endIt ) )
      {
        throw std::invalid_argument( "MatrixParameter::fromStream(): Error while parsing line." );
      }
      std::size_t const numEl = v.size( );
      if( numEl != 0 ) // ignore empty lines (might consist purely of comments)
      {
        tmpStorage.push_back( v );
        minSize = std::min( minSize, numEl );
        maxSize = std::max( maxSize, numEl );
      }
      strPos = (lineEndPos == std::string::npos) ? lineEndPos : lineEndPos + 1;
    }
  }
  if( (not tmpStorage.empty()) and (minSize != maxSize) ) // Empty arrays are legal.
  {
    throw std::invalid_argument( "MatrixParameter::fromStream(): Matrix rows (text lines) have different numbers of elements." );
  }
  if( tmpStorage.size() == 0 )
  {
    // Zero rows means empty matrix (which is legal)
    return MatrixParameter<ElementType>( alignment );
  }
  std::size_t const numRows = tmpStorage.size();
  std::size_t const numCols = minSize;
  MatrixParameter<ElementType> result( numRows, numCols, alignment );
  for( std::size_t rowIdx( 0 ); rowIdx < numRows; ++rowIdx )
  {
    std::vector<ElementType> const & v( tmpStorage[rowIdx] );
    std::copy( v.begin(), v.end(), result.row( rowIdx ) );
  }
  return result;
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
  sf_count_t read_samples( SNDFILE * file, efl::AlignedArray<ElementType> & result, std::size_t numElements );
  template<> sf_count_t read_samples<float>( SNDFILE * file, efl::AlignedArray<float> & result, std::size_t numElements )
  {
    return sf_read_float( file, result.data(), static_cast<sf_count_t>(numElements) );
  }

  /**
   * Specialisation for double.
   */
  template<> sf_count_t read_samples<double>( SNDFILE * file, efl::AlignedArray<double> & result, std::size_t numElements )
  {
    return sf_read_double( file, result.data( ), static_cast<sf_count_t>(numElements) );
  }

  /**
   * Implementation for complex types, to be used in the template specialisations.
   * Sample data is interpreted as real-only float sequences.
   */
  template<typename T>
  sf_count_t read_samples_complex( SNDFILE * file, efl::AlignedArray<std::complex<T> > & result, std::size_t numElements )
  {
    efl::AlignedArray<T> tmp( result.size(), result.alignmentElements() );
    sf_count_t numEl = read_samples<T>( file, tmp, static_cast<sf_count_t>(numElements) );
    std::copy( tmp.data(), tmp.data() + tmp.size(), result.data() );
    return numEl;
  }

  template<>
  sf_count_t read_samples<std::complex<float>>( SNDFILE * file, efl::AlignedArray<std::complex<float> > & result, std::size_t numElements )
  {
    return read_samples_complex( file, result, numElements );
  }

  template<>
  sf_count_t read_samples<std::complex<double>>( SNDFILE * file, efl::AlignedArray<std::complex<double> > & result, std::size_t numElements )
  {
    return read_samples_complex( file, result, numElements );
  }

} // unnamed namespace

template< typename ElementType >
/*static*/ MatrixParameter<ElementType>
MatrixParameter<ElementType>::fromAudioFile( std::string const & fileName, std::size_t alignment /*= 0*/ )
{
  boost::filesystem::path file( fileName );
  if( not exists( file ) or is_directory( file ) )
  {
    throw std::invalid_argument( "MatrixParameter::fromAudioFile(): The specified file name does not exist." );
  }
  SF_INFO fileInfo;
  SNDFILE* fileHandle = nullptr;
  fileHandle = sf_open( file.string().c_str(), SFM_READ, &fileInfo );
  if( not fileHandle )
  {
    throw std::invalid_argument( "MatrixParameter::fromAudioFile(): The specified file name could not be opened." );
  }
  try
  {
    std::size_t const numRows = static_cast<std::size_t>(fileInfo.channels);
    std::size_t numCols = static_cast<std::size_t>(fileInfo.frames);
    MatrixParameter<ElementType> resultMtx( numRows, numCols, alignment );

    efl::AlignedArray<ElementType> tmpStorage( numRows*numCols, alignment );
    sf_count_t const numReadRaw = read_samples<ElementType>( fileHandle, tmpStorage, numRows*numCols );
    if( numReadRaw < 0 )
    {
      std::invalid_argument( "MatrixParameter::fromAudioFile(): Reading the audio data failed." );
    }
    std::size_t const numRead = static_cast<std::size_t>(numReadRaw);
    if( numRead != numRows * numCols )
    {
      std::invalid_argument( "MatrixParameter::fromAudioFile(): Reading the audio data reeturned a wrong number of arguments." );
    }
    // De-interleaving, might be implemented in a library function
    for( std::size_t rowIdx(0); rowIdx < numRows; ++rowIdx )
    {
      ElementType * rowData = resultMtx.row( rowIdx );
      for( std::size_t colIdx(0); colIdx < numCols; ++colIdx )
      {
        *(rowData + colIdx) = tmpStorage[colIdx * numRows + rowIdx];
      }
    }
    sf_close( fileHandle );
    fileHandle = nullptr;
    return resultMtx;
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
/*static*/ MatrixParameter<ElementType>
MatrixParameter<ElementType>::fromTextFile( std::string const & fileName, std::size_t alignment /*= 0*/ )
{
  boost::filesystem::path file( fileName );
  if( not exists( file ) or is_directory( file ) )
  {
    throw std::invalid_argument( "MatrixParameter::fromTextFile(): The specified file name does not exist." );
  }
  std::ifstream stream( file.string(), std::ios_base::in );
  if( not stream )
  {
    throw std::invalid_argument( "MatrixParameter::fromTextFile(): Opening file for reading failed." );
  }
  return fromStream( stream, alignment );
}

// Explicit instantiations for sample types float and double
template class MatrixParameter<float>;
template class MatrixParameter<double>;
template class MatrixParameter<std::complex<float> >;
template class MatrixParameter<std::complex<double> >;

} // namespace pml
} // namespace visr
