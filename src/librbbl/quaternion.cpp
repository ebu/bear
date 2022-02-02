/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "quaternion.hpp"

#include "position_3d.hpp"

#include <boost/math/constants/constants.hpp>

#include <ciso646>
#include <limits>
#include <stdexcept>

namespace visr
{
namespace rbbl
{
template< typename CoordinateType >
Quaternion< CoordinateType >::Quaternion()
 : Quaternion{ 1.0f, 0.0f, 0.0f, 0.0f }
{
}

template< typename CoordinateType >
Quaternion< CoordinateType >::Quaternion( Coordinate w, Coordinate x,
                                          Coordinate y, Coordinate z,
                                          bool normalise /*= false*/ )
 : mQuat{ w, x, y, z }
{
}

template< typename CoordinateType >
Quaternion< CoordinateType >::Quaternion( InternalType const & impl )
 : mQuat( impl )
{
}

template< typename CoordinateType >
Quaternion< CoordinateType >::Quaternion( InternalType && impl )
 : mQuat( impl )
{
}

template< typename CoordinateType >
Quaternion< CoordinateType >::Quaternion(
    Quaternion< Coordinate > const& rhs ) = default;

template< typename CoordinateType >
Quaternion< CoordinateType >::Quaternion( Quaternion&& rhs ) = default;

template< typename CoordinateType >
/*static*/ Quaternion< CoordinateType > Quaternion< CoordinateType >::fromYPR(
    CoordinateType yaw, CoordinateType pitch, CoordinateType roll )
{
  Quaternion< CoordinateType > ret;
  ret.setYPR( yaw, pitch, roll );
  return ret;
}

template< typename CoordinateType >
/*static*/ Quaternion< CoordinateType >
Quaternion< CoordinateType >::fromRotationVector(
    Position3D< CoordinateType > const& rotVector, CoordinateType angle )
{
  Quaternion< Coordinate > ret;
  ret.setRotationVector( rotVector, angle );
  return ret;
}

template< typename CoordinateType >
Quaternion< CoordinateType >& Quaternion< CoordinateType >::operator=(
    Quaternion const& rhs ) = default;

template< typename CoordinateType >
Quaternion< CoordinateType >& Quaternion< CoordinateType >::operator=(
    Quaternion&& rhs ) = default;

template< typename CoordinateType >
Quaternion< CoordinateType >::~Quaternion() = default;

template< typename CoordinateType >
void Quaternion< CoordinateType >::setW( CoordinateType newW )
{
  mQuat = InternalType( newW, x(), y(), z() );
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::setX( CoordinateType newX )
{
  mQuat = InternalType( w(), newX, y(), z() );
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::setY( CoordinateType newY )
{
  mQuat = InternalType( w(), x(), newY, z() );
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::setZ( CoordinateType newZ )
{
  mQuat = InternalType( w(), x(), y(), newZ );
}

template< typename CoordinateType >
CoordinateType Quaternion< CoordinateType >::yaw() const
{
  // yaw = atan2( 2*(q.w*q.z+q.x*q.y), 1-2*(q.y * q.y + q.z * q.z) )
  using T = CoordinateType;
  return std::atan2( static_cast<T>(2.0)*(w()*z() +x()*y()),
   static_cast<T>(1.0)-static_cast<T>(2.0)*(y()*y()
     +z()*z()));
}

template< typename CoordinateType >
CoordinateType Quaternion< CoordinateType >::pitch() const
{
  CoordinateType const arg = static_cast< CoordinateType >(2.0)*(w()*y()-z()*x());
  // Check for 'out of range' arguments. 
  if( std::abs(arg) >= static_cast< CoordinateType >(1.0) )
  {
    return std::copysign( boost::math::constants::half_pi< CoordinateType >(), arg );
  }
  return std::asin( arg );
}

template< typename CoordinateType >
CoordinateType Quaternion< CoordinateType >::roll() const
{
  return std::atan2(
      static_cast< CoordinateType >( 2.0 ) * ( w() * x() + y() * z() ),
      static_cast< CoordinateType >( 1.0 ) -
          static_cast< CoordinateType >( 2.0 ) * ( x() * x() + y() * y() ) );
}

template< typename CoordinateType >
Position3D< CoordinateType > Quaternion< CoordinateType >::rotationVector()
    const
{
  using T = CoordinateType;
  T const norm{ std::sqrt(x() * x()
    + y() * y()
    + z() * z()) };
  if( norm <= std::numeric_limits<T>::epsilon() )
  {
    // Degenerate case, there is no rotation.
    // Return an arbitrary unit vector instead.
    return Position3D<T>{ 1.0f, 0.0f, 0.0f };
  }
  T const scale = 1.0f/norm;
  return Position3D< T >{ scale * x(), scale * y(), scale * z() };
}

template< typename CoordinateType >
CoordinateType Quaternion< CoordinateType >::rotationAngle() const
{
  using T = CoordinateType;
  T const norm{ std::sqrt(x() * x()
    + y() * y()
    + z() * z()) };
    return static_cast<T>( 2.0) * std::atan2( norm, w() );
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::set( Coordinate w, Coordinate x,
                                        Coordinate y, Coordinate z )
{
  mQuat = InternalType( w, x, y, z );
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::conjugate()
{
  mQuat = conj( mQuat );
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::setYPR( Coordinate yaw, Coordinate pitch,
                                           Coordinate roll )
{
  // Note: Conversion between Euler angles and quaternions inspired by
  // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
  CoordinateType const cy = std::cos( yaw * 0.5f );
  CoordinateType const sy = std::sin( yaw * 0.5f );
  CoordinateType const cp = std::cos( pitch * 0.5f );
  CoordinateType const sp = std::sin( pitch * 0.5f );
  CoordinateType const cr = std::cos( roll * 0.5f );
  CoordinateType const sr = std::sin( roll * 0.5f );
  mQuat =
      InternalType( cy * cp * cr + sy * sp * sr, cy * cp * sr - sy * sp * cr,
                    sy * cp * sr + cy * sp * cr, sy * cp * cr - cy * sp * sr );
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::setRotationVector(
    Position3D< CoordinateType > const& rotVec, CoordinateType angle )
{
  Position3D< Coordinate > const normVec{ rbbl::normalise( rotVec ) };
  Coordinate const sinPhi2{ std::sin( static_cast< Coordinate >( 0.5 ) *
                                      angle ) };
  mQuat = InternalType( std::cos( static_cast< Coordinate >( 0.5 ) * angle ),
                        sinPhi2 * normVec.x(), sinPhi2 * normVec.y(),
                        sinPhi2 * normVec.z() );
}

template< typename CoordinateType >
Quaternion< CoordinateType > 
conjugate( Quaternion< CoordinateType > const & quat )
{
  return Quaternion< CoordinateType >{ conj( quat.impl() ) };
}

template< typename CoordinateType >
CoordinateType Quaternion< CoordinateType >::norm() const
{
  return abs( impl() );
}

template< typename CoordinateType >
CoordinateType Quaternion< CoordinateType >::normSquare() const
{
  return w()*w() + x()*x() + y()*y()+z()*z();
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::normalise(bool adjustSign /*= false*/,
                                             bool silentDivideByZero /*= false*/ )
{
  CoordinateType const l2( norm() );
  if( not silentDivideByZero and (l2 < std::numeric_limits<CoordinateType>::epsilon() ))
  {
    throw std::runtime_error( "Quaternion::normalise(): Quaternion to close to zero." );
  }
  CoordinateType const scale = adjustSign and (w() < static_cast<CoordinateType>(0.0))
  ? static_cast<CoordinateType>(-1.0) : static_cast<CoordinateType>(1.0);
  operator*=( scale / l2 );
}

template< typename CoordinateType >
Quaternion< CoordinateType > &  Quaternion< CoordinateType >::operator+=( Quaternion const& rhs )
{
  mQuat += rhs.impl();
  return *this;
}

template< typename CoordinateType >
Quaternion< CoordinateType > & Quaternion< CoordinateType >::operator-=( Quaternion const& rhs )
{
  mQuat -= rhs.impl();
  return *this;
}

template< typename CoordinateType >
Quaternion< CoordinateType > & Quaternion< CoordinateType >::operator*=( Quaternion const& rhs )
{
  mQuat *= rhs.impl();
  return *this;
}

template< typename CoordinateType >
Quaternion< CoordinateType > & Quaternion< CoordinateType >::operator*=( CoordinateType scale )
{
  mQuat *= scale;
  return *this;
}

template< typename CoordinateType >
void Quaternion< CoordinateType >::rotate( Quaternion< CoordinateType > const & rot )
{
  mQuat = rot.impl() * mQuat;
}

// Implementaton of free functions.

template< typename CoordinateType >
Quaternion< CoordinateType > operator+(
    Quaternion< CoordinateType > const& lhs,
    Quaternion< CoordinateType > const& rhs )
{
  return Quaternion< CoordinateType >( lhs.impl() + rhs.impl() );
}

template< typename CoordinateType >
Quaternion< CoordinateType > operator-(
    Quaternion< CoordinateType > const& lhs,
    Quaternion< CoordinateType > const& rhs )
{
  return Quaternion< CoordinateType >( lhs.impl() - rhs.impl() );
}

template< typename CoordinateType >
Quaternion< CoordinateType > operator*(
    Quaternion< CoordinateType > const& lhs,
    Quaternion< CoordinateType > const& rhs )
{
  return Quaternion< CoordinateType >( lhs.impl() * rhs.impl() );
}

template< typename CoordinateType >
Quaternion< CoordinateType > operator-(
    Quaternion< CoordinateType > const& val )
{
  return Quaternion< CoordinateType >{ -val.impl() };
}

template< typename CoordinateType >
Quaternion< CoordinateType > operator*(
    CoordinateType scale, Quaternion< CoordinateType > const& val )
{
  return Quaternion< CoordinateType >{ scale * val.impl() };
}

template< typename CoordinateType >
Quaternion< CoordinateType > operator/(
     Quaternion< CoordinateType > const& val, CoordinateType scale )
{
  return Quaternion< CoordinateType >{
    static_cast<CoordinateType>( 1.0 )/scale * val.impl() };
}

template< typename CoordinateType >
CoordinateType angle( Quaternion< CoordinateType > const& q1,
                      Quaternion< CoordinateType > const& q2 )
{
  return angleNormalised( normalise( q1 ), normalise( q2 ) );
}

template< typename CoordinateType >
CoordinateType angleNormalised( Quaternion< CoordinateType > const& q1,
                                Quaternion< CoordinateType > const& q2 )
{
  using CT = CoordinateType;
  // See http://www.boris-belousov.net/2016/12/01/quat-dist/

  // First (w) component of the difference rotation quaternion q1*conj(q2)
  // Note: Despite the above formula, this is the standard dot product (according to the source)
  CT const wDot{ dot( q1, q2 ) };
  CT const wDiff{ std::sqrt( std::abs( wDot ) ) };
  // Clip using min/max to avoid arguments with  abs( x ) > 1.0 due to numerical errors.
  return static_cast<CT>(2.0)* std::acos( std::max( std::min( wDiff, static_cast<CT>(1.0) ),
    static_cast<CT>(-1.0) ) );
}

template< typename CoordinateType >
Quaternion< CoordinateType > normalise( Quaternion< CoordinateType > const& val,
                                        bool adjustSign /*=false*/,
                                        bool silentDivideByZero /*= false*/ )
{
  Quaternion< CoordinateType > ret{ val };
  ret.normalise( adjustSign, silentDivideByZero );
  return ret;
}

template< typename CoordinateType >
CoordinateType dot( Quaternion< CoordinateType > const& op1,
                    Quaternion< CoordinateType > const& op2 )
{
  return op1.w() * op2.w() + op1.x() * op2.x()
    + op1.y() * op2.y() + op1.z() *op2.z();
}

template< typename CoordinateType >
Quaternion< CoordinateType > 
interpolateSpherical( Quaternion< CoordinateType > const& pos0,
                      Quaternion< CoordinateType > const& pos1,
                      CoordinateType tInterp )
{
  // Example implementation
  // https://stackoverflow.com/questions/2879441/how-to-interpolate-rotations
  // def slerp(p0, p1, t):
  //       omega = arccos(dot(p0/norm(p0), p1/norm(p1)))
  //       so = sin(omega)
  //       return sin((1.0-t)*omega) / so * p0 + sin(t*omega)/so * p1
  // Note: This factor is independent of the actual interpolation position and 
  // could be reused.
  CoordinateType const omega = std::acos( 
    std::min( static_cast< CoordinateType >(1.0),
      std::max( static_cast< CoordinateType >(-1.0),
        dot( normalise(pos0), normalise(pos1) ) ) ) );
  CoordinateType const sinOmega = std::sin( omega );
  CoordinateType const sf1 = std::sin((static_cast<CoordinateType>(1.0)-tInterp)*omega) / sinOmega;
  CoordinateType const sf2 = std::sin(tInterp*omega) / sinOmega;
  return sf1 * pos0 + sf2 * pos1;
}

// Explicit instantiations
template class Quaternion< float >;
template class Quaternion< double >;

template VISR_RBBL_LIBRARY_SYMBOL Quaternion< float > 
conjugate< float >( Quaternion< float > const& );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< float > operator+< float >(
    Quaternion< float > const &,
    Quaternion< float > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< float > operator-< float >(
    Quaternion< float > const&,
    Quaternion< float > const& );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< float > operator*< float >(
    Quaternion< float > const&,
    Quaternion< float > const& );

template VISR_RBBL_LIBRARY_SYMBOL
float angle< float >(
    Quaternion< float > const &,
    Quaternion< float > const & );

template VISR_RBBL_LIBRARY_SYMBOL
float angleNormalised< float >(
    Quaternion< float > const &,
    Quaternion< float > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< float > operator-< float >(
    Quaternion< float > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< float > operator*< float >(
    float, Quaternion< float > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< float > operator/< float >(
    Quaternion< float > const&, float );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< float > normalise< float >(
    Quaternion< float > const &, bool, bool );

template VISR_RBBL_LIBRARY_SYMBOL
float dot< float >(
    Quaternion< float > const &,
    Quaternion< float > const & );

template VISR_RBBL_LIBRARY_SYMBOL Quaternion< float > interpolateSpherical< float >(
  Quaternion< float > const &,
  Quaternion< float > const&,
  float );


template VISR_RBBL_LIBRARY_SYMBOL Quaternion< double > 
conjugate< double >( Quaternion< double > const& );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< double > operator+< double >(
    Quaternion< double > const &,
    Quaternion< double > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< double > operator-< double >(
    Quaternion< double > const&,
    Quaternion< double > const& );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< double > operator*< double >(
    Quaternion< double > const&,
    Quaternion< double > const& );

template VISR_RBBL_LIBRARY_SYMBOL
double angle< double >(
    Quaternion< double > const &,
    Quaternion< double > const & );

template VISR_RBBL_LIBRARY_SYMBOL
double angleNormalised< double >(
    Quaternion< double > const &,
    Quaternion< double > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< double > operator-< double >(
    Quaternion< double > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< double > operator*< double >(
    double, Quaternion< double > const & );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< double > operator/< double >(
    Quaternion< double > const&, double );

template VISR_RBBL_LIBRARY_SYMBOL
Quaternion< double > normalise< double >(
    Quaternion< double > const &, bool, bool );

template VISR_RBBL_LIBRARY_SYMBOL
double dot< double >(
    Quaternion< double > const &,
    Quaternion< double > const & );

template VISR_RBBL_LIBRARY_SYMBOL Quaternion< double > interpolateSpherical< double >(
  Quaternion< double > const &,
  Quaternion< double > const&,
  double );

} // namespace rbbl
} // namespace visr
