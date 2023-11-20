/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libobjectmodel/object.hpp>
#include <libobjectmodel/object_vector.hpp>
#include <libobjectmodel/object_vector_parser.hpp>
#include <libobjectmodel/point_source_with_reverb.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <ciso646>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace visr
{
namespace objectmodel
{
namespace test
{

BOOST_AUTO_TEST_CASE( ParsePointSourceWithReverb )
{
  boost::filesystem::path const jsonFileName = boost::filesystem::path( CMAKE_CURRENT_SOURCE_DIR ) / boost::filesystem::path("/data/point_source_with_reverb_1.json" );
  BOOST_CHECK( exists( jsonFileName ) and not is_directory( jsonFileName ) );
  std::ifstream jsonFileStr( jsonFileName.string().c_str() );
  BOOST_CHECK( jsonFileStr and not jsonFileStr.eof() );

  std::stringstream msgStr;
  msgStr << jsonFileStr.rdbuf();
  std::string const msg = msgStr.str();
  ObjectVector scene;

  BOOST_CHECK_NO_THROW( ObjectVectorParser::fillObjectVector( msg, scene ));

  std::cout << "Scene size: " << scene.size() << std::endl;

  BOOST_CHECK( scene.size() == 1 );

  // TODO: Move to separate unit test.

  std::stringstream outputMessage;

  BOOST_CHECK_NO_THROW( ObjectVectorParser::encodeObjectVector( scene, outputMessage ) );

  std::cout << outputMessage.str() << std::endl;
}

BOOST_AUTO_TEST_CASE( SerialisePointSourceWithReverb )
{
  boost::filesystem::path const jsonFileName = boost::filesystem::path( CMAKE_CURRENT_SOURCE_DIR ) / boost::filesystem::path( "/data/point_source_with_reverb_1.json" );
  BOOST_CHECK( exists( jsonFileName ) and not is_directory( jsonFileName ) );
  std::ifstream jsonFileStr( jsonFileName.string().c_str() );
  BOOST_CHECK( jsonFileStr and not jsonFileStr.eof() );

  std::stringstream msgStr;
  msgStr << jsonFileStr.rdbuf();
  std::string const msg = msgStr.str();
  ObjectVector scene;

  BOOST_CHECK_NO_THROW( ObjectVectorParser::fillObjectVector( msg, scene ) );

  std::cout << "Scene size: " << scene.size() << std::endl;

  BOOST_CHECK( scene.size() == 1 );

  // TODO: Move to separate unit test.

  std::stringstream outputMessage;

  BOOST_CHECK_NO_THROW( ObjectVectorParser::encodeObjectVector( scene, outputMessage ) );

  std::cout << "Re-serialised reverb object: " << outputMessage.str() << std::endl;

  ObjectVector newVec;

  BOOST_CHECK_NO_THROW( ObjectVectorParser::fillObjectVector( outputMessage.str(), newVec ) );

  // No check whether the sources are identical (or close for floating-point quantities)
}


} // namespace test
} // namespace objectmodel
} // namespce visr
