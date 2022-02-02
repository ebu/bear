/* Copyright Institue of Sound and Vibration Research - All rights reserved. */


#include <libefl/basic_matrix.hpp>
#include <libvisr/constants.hpp>
#include <libpml/matrix_parameter.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ciso646>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace visr
{
namespace pml
{
namespace test
{

#ifdef VISR_PML_USE_SNDFILE_LIBRARY
BOOST_AUTO_TEST_CASE( readFiltersFromWavFile )
{
  using SampleType = float;
  std::size_t const alignElements = 8;

  boost::filesystem::path const baseDir( CMAKE_CURRENT_SOURCE_DIR );
  boost::filesystem::path const impulseWav = baseDir / boost::filesystem::path( "../../librbbl/test/fir/quasiAllpassFIR_f32_n63.wav" );
  BOOST_CHECK( exists( impulseWav ) and not is_directory( impulseWav ) );

  pml::MatrixParameter<SampleType> const impulses
    = pml::MatrixParameter<SampleType>::fromAudioFile( impulseWav.string( ), alignElements );
  BOOST_CHECK( impulses.numberOfRows() == 32 );
  BOOST_CHECK( impulses.numberOfColumns() == 63 );
}

BOOST_AUTO_TEST_CASE( readComplexMatrixFromWavFile )
{
  using SampleType = std::complex<float>;
  std::size_t const alignElements = 8;

  boost::filesystem::path const baseDir( CMAKE_CURRENT_SOURCE_DIR );
  boost::filesystem::path const impulseWav = baseDir / boost::filesystem::path( "../../librbbl/test/fir/quasiAllpassFIR_f32_n63.wav" );
  BOOST_CHECK( exists( impulseWav ) and not is_directory( impulseWav ) );

  pml::MatrixParameter<SampleType> const impulses
    = pml::MatrixParameter<SampleType>::fromAudioFile( impulseWav.string(), alignElements );
  BOOST_CHECK( impulses.numberOfRows() == 32 );
  BOOST_CHECK( impulses.numberOfColumns() == 63 );
}
#endif

BOOST_AUTO_TEST_CASE( readFiltersFromTextFile )
{
  using SampleType = double;
  std::size_t const alignElements = 8;

  boost::filesystem::path const baseDir( CMAKE_CURRENT_SOURCE_DIR );
  boost::filesystem::path const impulseTxt = baseDir / boost::filesystem::path( "../../librbbl/test/fir/quasiAllpassFIR_f32_n63.txt" );
  BOOST_CHECK( exists( impulseTxt ) and not is_directory( impulseTxt ) );

  pml::MatrixParameter<SampleType> const impulses
    = pml::MatrixParameter<SampleType>::fromTextFile( impulseTxt.string(), alignElements );

#if 0
  std::size_t irChannels = impulses.numberOfRows();
  std::size_t irLength = impulses.numberOfColumns();
  for( std::size_t rowIdx( 0 ); rowIdx < irChannels; ++rowIdx )
  {
    std::cout << "Filter " << rowIdx << ": ";
    std::copy( impulses.row( rowIdx ), impulses.row( rowIdx ) + irLength, std::ostream_iterator<SampleType>( std::cout, ", " ) );
    std::cout << std::endl;
  }
#endif
}

BOOST_AUTO_TEST_CASE( readFiltersFromStrings )
{
  using SampleType = double;
  std::size_t const alignElements = 8;

  std::string filterDef( " 0.0 0.75 -3.0\n 0.0, -1, +3.275 % comments are allowed \n \n    % Also lines with only comments" );

  pml::MatrixParameter<SampleType> const impulses
    = pml::MatrixParameter<SampleType>::fromString( filterDef, alignElements );

  std::size_t irChannels = impulses.numberOfRows( );
  std::size_t irLength = impulses.numberOfColumns( );

  for( std::size_t rowIdx( 0 ); rowIdx < irChannels; ++rowIdx )
  {
    std::cout << "Filter " << rowIdx << ": ";
    std::copy( impulses.row( rowIdx ), impulses.row( rowIdx ) + irLength, std::ostream_iterator<SampleType>( std::cout, ", " ) );
    std::cout << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( readSemicolonSeparatedMatrix )
{
  using SampleType = double;
  std::size_t const alignElements = 8;

  std::string filterDef( " 0.0 0.75 -3.0e-3; 0.0, -1, +3.275 % comments are allowed ;    % Also lines with only comments\n 1.0, -0.5, 0.333" );

  pml::MatrixParameter<SampleType> const impulses
    = pml::MatrixParameter<SampleType>::fromString( filterDef, alignElements );

  std::size_t irChannels = impulses.numberOfRows( );
  std::size_t irLength = impulses.numberOfColumns( );

  for( std::size_t rowIdx( 0 ); rowIdx < irChannels; ++rowIdx )
  {
    std::cout << "Filter " << rowIdx << ": ";
    std::copy( impulses.row( rowIdx ), impulses.row( rowIdx ) + irLength, std::ostream_iterator<SampleType>( std::cout, ", " ) );
    std::cout << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( readSemicolonSeparatedMatrixComplex )
{
  using SampleType = std::complex<double>;
  std::size_t const alignElements = 8;

  std::string filterDef( " 0.0 0.75 -3.0e-3; 0.0, -1, +3.275 % comments are allowed ;    % Also lines with only comments\n 1.0, -0.5, 0.333" );

  pml::MatrixParameter<SampleType> const impulses
    = pml::MatrixParameter<SampleType>::fromString( filterDef, alignElements );

  std::size_t irChannels = impulses.numberOfRows();
  std::size_t irLength = impulses.numberOfColumns();

  for( std::size_t rowIdx( 0 ); rowIdx < irChannels; ++rowIdx )
  {
    std::cout << "Filter " << rowIdx << ": ";
    std::copy( impulses.row( rowIdx ), impulses.row( rowIdx ) + irLength, std::ostream_iterator<SampleType>( std::cout, ", " ) );
    std::cout << std::endl;
  }
}

} // namespace test
} // namespace pml
} // namespace visr
