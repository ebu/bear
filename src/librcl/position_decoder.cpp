/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "position_decoder.hpp"

#include <libpml/empty_parameter_config.hpp>
#include <libpml/listener_position.hpp>
#include <libpml/string_parameter.hpp>

#include <librbbl/quaternion.hpp>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

namespace visr
{
namespace rcl
{

PositionDecoder::PositionDecoder( SignalFlowContext const & context,
  char const * name,
  CompositeComponent * parent,
  pml::ListenerPosition::PositionType const & positionOffset
   /*= pml:: ListenerPosition::PositionType()*/,
  pml::ListenerPosition::OrientationQuaternion const & orientationRotation
   /* = pml:: ListenerPosition::OrientationQuaternion()*/ )
 : AtomicComponent( context, name, parent )
 , mDatagramInput( "messageInput", *this, pml::EmptyParameterConfig() )
 , mPositionOutput( "positionOutput", *this, pml::EmptyParameterConfig() )
 , cOffsetPosition( positionOffset )
 , cOrientationRotation( orientationRotation )
{

}

PositionDecoder::PositionDecoder( SignalFlowContext const & context,
  char const * name,
  CompositeComponent * parent,
  pml:: ListenerPosition::PositionType const & positionOffset,
  pml:: ListenerPosition::OrientationYPR const & orientationRotation )
 : PositionDecoder( context, name, parent, positionOffset, 
    pml::ListenerPosition::OrientationQuaternion::fromYPR( orientationRotation[0],
     orientationRotation[1], orientationRotation[2] ) )
{
}

PositionDecoder::~PositionDecoder() = default;

void PositionDecoder::process()
{
  pml::ListenerPosition newPos;
  pml::ListenerPosition foundPos;
  pml::ListenerPosition::IdType smallestFaceId = std::numeric_limits<unsigned int>::max();
  pml::ListenerPosition::TimeType latestTimeStamp = 0;
  while( !mDatagramInput.empty() )
  {
    char const * nextMsg = mDatagramInput.front().str();
    std::stringstream msgStream( nextMsg );
    try
    {
      newPos.parseJson( msgStream );
      if( newPos.faceID() <= smallestFaceId )
      {
        // for a given face ID, update the position only if the timestamp is not older than the previously received timestamp.
        if( newPos.timeNs() >= latestTimeStamp )
        {
          foundPos = newPos;
          smallestFaceId = foundPos.faceID();
          latestTimeStamp = foundPos.timeNs();
        }
      }
    }
    catch( std::exception const & ex )
    {
      // Don't abort the program when receiving a corrupted message.
      std::cerr << "PositionDecoder: Error while decoding a position message: " << ex.what() << std::endl;
    }
    mDatagramInput.pop();
  }
  // Did we actually receive a valid message?
  if( smallestFaceId != std::numeric_limits<unsigned int>::max() )
  {
    mPositionOutput.data() = translatePosition( foundPos );
    mPositionOutput.swapBuffers();
  }
}

pml::ListenerPosition PositionDecoder::translatePosition( const pml::ListenerPosition &pos )
{
  pml::ListenerPosition res( pos );
  res.transform( cOrientationRotation, cOffsetPosition );
  return res;
}

} // namespace rcl
} // namespace visr
