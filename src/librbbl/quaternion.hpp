/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_RBBL_QUATERNION_HPP_INCLUDED
#define VISR_RBBL_QUATERNION_HPP_INCLUDED

#include "export_symbols.hpp"

#include <boost/math/quaternion.hpp>

namespace visr
{
namespace rbbl
{
// Forward declaration
template< typename CoordinateType >
class Position3D;

template< typename CoordinateType >
class VISR_RBBL_LIBRARY_SYMBOL Quaternion
{
public:
  using Coordinate = CoordinateType;
  using InternalType = boost::math::quaternion< Coordinate >;

  Quaternion();

  explicit Quaternion( Coordinate w,
                       Coordinate x,
                       Coordinate y,
                       Coordinate z,
                       bool normalise = false );

  explicit Quaternion( InternalType const & impl );

  explicit Quaternion( InternalType && impl );

  static Quaternion< Coordinate > fromYPR( Coordinate yaw,
                                           Coordinate roll,
                                           Coordinate pitch );

  static Quaternion< Coordinate > fromRotationVector(
      Position3D< Coordinate > const & rotVector, Coordinate angle );

  Quaternion( Quaternion< Coordinate > const & rhs );

  Quaternion( Quaternion && rhs );

  Quaternion & operator=( Quaternion const & rhs );

  Quaternion & operator=( Quaternion && rhs );

  ~Quaternion();

  Coordinate w() const { return mQuat.R_component_1(); }

  Coordinate x() const { return mQuat.R_component_2(); }

  Coordinate y() const { return mQuat.R_component_3(); };

  Coordinate z() const { return mQuat.R_component_4(); };

  void setW( Coordinate w );

  void setX( Coordinate x );

  void setY( Coordinate y );

  void setZ( Coordinate z );

  Coordinate yaw() const;

  Coordinate pitch() const;

  Coordinate roll() const;

  Position3D< Coordinate > rotationVector() const;

  Coordinate rotationAngle() const;

  void set( Coordinate w, Coordinate x, Coordinate y, Coordinate z );

  void setYPR( Coordinate yaw, Coordinate pitch, Coordinate roll );

  void setRotationVector( Position3D< Coordinate > const & rotVec,
                          Coordinate angle );

  /**
   * Conjugate the quaternion.
   */
  void conjugate();

  /**
   * Normalise to a unit quaternion.
   * @param adjustSign Whether to negate the quaternion to make the w component
   * nonnegative.
   * @param silentDivideByZero If set to false throw an exception if the norm of
   * the quaternion is very close to zero. This results in inaccurate values or
   * NaNs.
   */
  void normalise( bool adjustSign = false, bool silentDivideByZero = false );

  /**
   * Return the Euclidean (L2) norm of the vector.
   */
  Coordinate norm() const;

  Coordinate normSquare() const;

  Quaternion & operator+=( Quaternion const & rhs );

  Quaternion & operator-=( Quaternion const & rhs );

  Quaternion & operator*=( Coordinate scale );

  Quaternion & operator*=( Quaternion const & rhs );

  void rotate( Quaternion< CoordinateType > const & rot );

  /**
   * Return the internal implementation type.
   */
  InternalType const & impl() const { return mQuat; }

private:
  InternalType mQuat;
};

/**
 * Free functions
 */
//@{
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > conjugate(
    Quaternion< CoordinateType > const & quat );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > operator+(
    Quaternion< CoordinateType > const & lhs,
    Quaternion< CoordinateType > const & rhs );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > operator-(
    Quaternion< CoordinateType > const & lhs,
    Quaternion< CoordinateType > const & rhs );

/**
 * Quaternion multiplication, non-commutative
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > operator*(
    Quaternion< CoordinateType > const & lhs,
    Quaternion< CoordinateType > const & rhs );

/**
 * Unary minus operator, returns the opposite quaternion.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > operator-(
    Quaternion< CoordinateType > const & val );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > operator*(
    CoordinateType scale, Quaternion< CoordinateType > const & val );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > operator/(
    Quaternion< CoordinateType > const & val, CoordinateType scale );

/**
 * Return the angular distance between the rotations represented by the two
 * quaternions.
 * @throw std::runtime_error If one of the angles is zero (or very close).
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL CoordinateType
angle( Quaternion< CoordinateType > const & lhs,
       Quaternion< CoordinateType > const & rhs );

/**
 * Return the angular distance between two rotations represented by quaternions.
 * This is faster than the generalised angle() function but yields wrong results
 * if the arguments are not normalised (i.e., no unit quaternions).
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL CoordinateType
angleNormalised( Quaternion< CoordinateType > const & lhs,
                 Quaternion< CoordinateType > const & rhs );

/**
 * Create a unit quaternion by scaling an existing one.
 * @param val the quaternion to scale.
 * @param adjustSign If set to true, the resulting quaternion will negated if
 * the w component is negative.
 * @param silentDivideByZero If set to false (the default), this throws an
 * exception if the norm is very close to zero. If set to true, no exception is
 * thrown, but it may yoeld increased errors or NaNs.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > normalise(
    Quaternion< CoordinateType > const & val,
    bool adjustSign = false,
    bool silentDivideByZero = false );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL CoordinateType
dot( Quaternion< CoordinateType > const & op1,
     Quaternion< CoordinateType > const & op2 );

/**
 * Perform a spherical interpolation between two quaternions.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Quaternion< CoordinateType > interpolateSpherical(
    Quaternion< CoordinateType > const & pos0,
    Quaternion< CoordinateType > const & pos1,
    CoordinateType tInterp );
//@}

} // namespace rbbl
} // namespace visr

#endif // VISR_RBBL_QUATERNION_HPP_INCLUDED
