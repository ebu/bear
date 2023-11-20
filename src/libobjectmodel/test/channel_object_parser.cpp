/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libobjectmodel/object.hpp>
#include <libobjectmodel/object_vector.hpp>
#include <libobjectmodel/object_vector_parser.hpp>
#include <libobjectmodel/channel_object.hpp>
#include <libobjectmodel/channel_object_parser.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <stdexcept>
#include <iostream>

namespace visr
{
namespace objectmodel
{
namespace test
{

BOOST_AUTO_TEST_CASE( ParseChannelObject )
{
  std::string const msg = "{ \"objects\":[{\"id\": 1, \"channels\": \"0, 1, 3\", \n \"type\": \"channel\",\n \"group\": 0,\n \"priority\": 3,\n \"level\": 0.8,\n"
      "\"outputChannels\": \"M+000, U+030, B-045\" } ] }";

  ObjectVector scene;

  ObjectVectorParser::fillObjectVector( msg, scene );

  BOOST_CHECK( scene.size() == 1 );

  ChannelObject * co = nullptr ;
  auto & obj = scene.at( 1 );
  BOOST_CHECK_NO_THROW( co = &dynamic_cast<ChannelObject&>( obj ) );

  BOOST_CHECK( co->outputChannels().size() == 3 );
}

BOOST_AUTO_TEST_CASE(WriteChannelObject)
{
  std::string const objRepr = "{\"id\": 1, \"channels\": \"0,7,12,15\", \n \"type\": \"channel\",\n \"group\": 0,\n \"priority\": 3,\n \"level\": 0.8,\n"
    "\"outputChannels\": \"[ M+000 ], [ M-030, M+030 ], [U+045], []\", \"diffuseness\": 0.25 }";

  boost::property_tree::ptree propTree;
  try
  {
    std::stringstream strstr (objRepr);
    read_json( strstr, propTree);
  }
  catch (std::exception const & ex)
  {
    throw std::invalid_argument(std::string("Error while parsing a json object message: ") + ex.what());
  }

  ChannelObject co( 23 ); // Fake id
  ChannelObjectParser coParser;
  BOOST_CHECK_NO_THROW( coParser.parse( propTree, co ) );

  boost::property_tree::ptree writeTree;
  coParser.write(co, writeTree);

  std::stringstream outStr;
  boost::property_tree::write_json( outStr, writeTree, true /*pretty*/ );

  std::cout << "Wrote JSON representation: \"" << outStr.str() << "\"" << std::endl;
}


} // namespace test
} // namespace objectmodel
} // namespce visr
