/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_LISTENER_POSITION_HPP_INCLUDED
#define VISR_PML_LISTENER_POSITION_HPP_INCLUDED

#include "empty_parameter_config.hpp"
#include "export_symbols.hpp"

#include <libvisr/detail/compile_time_hash_fnv1.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/typed_parameter_base.hpp>

#include <librbbl/position_3d.hpp>
#include <librbbl/quaternion.hpp>

#include <boost/property_tree/ptree_fwd.hpp>

#include <array>
#include <cstdint>
#include <iosfwd>
#include <iosfwd>
#include <string>

namespace visr
{
namespace pml
{
/**
 * Define a unique name for the parameter type.
 */
static constexpr const char* sListenerPositionParameterName =
    "ListenerPosition";

/**
 * Parameter for a listener position, i.e. position and head orientation in 3D
 * space.
 */
class VISR_PML_LIBRARY_SYMBOL ListenerPosition
 : public TypedParameterBase< ListenerPosition, EmptyParameterConfig,
                              detail::compileTimeHashFNV1(
                                  sListenerPositionParameterName ) >
{
public:
  /**
   * Timestamp data type
   * @note Timestamping functionality is specific to a single application and
   * might be removed.
   */
  using TimeType = std::uint64_t;

  /**
   * Data type for disambiguating between listeners.
   * @note This is used in a specific application and therefore subject to
   * removal.
   */
  using IdType = unsigned int;

  /**
   * Data type for all coordinate values (positions and orientation angles)
   */
  using Coordinate = float;

  /**
   * Data type representing the position of the listener.
   * An array containing x, y, z as Caertesian coordinates [in metre]
   */
  using PositionType = rbbl::Position3D<Coordinate>;

  using OrientationQuaternion = rbbl::Quaternion< Coordinate >;

  /**
   * Data type representing the orientation of the listener as Euler angles.
   * A size-3 array containing yaw, pitch, and roll angles (in radian)
   */
  using OrientationYPR = std::array< Coordinate, 3 >;

  /**
   * Data type for roation vectors. Used for representing the orientation
   * as a rotation (with a given angle) around an arbitrary unit vector.
   */
  using RotationVector = PositionType;

  /**
   * Construction from a parameter configuration parameter.
   * @param config Configuration parameter, the dynamic type must be
   * pml::EmptyParameterConfig() Also acts as default constructor.
   * @throw std::bad_cast if the dynamic type of \p config differs from
   * pml::EmptyParameterConfig()
   */
  explicit ListenerPosition( ParameterConfigBase const& config );

  /**
   * Construction from a parameter configuration parameter.
   * @param config Configuration parameter of type be
   * pml::EmptyParameterConfig(), essentailly an empty placeholder.
   */
  explicit ListenerPosition(
      EmptyParameterConfig const& config = EmptyParameterConfig() );

  /**
   * Copy constructor.
   * @param rhs The object to be copied.
   */
  explicit ListenerPosition( ListenerPosition const& rhs );

  /**
   * Move constructor.
   */
  ListenerPosition( ListenerPosition&& rhs );

  /**
   * Constructor taking scalar arguments
   * @param x X coordiante [m]
   * @param y Y coordiante [m]
   * @param z Z coordiante [m]
   * @param yaw Yaw angle of the parameter orientation, default 0.0 [radian]
   * @param pitch Pitch angle of the parameter orientation, default 0.0 [radian]
   * @param roll Roll angle of the parameter orientation, default 0.0 [radian]
   */
  ListenerPosition( Coordinate x, Coordinate y, Coordinate z,
                    Coordinate yaw = 0.0f, Coordinate pitch = 0.0f,
                    Coordinate roll = 0.0f );

  /**
   * Constructor taking coordinate vectors for position and orientation.
   * @param position Cartesian position vector {x y z}, unit [m]
   * @param orientation Orientation vector [yaw pitch roll], unit [radian].
   * Default [0 0 0]
   */
  ListenerPosition( PositionType const& position,
                    OrientationYPR const& orientation );

  ListenerPosition( PositionType const& position,
                    OrientationQuaternion const& orientation );

  /**
   * Create a ListenerPosition object from a position and a rotation around an
   * arbitrary rotation axis.
   * This function implements the 'named constructor' idiom.
   * @param position The listner position in Cartesian coordinates [in m]
   * @param rotationVector 3D vector (Cartesian coordinates) representing the
   * orientation in rotation axis /  rotation angle form.
   * @param rotationAngle rotationAngle Rotation angle around the \p
   * rotationVector to represent the listener orientation [in radian].
   */
  static ListenerPosition fromRotationVector(
      PositionType const& position, RotationVector const& rotationVector,
      Coordinate rotationAngle );

  static ListenerPosition fromJson( std::istream& stream );

  static ListenerPosition fromJson( std::string const& str );

  static ListenerPosition fromJson( boost::property_tree::ptree const& config );

  /**
   * Destructor (virtual)
   */
  virtual ~ListenerPosition() override;

  /**
   * Copy assignment operator.
   * @param rhs The object to be copied.
   */
  ListenerPosition& operator=( ListenerPosition const& rhs );

  /**
   * Move assignment operator.
   * @param rhs The object to be moved.
   */
  ListenerPosition& operator=( ListenerPosition&& rhs );

  /**
   * Return the x coordinate [m]
   */
  Coordinate x() const { return mPosition.x(); }

  /**
   * Return the y coordinate [m]
   */
  Coordinate y() const { return mPosition.y(); }

  /**
   * Return the z coordinate [m]
   */
  Coordinate z() const { return mPosition.z(); }

  /**
   * Return the position as a 3D Cartesian vector unit: [m]
   */
  PositionType const& position() const { return mPosition; }

  /**
   * Set the position using scalar values.
   * @param newX New x coordinate [m]
   * @param newY New y coordinate [m]
   * @param newZ New z coordinate [m]
   * @deprecated Use setPosition() instead.
   */
  void set( Coordinate newX, Coordinate newY, Coordinate newZ = 0.0f );

  /**
   * Set the position using a Cartesian 3-element vector, unit: [m]
   * @param position New position.
   */
  void setPosition( PositionType const& position );

  /**
   * Set the position using scalar values.
   * @param newX New x coordinate [m]
   * @param newY New y coordinate [m]
   * @param newZ New z coordinate [m]
   */
  void setPosition( Coordinate newX, Coordinate newY, Coordinate newZ = 0.0f );


  /**
   * Set the x position.
   * @param newX New x coordinate [m]
   */
  void setX( Coordinate newX );

  /**
   * Set the y position.
   * @param newY New y coordinate [m]
   */
  void setY( Coordinate newY );

  /**
   * Set the z position.
   * @param newZ New z coordinate [m]
   */
  void setZ( Coordinate newZ );

  /**
   * Return the yaw angle of the listener's orientation [radian]
   */
  Coordinate yaw() const;

  /**
   * Return the pitch angle of the listener's orientation [radian]
   */
  Coordinate pitch() const;

  /**
   * Return the roll angle of the listener's orientation [radian]
   */
  Coordinate roll() const;

  /**
   * Return the listener's orientation as a 3-element vector.
   * Layout: [yaw pitch roll], unit: [radian]
   */
  OrientationYPR orientationYPR() const;

  OrientationQuaternion const& orientationQuaternion() const;

  /**
   * Return the rotation vector representing the listener orientation in
   * rotation vector / angle form.
   * @return Unit vector in Cartesian coordinates.
   */
  RotationVector orientationRotationVector() const;

  /**
   * Return the angle part of the listener orientation in rotation vector/angle
   * form.
   * @return Rotation angle [radian].
   */
  Coordinate orientationRotationAngle() const;

  /**
   * Set the listener's orientation using scalar values.
   * @param yaw New yaw angle [radian]
   * @param pitch New pitch angle [radian]
   * @param roll New roll angle [radian]
   */
  void setOrientationYPR( Coordinate yaw, Coordinate pitch, Coordinate roll );

  /**
   * Set the listener's orientation as a 3-element vector.
   * @param orientation New orientation vector, layout: [yaw pitch roll], unit:
   * [radian]
   */
  void setOrientationYPR( OrientationYPR const& orientation );

  /**
   * Set the object's orientation.
   * @param orientation The new orientation, specified as a unit quaternion.
   */
  void setOrientationQuaternion( OrientationQuaternion const& orientation );

  /**
   * Set the listener orientation in rotation axis / rotation angle format.
   * @param rotationVector The rotation axis in Cartesian coordinates [units
   * irelevant].
   * @param angle Rotation angle [radian].
   */
  void setOrientationRotationVector( RotationVector const& rotationVector,
                                     Coordinate angle );

  /**
   * Shift the position part.
   * @param translationVector The position shift, vector in Cartesian coordinate
   * [m].
   */
  void translate( PositionType const& translationVector );

  /**
   * Rotate the object's postion and orientation.
   * @param rotation The desired rotation, specified as a unit quaternion.
   */
  void rotate( OrientationQuaternion const& rotation );

  /**
   * Rotate the listener position around the origin.
   * This leaves the orientation component unchanged.
   * @param rotation The desired rotation, specified as a unit quaternion.
   */
  void rotatePosition( OrientationQuaternion const& rotation );

  /**
   * Rotate the listener orientation.
   * This leaves the position component unchanged.
   * @param rotation The desired rotation, specified as a unit quaternion.
   */
  void rotateOrientation( OrientationQuaternion const& rotation );

  /**
   * General transformation consisting of a rotation around the origin of the
   * coordinate system followed by a translation of the position.
   * @param rotation The desired rotation, specified as a unit quaternion.
   * @param translation The position shift, vector in Cartesian coordinate [m].
   */
  void transform( OrientationQuaternion const& rotation,
                  PositionType translation );

  /**
   * Return the time stamp of the position parameter.
   * @note Consider removal, because this is used only in a specific
   * application.
   */
  TimeType timeNs() const { return mTimeNs; }

  /**
   * Set the time stamp of the position parameter.
   * @param timeNs new time stamp (in nanoseconds)
   * @note Consider removal, because this is used only in a specific
   * application.
   */
  void setTimeNs( TimeType timeNs );

  /**
   * Return the listener id (face id) associated with this listener position.
   * @note Consider removal, because this is used only in a specific
   * application.
   */
  IdType faceID() const { return mFaceID; }

  /**
   * Set the listener id (face id) associated with this listener position.
   * @param faceID New listener ID
   * @note Consider removal, because this is used only in a specific
   * application.
   */
  void setFaceID( IdType faceID );

  /**
   * Return the number of listeners currently detected.
   * Although a ListenerPosition object always contains position data for a
   * single listener, this attribute can be used to signal whether there are
   * multiple listeners in the range of a capturing device, or whether the
   * tracking data might be outdated (value of zero).
   * Valid values are 1 (one listener is tracked and its coordinates are
   * sent in the position and orientaton members), 0 (no listener is currently
   * detected, and the position and orientation members represent either default
   * values or the last detected state), or more than one (the coordinates of
   * the active, tracked user are contained in the position and orientation
   * members, but additional users are present).
   */
  std::size_t numberOfListeners() const;

  /**
   * Set the number of listners currently sensed.
   * This does not change the amount of tracking data, which is always
   * transmitted for one user, but to provide additional metadata about the
   * listening environment.
   * This value is initialised to 1 (for backward compatibility with
   * applications that do not consider this additional listener metadata.)
   */
  void setNumberOfListeners( std::size_t numListeners );

  void parseJson( std::istream& stream );

  void parseJson( std::string const& str );

  void parseJson( boost::property_tree::ptree const& config );

  enum class RotationFormat
  {
    YPR,
    RotationVector,
    Quaternion
  };

  void writeJson( std::ostream& stream,
                  RotationFormat rotationFormat = RotationFormat::Quaternion,
                  bool prettyPrint = false ) const;

  std::string writeJson(
      RotationFormat rotationFormat = RotationFormat::Quaternion,
      bool prettyPrint = false ) const;

  void writeJson(
      boost::property_tree::ptree& tree,
      RotationFormat rotationFormat = RotationFormat::Quaternion ) const;

private:
  /**
   * Cartesian listener coordinates as a 3D vector [xyz], in [m]
   */
  PositionType mPosition;

  /**
   * Listener orientation, stored as a quaternion.
   */
  OrientationQuaternion mOrientation;

  /**
   * Time stamp, in [ns]
   * @note Subject to removal, see timeNs()
   */
  TimeType mTimeNs;

  /**
   * Listener id
   * @note Subject to removal, see faceID()
   */
  IdType mFaceID;

  std::size_t mNumberOfListeners;
};

VISR_PML_LIBRARY_SYMBOL std::ostream& operator<<( std::ostream& stream,
                                                  const ListenerPosition& pos );

} // namespace pml
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::pml::ListenerPosition,
                       visr::pml::ListenerPosition::staticType(),
                       visr::pml::EmptyParameterConfig )

#endif // VISR_PML_LISTENER_POSITION_HPP_INCLUDED
