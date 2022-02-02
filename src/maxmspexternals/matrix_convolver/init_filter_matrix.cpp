/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "init_filter_matrix.hpp"

#include <libpml/matrix_parameter.hpp>

#include <librbbl/index_sequence.hpp>

#include <boost/algorithm/string.hpp>

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit/include/qi.hpp>

#include <memory>
#include <numeric>
#include <string>
#include <vector>

namespace visr
{
namespace maxmsp
{
namespace matrix_convolver
{
namespace // unnamed
{
/**
 * Helper function to partition a comma-separated list of strings into a vector of strings.
 */
std::vector<std::string> stringListToVec( std::string const & list )
{
  if( list.empty( ) ) // Basically if this option is not provided.
  {
    return std::vector<std::string>();
  }
  std::vector<std::string> result;
  boost::split( result, list, boost::is_any_of( "," ) );
  // Simple version: remove only leading and trailing spaces around file names
  // std::for_each( result.begin(), result.end(), []( std::string& s ) { boost::algorithm::trim( s ); } );
  // Remove spaces and quotation marks around file names.
  std::for_each( result.begin( ), result.end( ),
    []( std::string& s ) { boost::algorithm::trim_if( s, boost::is_any_of( "\" " ) ); } );
  // result.erase( std::remove_if( result.begin(), result.end(), []( const std::string & s ) { return s.empty(); } );
  return result;
}

} // unnamed namespace

template< typename DataType >
void initFilterMatrix( std::string const & filterList,
                       std::size_t maxFilterLength,
                       std::size_t maxFilterEntries,
                       rbbl::IndexSequence const & indexOffsets,
                       efl::BasicMatrix<DataType> & matrix )
{
  std::vector<std::string> const filterNames = stringListToVec( filterList );
  std::size_t const numFilterFiles = filterNames.size();
  bool const offsetsProvided = indexOffsets.size() > 0;
  if( offsetsProvided )
  {
    if( indexOffsets.size() != numFilterFiles )
    {
      throw std::invalid_argument( "MatrixConvolver::initFilter(): If the parameter \"filterFileIndexOffsets\" is provided, "
        "it must match the number of filter files." );
    }
  }
  if( numFilterFiles == 0 )
  {
    if( maxFilterLength == std::numeric_limits<std::size_t>::max()
      or maxFilterEntries == std::numeric_limits<std::size_t>::max() )
    {
      throw std::invalid_argument( "MatrixConvolver::initFilter(): If no filter files are provided, the parameters "
        "\"maxFilterLength\" and \"maxFilters\" must be provided." );
    }
    matrix.resize( maxFilterEntries, maxFilterLength );
    return;
  }
  std::vector< std::unique_ptr<pml::MatrixParameter<DataType> > > allMatrices;

  // First run: Load files to gather information.
  using IndexVec = std::vector<std::size_t>;
  IndexVec lengths( numFilterFiles, 0 );
  IndexVec numChannels( numFilterFiles, 0 );
  IndexVec startIndex( numFilterFiles, 0 );
  IndexVec endIndex( numFilterFiles, 0 ); // STL convention: one past end
  for( std::size_t fileIdx( 0 ); fileIdx < numFilterFiles; ++fileIdx )
  {
    boost::filesystem::path const filePath = absolute( boost::filesystem::path( filterNames[fileIdx] ) );
    if( not exists( filePath ) )
    {
      throw std::invalid_argument( std::string( "MatrixConvolver::initFilter(): The file \"" ) + filePath.string()
        + std::string( "\" does not exist.") );
    }
#ifdef VISR_PML_USE_SNDFILE_LIBRARY
    pml::MatrixParameter<DataType> const mat
      = pml::MatrixParameter<DataType>::fromAudioFile( filePath.string( ), matrix.alignmentElements( ) );
    lengths[fileIdx] = mat.numberOfColumns();
    numChannels[fileIdx] = mat.numberOfRows();

    if( offsetsProvided )
    {
      startIndex[fileIdx] = indexOffsets[fileIdx];
    }
    else
    {
      startIndex[fileIdx] = (fileIdx == 0) ? 0 : endIndex[fileIdx - 1];
    }
    endIndex[fileIdx] = startIndex[fileIdx] + numChannels[fileIdx];
#else
    throw std::logic_error( "MatrixConvolver: Loading of filter files requires that VISR is built with option BUILD_USE_SNDFILE_LIBRARY." );
#endif
  }
  // Check and set the final filter length
  IndexVec::const_iterator maxLengthIt = std::max_element( lengths.begin(), lengths.end() );
  assert( maxLengthIt != lengths.end() ); // Empty file name vectors are handled above.
  std::size_t const maxFilterFileLength = *maxLengthIt;
  if( maxFilterLength != std::numeric_limits<std::size_t>::max() // special value? 
    and( maxFilterFileLength > maxFilterLength ) )
  {
    throw std::invalid_argument( "MatrixConvolver::initFilter(): A provided impulse response exceeds the length specified by the \"maxFilterLength\" parameter." );
  }
  std::size_t const filterLength = (maxFilterLength != std::numeric_limits<std::size_t>::max())
    ? maxFilterLength : maxFilterFileLength;

  // Check whether the index ranges for the files are non-overlapping 
  // Normally, this can happen only if "filterFileIndexOffsets" is specified, 
  // but we test the automatic assignment as well.
  
  // Get a index vector containing the file indices in ascending order of their start index.
  IndexVec orderByStartIndex( numFilterFiles );
  std::iota( orderByStartIndex.begin(), orderByStartIndex.end(), 0 ); // fill with 0,1,..,numFilterFiles-1
  // Reorder the index vector
  std::sort( orderByStartIndex.begin(), orderByStartIndex.end(),
    [&startIndex]( std::size_t a, std::size_t b ) { return startIndex[a] < startIndex[b]; } );
  for( std::size_t runIdx( 1 ); runIdx < numFilterFiles; ++runIdx )
  {
    if( startIndex[orderByStartIndex[runIdx] - 1] > endIndex[orderByStartIndex[runIdx]] ) // "==" is legal due to the one-past-end convention of endIndex
    {
      throw std::invalid_argument( "MatrixConvolver::initFilter(): The index ranges of the filters overlap." );
    }
  }
  std::size_t const maxUsedIndex = endIndex.back(); // one past end
  if( maxFilterEntries != std::numeric_limits<std::size_t>::max( ) // special value? 
    and( maxUsedIndex > maxFilterEntries ) )
  {
    throw std::invalid_argument( "MatrixConvolver::initFilter(): The maximum filter index exceeds the value specified by the \"max-filters\" parameter." );
  }
  std::size_t const numFilterEntries = (maxFilterEntries != std::numeric_limits<std::size_t>::max())
    ? maxFilterEntries : maxUsedIndex;

  matrix.resize( numFilterEntries, filterLength ); // Zeros all elements
  // Second run: Set the impulse responses
  for( std::size_t fileIdx( 0 ); fileIdx < numFilterFiles; ++fileIdx )
  {
    boost::filesystem::path const filePath = absolute( boost::filesystem::path( filterNames[fileIdx] ) );
    // File existence already checked.
#ifdef VISR_PML_USE_SNDFILE_LIBRARY
    pml::MatrixParameter<DataType> const mat
      = pml::MatrixParameter<DataType>::fromAudioFile( filePath.string(), matrix.alignmentElements() );
    for( std::size_t irIdx( 0 ); irIdx < mat.numberOfRows(); ++irIdx )
    {
      std::size_t const matrixIdx = startIndex[fileIdx] + irIdx;
      if( efl::vectorCopy( mat.row( irIdx ), matrix.row( matrixIdx ),
                           mat.numberOfColumns(), matrix.alignmentElements() ) != efl::noError )
      {
        throw std::invalid_argument( "MatrixConvolver::initFilter(): Error while copying impulse response" );
      }
    }
#else
    assert( false and "Logical error: Missing build option BUILD_USE_SNDFILE_LIBRARY library must have been detected above." );
#endif
  }
}

// Explicit instantiation
template void initFilterMatrix<float>( std::string const&, std::size_t, std::size_t,
                                       rbbl::IndexSequence const &, efl::BasicMatrix<float> & );
template void initFilterMatrix<double>( std::string const&, std::size_t, std::size_t,
                                        rbbl::IndexSequence const &, efl::BasicMatrix<double> & );


} // namespace matrix_convolver
} // namespace maxmsp
} // namespace visr
