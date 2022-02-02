//
//  main.cpp
//
//  Created by Dylan Menzies on 10/11/2014.
//  Copyright (c) 2014 ISVR, University of Southampton. All rights reserved.
//

#include <libpanning/VBAP.h>
#include <libpanning/AllRAD.h>
#include <boost/filesystem.hpp>
#include <libefl/cartesian_spherical_conversion.hpp>
#include <libefl/degree_radian_conversion.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <array>

// Text format will be removed soon.

BOOST_AUTO_TEST_CASE( LoudspeakerArrayLoad)
{
  using namespace visr;
  using namespace visr::panning;
  std::size_t numfiles = 4;
  std::vector<boost::filesystem::path> configFiles(numfiles);
  
//  LoudspeakerArray regArray;
  
  //std::size_t numberOfSources = 8;

  // std::vector<XYZ> sourcePos( numberOfSources );

//  FILE* file;
    
  boost::filesystem::path const configDir( CMAKE_SOURCE_DIR "/config" );
  boost::filesystem::path const sourceDir( CMAKE_CURRENT_SOURCE_DIR );

  configFiles[0] = (configDir / boost::filesystem::path( "isvr/audiolab_stereo_1sub_with_rerouting.xml" ));
  configFiles[1] = (configDir / boost::filesystem::path( "isvr/audiolab_39speakers_1subwoofer.xml" ));
  configFiles[2] = (configDir / boost::filesystem::path( "bbc/bs2051-9+10+3.xml" ));
  configFiles[3] = (configDir / boost::filesystem::path( "generic/bs2051-9+10+3.xml" ));

  for(size_t i =0; i<numfiles; i++)
  {
    LoudspeakerArray array;
    //  BOOST_CHECK_MESSAGE( exists(configFiles[i]), "Loudspeaker configuration text file dows not exist." );
    BOOST_ASSERT( exists(configFiles[i] ));
    // file = fopen( configFiles[i].string().c_str(), "r" );
    // BOOST_CHECK( file != 0 );
    BOOST_CHECK_NO_THROW( array.loadXmlFile( configFiles[i].string() ));

    std::cout<<"FILE: "<<configFiles[i].string()<<" parsed correctly"<<std::endl;
  
//  
//    std::size_t const numCols = array.getNumRegularSpeakers();
//    std::cout << "CHNLS:\t";
//    std::copy( array.getLoudspeakerChannels(), array.getLoudspeakerChannels() + numCols, std::ostream_iterator<Afloat>( std::cout, "\t\t" ) );
//    std::cout << std::endl;
//    std::cout << "IDXES:\t";
//    for(size_t j =0; j<numCols; j++){
//      
//      std::cout << a<<"\t\t";
//    }
//    std::cout << std::endl;
  }
}


#if 1
BOOST_AUTO_TEST_CASE( LoadArrayConfigXmlString ){
  using namespace visr;
  using namespace visr::panning;
  LoudspeakerArray array;

  //boost::filesystem::path const configDir( CMAKE_SOURCE_DIR "/config" );
  //boost::filesystem::path const sourceDir( CMAKE_CURRENT_SOURCE_DIR );

  //// Alternatively, load the config file in XML format.
  //boost::filesystem::path configFileXml = configDir / boost::filesystem::path( "isvr/audiolab_5.1_1subwoofer.xml" );
  //BOOST_ASSERT( exists( configFileXml ) );
  //BOOST_CHECK_NO_THROW( array.loadXmlFile( configFileXml.string() ) );
  //
  std::string xml = "<?xml version=\"1.0\" encoding=\"utf - 8\"?>"
  "<panningConfiguration dimension = \"2\" infinite = \"false\">"
  "<outputEqConfiguration numberOfBiquads = \"1\" type = \"iir\">"
  "<filterSpec name = \"lowpass\">"
  "<biquad a1 = \"-1.9688283\" a2 = \"0.96907117\" b0 = \"6.0729856e-05\" b1 = \"0.00012145971\" b2 = \"6.0729856e-05\"/>"
  "</filterSpec>"
  "<filterSpec name = \"highpass\">"
  "<biquad a1 = \"-1.9688283\" a2 = \"0.96907117\" b0 = \"-0.98447486\" b1 = \"1.9689497\" b2 = \"-0.98447486\"/>"
  "</filterSpec>"
  "</outputEqConfiguration>"
  "<loudspeaker channel = \"27\" delay = \"0\" eq = \"highpass\" gainDB = \"0\" id = \"1\">"
  "<cart x = \"2.17\" y = \"1.36\" z = \"0\"/>"
  "</loudspeaker>"
  "<loudspeaker channel = \"16\" delay = \"0.0011502\" eq = \"highpass\" gainDB = \"-2.1488\" id = \"2\">"
  "<cart x = \"2.08\" y = \"0.03\" z = \"0\"/>"
  "</loudspeaker>"
  "<loudspeaker channel = \"17\" delay = \"9.8764e-05\" eq = \"highpass\" gainDB = \"-0.35712\" id = \"3\">"
  "<cart x = \"2.15\" y = \"-1.22\" z = \"0.01\"/>"
  "</loudspeaker>"
  "<loudspeaker channel = \"20\" delay = \"0.0015247\" eq = \"highpass\" gainDB = \"-1.8169\" id = \"4\">"
  "<cart x = \"-1.01\" y = \"-1.69\" z = \"0\"/>"
  "</loudspeaker>"
  "<loudspeaker channel = \"24\" delay = \"0.0016965\" eq = \"highpass\" gainDB = \"-2.0675\" id = \"5\">"
  "<cart x = \"-0.98\" y = \"1.74\" z = \"0\"/>"
  "</loudspeaker>"
  "<subwoofer assignedLoudspeakers = \"1, 2, 3, 4, 5\" channel = \"41\" delay = \"0\" eq = \"lowpass\" gainDB = \"0\" weights = \"1.000000, 1.000000, 1.000000, 1.000000, 1.000000\"/>"
  "<triplet l1 = \"4\" l2 = \"5\"/>"
  "<triplet l1 = \"3\" l2 = \"4\"/>"
  "<triplet l1 = \"2\" l2 = \"3\"/>"
  "<triplet l1 = \"1\" l2 = \"2\"/>"
  "<triplet l1 = \"5\" l2 = \"1\"/>"
  "</panningConfiguration>";
  //std::cout<<xml<<std::endl;
  array.loadXmlString( xml );
  VBAP vbap( array, 0.0f, 0.0f, 0.0f );

  std::vector<SampleType> vbapGains;
  std::fill( vbapGains.begin(), vbapGains.end(), 0.0f );
  vbapGains.resize( array.getNumRegularSpeakers() );
  vbap.calculateGains( 2.08f, 1.0f, -5.0f, vbapGains.data(), false /*planeWave*/ );
  //vbap.calculateGains( 1.0f, 1.0f, 0.0f, vbapGains.data() , false /*planeWave*/ );
  std::size_t const numCols = array.getNumRegularSpeakers();

  std::cout << "VBAP gain for source : ";
  std::copy( vbapGains.data(), vbapGains.data() + numCols, std::ostream_iterator<Afloat>( std::cout, ", " ) );
  std::cout << std::endl;
}
#endif

BOOST_AUTO_TEST_CASE( LoadArrayConfigXmlFile )
{
  using namespace visr;
  using namespace visr::panning;
  LoudspeakerArray array;

  boost::filesystem::path const configDir( CMAKE_SOURCE_DIR "/config" );
  boost::filesystem::path const sourceDir( CMAKE_CURRENT_SOURCE_DIR );

  boost::filesystem::path configFileXml = configDir / boost::filesystem::path( "bbc/bs2051-4+5+0.xml" );

  //boost::filesystem::path configFileXml = configDir / boost::filesystem::path( "isvr/audiolab_22speakers_1subwoofer.xml" );
  BOOST_ASSERT( exists( configFileXml ) );
  BOOST_CHECK_NO_THROW( array.loadXmlFile( configFileXml.string() ) );
  
  std::vector<std::size_t> chan ( array.getLoudspeakerChannels(), array.getLoudspeakerChannels()+ array.getNumRegularSpeakers());
  std::vector<LoudspeakerArray::ChannelIndex>::iterator pos = std::adjacent_find( chan.begin(), chan.end(), std::greater<std::size_t>() );
  BOOST_CHECK( pos == chan.end() );
  // Doesn't make sense at the moment.
//  for(std::size_t w=0; w< array.getNumRegularSpeakers();w++)
//  {
//    std::size_t ch = array.getSpeakerChannel( w );
//    //std::cout<<"w: "<<w<< " chn: "<< array.getSpeakerChannel( w )<<" spk: "<< array.getSpeakerIndexFromChn(ch+1)<<std::endl;
//    // BOOST_CHECK( w == array.(ch));
//  }
}
