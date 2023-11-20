/* Copyright Institue of Sound and Vibration Research - All rights reserved. */


#include <libpml/vector_parameter.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ciso646>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace visr
{
namespace pml
{
namespace test
{

BOOST_AUTO_TEST_CASE( VectorParameterDefaultConstructor )
{
  using SampleType = float;

  VectorParameter<SampleType> v1{};

  BOOST_CHECK( v1.size() == 0 );
}

BOOST_AUTO_TEST_CASE( VectorParameterAlignmentConstructor )
{
  using SampleType = float;
  std::size_t const alignElements = 8;

  VectorParameter<SampleType> v1( alignElements );

  BOOST_CHECK( v1.size() == 0 );
  BOOST_CHECK( v1.alignmentElements() == alignElements );
}

BOOST_AUTO_TEST_CASE( VectorParameterSizeConstructor )
{
  using SampleType = float;
  std::size_t const vecSize = 5;
  std::size_t const alignElements = 8;

  VectorParameter<SampleType> v1( vecSize, alignElements );

  BOOST_CHECK( v1.size() == vecSize );
  BOOST_CHECK( v1.alignmentElements() == alignElements );

  for( std::size_t idx{0}; idx < vecSize; ++idx )
  {
    BOOST_CHECK( v1[idx] == static_cast<SampleType>( 0.0) );
  }
}

BOOST_AUTO_TEST_CASE( VectorParameterFromInitString )
{
  using SampleType = float;
  std::size_t const vecSize = 5;
  std::size_t const alignElements = 8;

  VectorParameter<SampleType> v1( { 0.5, 1.5, 2.5, 3.5 , 4.5 },  alignElements );

  BOOST_CHECK( v1.size() == vecSize );
  BOOST_CHECK( v1.alignmentElements() == alignElements );

  for( std::size_t idx{ 0 }; idx < vecSize; ++idx )
  {
    BOOST_CHECK_CLOSE( v1[idx], static_cast<SampleType>(idx) + 0.5, static_cast<SampleType>(1e-7) );
  }
}

BOOST_AUTO_TEST_CASE( VectorParameterFromString )
{
  using SampleType = float;
  std::size_t const vecSize = 5;
  std::size_t const alignElements = 8;

  std::string initStr = "0.5, 1.5,2.5    3.5   ,   4.5";

  VectorParameter<SampleType> v1 = 
    VectorParameter<SampleType>::fromString( initStr, alignElements );

  BOOST_CHECK( v1.size() == vecSize );
  BOOST_CHECK( v1.alignmentElements() == alignElements );

  for( std::size_t idx{ 0 }; idx < vecSize; ++idx )
  {
    BOOST_CHECK_CLOSE( v1[idx], static_cast<SampleType>(idx)+0.5, static_cast<SampleType>(1e-7) );
  }
}

BOOST_AUTO_TEST_CASE( VectorParameterFromStream )
{
  using SampleType = float;
  std::size_t const vecSize = 5;
  std::size_t const alignElements = 8;

  // irregular whitespace and commas to test parser.
  std::string const initStr = "0.5, 1.5,2.5    3.5   ,   4.5";

  std::stringstream stream;
  stream << initStr;

  VectorParameter<SampleType> v1 =
    VectorParameter<SampleType>::fromStream( stream, alignElements );

  BOOST_CHECK( v1.size() == vecSize );
  BOOST_CHECK( v1.alignmentElements() == alignElements );

  for( std::size_t idx{ 0 }; idx < vecSize; ++idx )
  {
    BOOST_CHECK_CLOSE( v1[idx], static_cast<SampleType>(idx) + 0.5, static_cast<SampleType>(1e-7) );
  }
}

BOOST_AUTO_TEST_CASE( VectorParameterFromTextFile )
{
  using SampleType = float;
  std::size_t const alignElements = 8;

  // Data created using Python:
  // import scipy.io.wavfile as wavio
  // import numpy as np
  // a = np.cos( np.arange( 16 ) / np.pi, dtype = np.float32 )
  // The reshape into a 2D matrix saves the vector in a single line.
  // np.savetxt( 'vector_data_float32_16elements.dat', np.reshape( a, (1,-1)) )

  boost::filesystem::path const fileName = boost::filesystem::path( CMAKE_CURRENT_SOURCE_DIR )
    / boost::filesystem::path( "data/vector_data_float32_16elements.dat" );

  std::vector<SampleType> refData{ 1.0f,  0.94976574f,  0.8041099f ,  0.57766616f,  0.2931853f,
    -0.02075158f, -0.33260357f, -0.61103934f, -0.82808477f, -0.96193385f,
    -0.9991388f, -0.93596154f, -0.77874976f, -0.5432979f, -0.25326186f,
    0.06221899f };

  VectorParameter<SampleType> v1 =
    VectorParameter<SampleType>::fromTextFile( fileName.string(), alignElements );

  BOOST_CHECK( v1.size() == refData.size() );
  BOOST_CHECK( v1.alignmentElements() == alignElements );

  // Avoid past-the-end accesses if the size check fails.
  for( std::size_t idx{ 0 }; idx < std::min( v1.size(), refData.size() ); ++idx )
  {
    BOOST_CHECK_CLOSE( v1[idx], refData[idx], static_cast<SampleType>(1e-4) );
  }
}

#ifdef VISR_PML_USE_SNDFILE_LIBRARY
BOOST_AUTO_TEST_CASE( VectorParameterFromWavFile )
{
  using SampleType = float;
  std::size_t const alignElements = 8;

  // Data created using Python:
  // import scipy.io.wavfile as wavio
  // import numpy as np
  // a = np.cos( np.arange( 16 ) / np.pi, dtype = np.float32 )
  // wavio.write( 'vector_data_float32_16elements.wav', 48000, a )

  boost::filesystem::path const fileName = boost::filesystem::path(CMAKE_CURRENT_SOURCE_DIR)
    / boost::filesystem::path( "data/vector_data_float32_16elements.wav");

  std::vector<SampleType> refData{ 1.0f,  0.94976574f,  0.8041099f ,  0.57766616f,  0.2931853f,
    -0.02075158f, -0.33260357f, -0.61103934f, -0.82808477f, -0.96193385f,
    -0.9991388f, -0.93596154f, -0.77874976f, -0.5432979f, -0.25326186f,
    0.06221899f };

  VectorParameter<SampleType> v1 =
    VectorParameter<SampleType>::fromAudioFile( fileName.string(), alignElements );

  BOOST_CHECK( v1.size() == refData.size() );
  BOOST_CHECK( v1.alignmentElements() == alignElements );

  // Avoid past-the-end accesses if the size check fails.
  for( std::size_t idx{ 0 }; idx < std::min(v1.size(),refData.size()); ++idx )
  {
    BOOST_CHECK_CLOSE( v1[idx], refData[idx], static_cast<SampleType>(1e-4) );
  }
}
#endif

} // namespace test
} // namespace pml
} // namespace visr
