/* Copyright Institue of Sound and Vibration Research - All rights reserved. */

#include <librbbl/biquad_coefficient.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <algorithm>
#include <ciso646>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace visr
{
namespace rbbl
{
namespace test
{

BOOST_AUTO_TEST_CASE( InstantiateBiquadDefault )
{
  using namespace visr::rbbl;
  using CoeffType = float;

  BiquadCoefficient<CoeffType> const biq;

  BOOST_CHECK_CLOSE( biq.b0( ), 1.0f, std::numeric_limits<CoeffType>::epsilon() );
  BOOST_CHECK_CLOSE( biq.b1( ), 0.0f, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b2( ), 0.0f, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a1( ), 0.0f, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a2( ), 0.0f, std::numeric_limits<CoeffType>::epsilon( ) );
}

BOOST_AUTO_TEST_CASE( InstantiateBiquadInitList )
{
  using namespace visr::rbbl;
  using CoeffType = float;

  CoeffType const b0 = 0.1f;
  CoeffType const b1 = 0.23f;
  CoeffType const b2 = -0.785f;
  CoeffType const a1 = 0.5f;
  CoeffType const a2 = 0.25f;

  BiquadCoefficient<CoeffType> const biq( { b0, b1, b2, a1, a2 } );
  BOOST_CHECK_CLOSE( biq.b0( ), b0, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b1( ), b1, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b2( ), b2, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a1( ), a1, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a2( ), a2, std::numeric_limits<CoeffType>::epsilon( ) );
}

BOOST_AUTO_TEST_CASE( InstantiateBiquadFromXml )
{
  using namespace visr::rbbl;
  using CoeffType = float;

  std::string const xmlRepr = "<biquad b0=\"0.2713121e-4\" b1=\"0.5426241e-4\" b2=\"0.2713121e-4\" a1=\"-1.9792\" a2=\"0.9793\" />";
  std::stringstream str( xmlRepr );

  BiquadCoefficient<CoeffType> const biq = BiquadCoefficient<CoeffType>::fromXml( str );

  CoeffType const b0 = 0.2713121e-4f;
  CoeffType const b1 = 0.5426241e-4f;
  CoeffType const b2 = 0.2713121e-4f;
  CoeffType const a1 = -1.9792f;
  CoeffType const a2 = 0.9793f;

  BOOST_CHECK_CLOSE( biq.b0( ), b0, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b1( ), b1, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b2( ), b2, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a1( ), a1, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a2( ), a2, std::numeric_limits<CoeffType>::epsilon( ) );
}

BOOST_AUTO_TEST_CASE( InstantiateBiquadFromJson )
{
  using namespace visr::rbbl;
  using CoeffType = float;

  std::string const jsonRepr = "{ \"b0\":0.2713121e-4, \"b1\":0.5426241e-4, \"b2\": 0.2713121e-4, \"a1\":-1.9792, \"a2\":0.9793 }";
  std::stringstream str( jsonRepr );

  BiquadCoefficient<CoeffType> const biq = BiquadCoefficient<CoeffType>::fromJson( str );

  CoeffType const b0 = 0.2713121e-4f;
  CoeffType const b1 = 0.5426241e-4f;
  CoeffType const b2 = 0.2713121e-4f;
  CoeffType const a1 = -1.9792f;
  CoeffType const a2 = 0.9793f;

  BOOST_CHECK_CLOSE( biq.b0( ), b0, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b1( ), b1, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b2( ), b2, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a1( ), a1, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a2( ), a2, std::numeric_limits<CoeffType>::epsilon( ) );
}

BOOST_AUTO_TEST_CASE( InstantiateBiquadFromJsonNonStandardA0 )
{
  using namespace visr::rbbl;
  using CoeffType = float;

  std::string const jsonRepr = "{ \"b0\":0.2713121e-4, \"b1\":0.5426241e-4, \"b2\": 0.2713121e-4, \"a0\": 0.3333333, \"a1\":-1.9792, \"a2\":0.9793 }";
  std::stringstream str( jsonRepr );

  BiquadCoefficient<CoeffType> const biq = BiquadCoefficient<CoeffType>::fromJson( str );

  CoeffType const a0 = 0.3333333f;
  CoeffType const b0 = 0.2713121e-4f;
  CoeffType const b1 = 0.5426241e-4f;
  CoeffType const b2 = 0.2713121e-4f;
  CoeffType const a1 = -1.9792f;
  CoeffType const a2 = 0.9793f;

  // The parsed coefficients are normalised by a0.
  BOOST_CHECK_CLOSE( biq.b0( ), b0 / a0, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b1( ), b1 / a0, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.b2( ), b2 / a0, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a1( ), a1 / a0, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( biq.a2( ), a2 / a0, std::numeric_limits<CoeffType>::epsilon( ) );
}

BOOST_AUTO_TEST_CASE( InstantiateListFromXml )
{
  using CoeffType = float;

  std::string const xmlStr( "<filterSpec name=\"lowpass80Hz\">"
   "<biquad b0=\"0.2713121e-4\" b1=\"0.5426241e-4\" b2=\"0.2713121e-4\" a1=\"-1.9792\" a2=\"0.9793\" />"
   "<biquad b0=\"0.2713121e-2\" b1=\"0.5426241e-2\" b2=\"0.2713121e-2\" a1=\"1.9792\" a2=\"-0.9793\" />"
   "</filterSpec>" );

  visr::rbbl::BiquadCoefficientList<CoeffType> const biqList = visr::rbbl::BiquadCoefficientList<CoeffType>::fromXml( xmlStr );

  BOOST_CHECK( biqList.size() == 2 );
  visr::rbbl::BiquadCoefficient<CoeffType> const & bi0 = biqList[0];
  visr::rbbl::BiquadCoefficient<CoeffType> const & bi1 = biqList[1];

  CoeffType const b00 = 0.2713121e-4f;
  CoeffType const b01 = 0.5426241e-4f;
  CoeffType const b02 = 0.2713121e-4f;
  CoeffType const a01 = -1.9792f;
  CoeffType const a02 = 0.9793f;
  CoeffType const b10 = 0.2713121e-2f;
  CoeffType const b11 = 0.5426241e-2f;
  CoeffType const b12 = 0.2713121e-2f;
  CoeffType const a11 = 1.9792f;
  CoeffType const a12 = -0.9793f;

  BOOST_CHECK_CLOSE( bi0.b0( ), b00, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.b1( ), b01, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.b2( ), b02, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.a1( ), a01, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.a2( ), a02, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.b0( ), b10, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.b1( ), b11, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.b2( ), b12, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.a1( ), a11, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.a2( ), a12, std::numeric_limits<CoeffType>::epsilon( ) );

}

BOOST_AUTO_TEST_CASE( InstantiateBiquadListFromJson )
{
  using CoeffType = float;

  std::string const jsonStr( "["
    "{ \"b0\": 0.2713121e-4, \"b1\": 0.5426241e-4, \"b2\": 0.2713121e-4, \"a1\": -1.9792, \"a2\": 0.9793 }, "
    "{ \"b0\": 0.2713121e-2, \"b1\": 0.5426241e-2, \"b2\": 0.2713121e-2, \"a1\": 1.9792, \"a2\": -0.9793 }"
    "]"  );

  visr::rbbl::BiquadCoefficientList<CoeffType> const biqList = visr::rbbl::BiquadCoefficientList<CoeffType>::fromJson( jsonStr );

  BOOST_CHECK( biqList.size( ) == 2 );
  visr::rbbl::BiquadCoefficient<CoeffType> const & bi0 = biqList[0];
  visr::rbbl::BiquadCoefficient<CoeffType> const & bi1 = biqList[1];

  CoeffType const b00 = 0.2713121e-4f;
  CoeffType const b01 = 0.5426241e-4f;
  CoeffType const b02 = 0.2713121e-4f;
  CoeffType const a01 = -1.9792f;
  CoeffType const a02 = 0.9793f;
  CoeffType const b10 = 0.2713121e-2f;
  CoeffType const b11 = 0.5426241e-2f;
  CoeffType const b12 = 0.2713121e-2f;
  CoeffType const a11 = 1.9792f;
  CoeffType const a12 = -0.9793f;

  BOOST_CHECK_CLOSE( bi0.b0( ), b00, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.b1( ), b01, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.b2( ), b02, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.a1( ), a01, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi0.a2( ), a02, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.b0( ), b10, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.b1( ), b11, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.b2( ), b12, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.a1( ), a11, std::numeric_limits<CoeffType>::epsilon( ) );
  BOOST_CHECK_CLOSE( bi1.a2( ), a12, std::numeric_limits<CoeffType>::epsilon( ) );
}

// ============================================================================
// BiquadCoefficientMatrix

BOOST_AUTO_TEST_CASE( InstantiateBiquadMatrixFromJson )
{
  using CoeffType = float;

  std::string const jsonStr( "[ ["
    "{ \"b0\": 0.2713121e-4, \"b1\": 0.5426241e-4, \"b2\": 0.2713121e-4, \"a1\": -1.9792, \"a2\": 0.9793 }, "
    "{ \"b0\": 0.2713121e-2, \"b1\": 0.5426241e-2, \"b2\": 0.2713121e-2, \"a1\": 1.9792, \"a2\": -0.9793 }"
    "], ["
    "{ \"b0\": 0.2713121e-4, \"b1\": 0.5426241e-4, \"b2\": 0.2713121e-4, \"a1\": -1.9792, \"a2\": 0.9793 }, "
    "{ \"b0\": 0.2713121e-2, \"b1\": 0.5426241e-2, \"b2\": 0.2713121e-2, \"a1\": 1.9792, \"a2\": -0.9793 }"
    "], ["
    "{ \"b0\": 0.2713121e-4, \"b1\": 0.5426241e-4, \"b2\": 0.2713121e-4, \"a1\": -1.9792, \"a2\": 0.9793 }, "
    "{ \"b0\": 0.2713121e-2, \"b1\": 0.5426241e-2, \"b2\": 0.2713121e-2, \"a1\": 1.9792, \"a2\": -0.9793 }"
    "] ]" );

  visr::rbbl::BiquadCoefficientMatrix<CoeffType> const biqMtx = visr::rbbl::BiquadCoefficientMatrix<CoeffType>::fromJson( jsonStr );

  BOOST_CHECK( biqMtx.numberOfFilters() == 3 );
  BOOST_CHECK( biqMtx.numberOfSections() == 2 );
}


} // namespace test
} // namespace rbbl
} // namespace visr
