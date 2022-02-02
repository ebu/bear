/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "listener_position.hpp"

#include <libefl/degree_radian_conversion.hpp>

#include <librbbl/quaternion.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <algorithm>
#include <ciso646>
#include <cmath>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace visr
{
namespace pml
{
ListenerPosition::ListenerPosition( ParameterConfigBase const & config )
 : ListenerPosition( dynamic_cast< EmptyParameterConfig const & >( config ) )
{
}

ListenerPosition::ListenerPosition(
    EmptyParameterConfig const & /* = EmptyParameterConfig( )*/ )
 : ListenerPosition{ PositionType{}, 
     ListenerPosition::OrientationQuaternion{ 1.0f, 0.0f, 0.0f, 0.0f } }
{
}

ListenerPosition::ListenerPosition( ListenerPosition const & rhs ) = default;

ListenerPosition::ListenerPosition( Coordinate x,
                                    Coordinate y,
                                    Coordinate z,
                                    Coordinate yaw /*= 0.0f*/,
                                    Coordinate pitch /*= 0.0f*/,
                                    Coordinate roll /*= 0.0f*/ )
 : ListenerPosition{ PositionType{ x, y, z },
                     OrientationQuaternion::fromYPR( yaw, pitch, roll ) }
{
}

ListenerPosition::ListenerPosition(
    ListenerPosition::PositionType const & position,
    OrientationYPR const & orientation )
 : ListenerPosition{
   position, OrientationQuaternion::fromYPR(
                 orientation[ 0 ], orientation[ 1 ], orientation[ 2 ] )
 }
{
}

ListenerPosition::ListenerPosition( PositionType const & position,
                                    OrientationQuaternion const & orientation )
 : mPosition( position )
 , mOrientation( orientation )
 , mTimeNs{ 0 }
 , mFaceID{ 0 }
 , mNumberOfListeners{ 1 }
{
}

ListenerPosition::ListenerPosition( ListenerPosition && rhs ) = default;

ListenerPosition & ListenerPosition::operator=( ListenerPosition const & rhs ) =
    default;

ListenerPosition & ListenerPosition::operator=( ListenerPosition && rhs ) =
    default;

/*static*/ ListenerPosition ListenerPosition::fromRotationVector(
    ListenerPosition::PositionType const & position,
    ListenerPosition::RotationVector const & rotationVector,
    ListenerPosition::Coordinate rotationAngle )
{
  return ListenerPosition{ position, OrientationQuaternion::fromRotationVector(
                                         rotationVector, rotationAngle ) };
}

/*static*/ ListenerPosition ListenerPosition::fromJson(
    boost::property_tree::ptree const & config )
{
  ListenerPosition res;
  res.parseJson( config );
  return res;
}

/*static*/ ListenerPosition ListenerPosition::fromJson( std::istream & stream )
{
  ListenerPosition res;
  res.parseJson( stream );
  return res;
}

/*static*/ ListenerPosition ListenerPosition::fromJson(
    std::string const & str )
{
  ListenerPosition res;
  res.parseJson( str );
  return res;
}

ListenerPosition::~ListenerPosition() = default;

void ListenerPosition::set( Coordinate newX,
                            Coordinate newY,
                            Coordinate newZ /*= 0.0f*/ )
{
  mPosition.set( newX, newY, newZ );
}

void ListenerPosition::setPosition( PositionType const & position )
{
  mPosition = position;
}

void ListenerPosition::setPosition( Coordinate newX, Coordinate newY,
   Coordinate newZ )
{
  mPosition.set( newX, newY, newZ );
}

void ListenerPosition::setX( Coordinate newX ) { mPosition.setX( newX ); }

void ListenerPosition::setY( Coordinate newY ) { mPosition.setY( newY ); }

void ListenerPosition::setZ( Coordinate newZ ) { mPosition.setZ( newZ ); }

ListenerPosition::Coordinate ListenerPosition::yaw() const
{
  return mOrientation.yaw();
}

ListenerPosition::Coordinate ListenerPosition::pitch() const
{
  return mOrientation.pitch();
}

ListenerPosition::Coordinate ListenerPosition::roll() const
{
  return mOrientation.roll();
}

ListenerPosition::OrientationYPR ListenerPosition::orientationYPR() const
{
  return ListenerPosition::OrientationYPR{ yaw(), pitch(), roll() };
}

ListenerPosition::OrientationQuaternion const &
ListenerPosition::orientationQuaternion() const
{
  return mOrientation;
}

ListenerPosition::RotationVector ListenerPosition::orientationRotationVector()
    const
{
  return mOrientation.rotationVector();
}

ListenerPosition::Coordinate ListenerPosition::orientationRotationAngle() const
{
  return mOrientation.rotationAngle();
}

void ListenerPosition::setOrientationYPR( ListenerPosition::Coordinate yaw,
                                          ListenerPosition::Coordinate pitch,
                                          ListenerPosition::Coordinate roll )
{
  mOrientation.setYPR( yaw, pitch, roll );
}

void ListenerPosition::setOrientationYPR( OrientationYPR const & orientation )
{
  setOrientationYPR( orientation[ 0 ], orientation[ 1 ], orientation[ 2 ] );
}

void ListenerPosition::setOrientationQuaternion(
    OrientationQuaternion const & orientation )
{
  mOrientation = orientation;
}

void ListenerPosition::setOrientationRotationVector(
    ListenerPosition::RotationVector const & rotationVector,
    ListenerPosition::Coordinate angle )
{
  mOrientation.setRotationVector( rotationVector, angle );
}

void ListenerPosition::translate(
    ListenerPosition::PositionType const & translationVector )
{
  mPosition += translationVector;
}

void ListenerPosition::rotate(
    ListenerPosition::OrientationQuaternion const & rotation )
{
  rotatePosition( rotation );
  rotateOrientation( rotation );
}

void ListenerPosition::rotatePosition(
    ListenerPosition::OrientationQuaternion const & rotation )
{
  mPosition.rotate( rotation );
}

void ListenerPosition::rotateOrientation(
    ListenerPosition::OrientationQuaternion const & rotation )
{
  mOrientation = rotation * mOrientation;
}

void ListenerPosition::transform(
    ListenerPosition::OrientationQuaternion const & rotation,
    ListenerPosition::PositionType translation )
{
  rotate( rotation );
  translate( translation );
}

void ListenerPosition::setTimeNs( TimeType timeNs ) { mTimeNs = timeNs; }

void ListenerPosition::setFaceID( IdType faceID ) { mFaceID = faceID; }

std::size_t ListenerPosition::numberOfListeners() const
{
  return mNumberOfListeners;
}

void ListenerPosition::setNumberOfListeners( std::size_t numListeners )
{
  mNumberOfListeners = numListeners;
}


void ListenerPosition::parseJson( std::istream & stream )
{
  namespace pt = boost::property_tree;
  pt::ptree tree;
  read_json( stream, tree );
  parseJson( tree );
}

void ListenerPosition::parseJson( boost::property_tree::ptree const & tree )
{
  namespace pt = boost::property_tree;
  Coordinate const x = tree.get< Coordinate >( "x", 0.0f );
  Coordinate const y = tree.get< Coordinate >( "y", 0.0f );
  Coordinate const z = tree.get< Coordinate >( "z", 0.0f );
  if( tree.count( "orientation" ) > 0 )
  {
    pt::ptree const & orTree = tree.get_child( "orientation" );
    bool const hasQuat{ orTree.count( "quaternion" ) > 0 };
    bool const hasYpr{ ( orTree.count( "yaw" ) > 0 ) or
                       ( orTree.count( "pitch" ) > 0 ) or
                       ( orTree.count( "roll" ) > 0 ) };
    bool const hasRotationVec{ orTree.count( "rotationVector" ) > 0 };
    if( ( hasQuat ? 1 : 0 ) + ( hasYpr ? 1 : 0 ) + ( hasRotationVec ? 1 : 0 ) >
        1 )
    {
      throw std::invalid_argument(
          "ListenerPosition::parseJson():"
          " Orientation must be provided either as yaw,pitch,roll, a rotation "
          "vector/angle or as a quaternion" );
    }
    if( hasQuat )
    {
      pt::ptree const & quatTree = orTree.get_child( "quaternion" );
      Coordinate const qw = quatTree.get< Coordinate >( "w" );
      Coordinate const qx = quatTree.get< Coordinate >( "x" );
      Coordinate const qy = quatTree.get< Coordinate >( "y" );
      Coordinate const qz = quatTree.get< Coordinate >( "z" );
      setOrientationQuaternion( OrientationQuaternion( qw, qx, qy, qz ) );
    }
    else if( hasYpr )
    {
      Coordinate const yaw =
          efl::degree2radian( orTree.get< Coordinate >( "yaw", 0.0f ) );
      Coordinate const pitch =
          efl::degree2radian( orTree.get< Coordinate >( "pitch", 0.0f ) );
      Coordinate const roll =
          efl::degree2radian( orTree.get< Coordinate >( "roll", 0.0f ) );
      setOrientationYPR( yaw, pitch, roll );
    }
    else if( hasRotationVec )
    {
      pt::ptree const & vecTree = orTree.get_child( "rotationVector" );
      Coordinate const rx = vecTree.get< Coordinate >( "x" );
      Coordinate const ry = vecTree.get< Coordinate >( "y" );
      Coordinate const rz = vecTree.get< Coordinate >( "z" );
      Coordinate const angle = vecTree.get< Coordinate >( "angle" );
      RotationVector const vec{ rx, ry, rz };
      setOrientationRotationVector( vec, efl::degree2radian( angle ) );
    }
    else
    {
      setOrientationYPR( 0.0f, 0.0f, 0.0f );
    }
  }
  else
  {
    // set default rotation
    setOrientationQuaternion( OrientationQuaternion() );
  }
  set( x, y, z );
  setTimeNs( tree.get< pml::ListenerPosition::TimeType >( "timeStamp", 0 ) );
  setFaceID( tree.get< pml::ListenerPosition::IdType >( "faceId", 0 ) );
  setNumberOfListeners( tree.get< std::size_t >( "numListeners", 1 ) );
}

void ListenerPosition::parseJson( std::string const & str )
{
  std::stringstream stream{ str };
  parseJson( stream );
}

void ListenerPosition::writeJson( std::ostream & stream,
                                  RotationFormat rotationFormat,
                                  bool prettyPrint /*=false*/ ) const
{
  namespace pt = boost::property_tree;
  pt::ptree tree;
  writeJson( tree, rotationFormat );
  pt::write_json( stream, tree, prettyPrint );
}

void ListenerPosition::writeJson( boost::property_tree::ptree & tree,
                                  RotationFormat rotationFormat ) const
{
  namespace pt = boost::property_tree;
  tree.put< Coordinate >( "x", x() );
  tree.put< Coordinate >( "y", y() );
  tree.put< Coordinate >( "z", z() );
  pt::ptree orientationTree;
  switch( rotationFormat )
  {
    case RotationFormat::YPR:
    {
      OrientationYPR const ypr{ orientationYPR() };
      orientationTree.put< Coordinate >( "yaw",
                                         efl::radian2degree( ypr[ 0 ] ) );
      orientationTree.put< Coordinate >( "pitch",
                                         efl::radian2degree( ypr[ 1 ] ) );
      orientationTree.put< Coordinate >( "roll",
                                         efl::radian2degree( ypr[ 2 ] ) );
      break;
    }
    case RotationFormat::RotationVector:
    {
      pt::ptree rvTree;
      RotationVector const vec{ orientationRotationVector() };
      Coordinate const angle = efl::radian2degree( orientationRotationAngle() );
      rvTree.put< Coordinate >( "angle", angle );
      rvTree.put< Coordinate >( "x", vec.x() );
      rvTree.put< Coordinate >( "y", vec.y() );
      rvTree.put< Coordinate >( "z", vec.z() );
      orientationTree.add_child( "rotationVector", rvTree );
      break;
    }
    case RotationFormat::Quaternion:
    {
      pt::ptree quatTree;
      OrientationQuaternion const quat{ orientationQuaternion() };
      quatTree.put< Coordinate >( "w", quat.w() );
      quatTree.put< Coordinate >( "x", quat.x() );
      quatTree.put< Coordinate >( "y", quat.y() );
      quatTree.put< Coordinate >( "z", quat.z() );
      orientationTree.add_child( "quaternion", quatTree );
      break;
    }
  }
  tree.add_child( "orientation", orientationTree );
  if( timeNs() != 0 )
  {
    tree.put< pml::ListenerPosition::TimeType >( "timeStamp", this->timeNs() );
  }
  if( faceID() != 0 )
  {
    tree.put< pml::ListenerPosition::IdType >( "faceId", faceID() );
  }
  tree.put< std::size_t >( "numListeners", numberOfListeners() );
}

std::string ListenerPosition::writeJson( RotationFormat rotationFormat,
                                         bool prettyPrint /*=false*/ ) const
{
  std::stringstream stream;
  writeJson( stream, rotationFormat );
  return stream.str();
}

std::ostream & operator<<( std::ostream & stream, const ListenerPosition & pos )
{
  using T = ListenerPosition::Coordinate;
  stream << "time: " << static_cast< double >( pos.timeNs() ) / 1.0e9
         << " s, face ID " << pos.faceID() << ", pos: (" << pos.x() << ", "
         << pos.y() << ", " << pos.z() << ")"
         << ", orientation ypr: (" << efl::radian2degree< T >( pos.yaw() )
         << ", " << efl::radian2degree< T >( pos.pitch() ) << ", "
         << efl::radian2degree< T >( pos.roll() ) << " )";
  return stream;
}

} // namespace
} // namespace pml
