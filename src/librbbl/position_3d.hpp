/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_POSITION_3D_HPP_INCLUDED
#define VISR_POSITION_3D_HPP_INCLUDED

#include <boost/property_tree/ptree_fwd.hpp>

#include "export_symbols.hpp"

#include <array>

namespace visr
{
namespace rbbl
{
// Forward declaration
template< typename Coordinate >
class Quaternion;

/**
 * Template class for representing 3D positions.
 * @tparam CoordinateType The scalar type representing the coordinates.
 * Explicit instanatiations are provided for float and double.
 */
template< typename CoordinateType >
class VISR_RBBL_LIBRARY_SYMBOL Position3D
{
public:
  using Coordinate = CoordinateType;

  /**
   * Default constructor, creates a position with Cartesian coordinates [0,0,0].
   */
  Position3D();

  /**
   * Constructor, create a Position3D object from individual coordinates.
   */
  explicit Position3D( Coordinate x, Coordinate y, Coordinate z );

  /**
   * Copy constructor.
   */
  Position3D( Position3D< Coordinate > const & rhs );

  /**
   * Move constructor.
   */
  Position3D( Position3D && rhs );

  /**
   * Assignment constructor.
   */
  Position3D & operator=( Position3D const & rhs );

  /**
   * Move assignment constructor.
   */
  Position3D & operator=( Position3D && rhs );

  /**
   * Destructor.
   */
  ~Position3D();

  /**
   * Index operator, const version.
   * Access the x,y, and z coordinates through indices 0..2.
   */
  Coordinate operator[]( std::size_t idx ) const { return mPos[ idx ]; }

  /**
   * Index operator, nonconst version.
   * Obtain modifiable references to the x,y, and z coordinates through indices 0..2.
   */
  Coordinate & operator[]( std::size_t idx ) { return mPos[ idx ]; }

  /**
   * Coordinate get methods, const version.
   */
  //@{
  Coordinate x() const { return mPos[ 0 ]; }

  Coordinate y() const { return mPos[ 1 ]; };

  Coordinate z() const { return mPos[ 2 ]; };
  //@}

  /**
   * Coordinate access functions, non-const version.
   * Return modifiable references to the coordinates
   */
  //@{
  Coordinate & x() { return mPos[ 0 ]; };

  Coordinate & y() { return mPos[ 1 ]; };

  Coordinate & z() { return mPos[ 2 ]; };
  //@}

  /**
   * Coordinate setter methods.
   */
  //@{
  /**
   * Set the position from individual coordinates.
   */
  void set( Coordinate x, Coordinate y, Coordinate z ) { mPos = { x, y, z }; }

  /**
   * Set the x coordinate.
   */
  void setX( Coordinate x ) { mPos[ 0 ] = x; }

  /**
   * Set the y coordinate.
   */
  void setY( Coordinate y ) { mPos[ 1 ] = y; }

  /**
   * Set the z coordinate.
   */
  void setZ( Coordinate z ) { mPos[ 2 ] = z; }
  //@}

  /**
   * Return the Euclidean (L2) norm of the vector.
   */
  Coordinate norm() const;

  /**
   * Return the squared L2 norm of the position vector.
   * Thos avoids the square root used in the norm() call.
   */
  Coordinate normSquare() const;

  /**
   * Turn the vector into a unit vector.
   * @throw std::runtime_error If the vector is very close to zero
   * and \p silentDivideByZero is not false.
   */
  void normalise( bool silentDivideByZero = false );

  /**
   * Shift a position vector by an offset provided as another position.
   */
  Position3D & operator+=( Position3D const & rhs );

  /**
   * Shift a position by subtracting another position vector.
   */
  Position3D & operator-=( Position3D const & rhs );

  /**
   * Scale a position by a scalar, changing its length.
   */
  Position3D & operator*=( Coordinate scale );

  /**
   * Rotate a position vector.
   * @param rot The rotation provided as a unit quaternion.
   */
  void rotate( Quaternion< CoordinateType > const & rot );

  /**
   * Translate a position offset by a given shift.
   */
  void translate( Position3D< CoordinateType > const & shift );

  /**
   * Rotate and translate a 3D position.
   */
  void transform( Quaternion< CoordinateType > const & rot,
                  Position3D< CoordinateType > const & shift );

private:
  std::array< Coordinate, 3 > mPos;
};

/**
 * Free functions
 */
//@{

/**
 * Add two positions.
 * Free function returning a new position.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > operator+(
    Position3D< CoordinateType > const & lhs,
    Position3D< CoordinateType > const & rhs );

/**
 * Subtract two positions.
 * Free function returning a new position.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > operator-(
    Position3D< CoordinateType > const & lhs,
    Position3D< CoordinateType > const & rhs );

/**
 * Unary minus operator, returns the antipodal (opposite) vector.
 * Free function, returns a new position object.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > operator-(
    Position3D< CoordinateType > const & val );

/**
 * Multiplication with a real-valued scalar to scale the position vector.
 * Free function, returns a new position.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > operator*(
    CoordinateType scale, Position3D< CoordinateType > const & val );

/**
 * Scale a vector through division by a real-valued scalar.
 * Free function, returns a new position.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > operator/(
    Position3D< CoordinateType > const & val, CoordinateType scale );

/**
 * Return the subtended angle between two position vectors.
 * @return angle in radian
 * @throw std::runtime_error If one of the angles is zero (or very close).
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL CoordinateType
angle( Position3D< CoordinateType > const & lhs,
       Position3D< CoordinateType > const & rhs );

/**
 * Return the subtended angle between two unit position vectors.
 * This is faster than the generalised angle() function but yields wrong results
 * if the arguments are not normalised (i.e., no unit vectors).
 * @return angle in radian
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL CoordinateType
angleNormalised( Position3D< CoordinateType > const & lhs,
                 Position3D< CoordinateType > const & rhs );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > normalise(
    Position3D< CoordinateType > const & val, bool silentDivideByZero = false );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL CoordinateType
dot( Position3D< CoordinateType > const & op1,
     Position3D< CoordinateType > const & op2 );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > rotate(
    Position3D< CoordinateType > const & pos,
    Quaternion< CoordinateType > const & rot );

template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > transform(
    Position3D< CoordinateType > const & pos,
    Quaternion< CoordinateType > const & rot,
    Position3D< CoordinateType > const & shift );

/**
 * Perform a spherical interpolation between two vectors.
 * It uses spherical linear interpolation for the directon, and linear
 * interpolation for the radius component.
 */
template< typename CoordinateType >
VISR_RBBL_LIBRARY_SYMBOL Position3D< CoordinateType > interpolateSpherical(
    Position3D< CoordinateType > const & pos0,
    Position3D< CoordinateType > const & pos1,
    CoordinateType tInterp );

//@}

} // namespace rbbl
} // namespace visr

#endif // VISR_POSITION_3D_HPP_INCLUDED
