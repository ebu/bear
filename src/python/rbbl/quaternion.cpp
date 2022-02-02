/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librbbl/quaternion.hpp>
#include <librbbl/position_3d.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <array>
#include <string>
#include <sstream>
#include <vector>

namespace visr
{
namespace rbbl
{
namespace python
{

namespace py = pybind11;

namespace 
{
  template< typename Coordinate >
  void exportQuaternion( pybind11::module & m, char const * className )
  {
    py::class_< Quaternion<Coordinate> >( m, className )
      .def( py::init<>(), "Default constructor" )
      .def( py::init< Quaternion< Coordinate > const & >(), py::arg( "rhs" ) )
      .def( py::init< Coordinate, Coordinate, Coordinate, Coordinate >(),
        py::arg( "w" ), py::arg( "x" ), py::arg( "y" ), py::arg( "z" ) )
      .def( py::init(
        []( std::array< Coordinate, 4> const & val )
        {
          return new Quaternion<Coordinate>( val[0], val[1], val[2], val[3] );
        }), py::arg( "val" ) )
      .def_static( "fromYPR", &Quaternion< Coordinate >::fromYPR, py::arg("yaw"),
        py::arg( "pitch" ), py::arg( "roll" ) )
      .def_static( "fromYPR", []( std::array< Coordinate, 3 > const & ypr )
        {
          return Quaternion< Coordinate >::fromYPR( ypr[0], ypr[1], ypr[2] );
        }, py::arg("ypr") )
      .def_static( "fromRotationVector", Quaternion< Coordinate >::fromRotationVector, 
         py::arg("rotVector"), py::arg( "angle" ) )
      .def_property( "w", &Quaternion< Coordinate >::w, &Quaternion< Coordinate >::setW )
      .def_property( "x", &Quaternion< Coordinate >::x, &Quaternion< Coordinate >::setX )
      .def_property( "y", &Quaternion< Coordinate >::y, &Quaternion< Coordinate >::setY )
      .def_property( "z", &Quaternion< Coordinate >::z, &Quaternion< Coordinate >::setZ )
      .def_property( "data",
         []( Quaternion< Coordinate > const & self )
         { return std::array< Coordinate, 4 >( {self.w(), self.x(), self.y(), self.z()} ); },
         []( Quaternion< Coordinate > & self, std::array< Coordinate, 4 > const & val )
         { self.set( val[0], val[1], val[2], val[3] ); }
         )
      .def_property_readonly( "yaw", &Quaternion< Coordinate >::yaw )
      .def_property_readonly( "pitch", &Quaternion< Coordinate >::pitch )
      .def_property_readonly( "roll", &Quaternion< Coordinate >::roll )
      .def_property_readonly( "rotationVector", &Quaternion< Coordinate >::rotationVector )
      .def_property_readonly( "rotationAngle", &Quaternion< Coordinate >::rotationAngle )
      .def( "set", &Quaternion< Coordinate >::set, py::arg("w"), py::arg("x"),
         py::arg("y"), py::arg("z") )
      .def( "setYPR", &Quaternion< Coordinate >::setYPR, py::arg("yaw"),
         py::arg("pitch"), py::arg("roll") )
      .def( "setRotationVector", &Quaternion< Coordinate >::setRotationVector,
         py::arg("rotVector"), py::arg("angle") )
      .def( "conjugate", &Quaternion< Coordinate >::conjugate )
      .def( "normalise", &Quaternion< Coordinate >::normalise,
         py::arg( "adjustSign" ) = false, py::arg( "silentDivideByZero" ) = false )
      .def( "norm", &Quaternion< Coordinate >::norm )
      .def( "normSquare", &Quaternion< Coordinate >::normSquare )
      .def( -py::self ) // Negation operator
      .def( py::self += py::self)
      .def( py::self -= py::self )
      .def( py::self *= py::self )
      .def( py::self *= Coordinate() )
      .def( py::self + py::self)
      .def( py::self - py::self)
      .def( py::self * py::self)
      .def( Coordinate() * py::self )
      .def( py::self / Coordinate() )
      .def( "rotate", &Quaternion< Coordinate >::rotate, py::arg( "rotation" ) )
    ;
      
    m.def( "conjugate", static_cast<Quaternion< Coordinate >(*)( Quaternion< Coordinate > const&)>(
      &visr::rbbl::conjugate) ) 
     .def( "angle", static_cast< Coordinate(*)( Quaternion< Coordinate > const &, Quaternion< Coordinate > const & )>
        (&visr::rbbl::angle), py::arg( "lhs" ), py::arg( "rhs" ) )
     .def( "angleNormalised", static_cast< Coordinate(*)( Quaternion< Coordinate > const &, Quaternion< Coordinate > const & )>
        (&visr::rbbl::angleNormalised), py::arg( "lhs" ), py::arg( "rhs" ) )
     .def( "normalise", static_cast< Quaternion< Coordinate >(*)( Quaternion< Coordinate > const &, bool, bool )>
        (&visr::rbbl::normalise), py::arg( "val" ), py::arg( "adjustSign" )=false, py::arg( "silentDivideByZero" )=false )
     .def( "angle", static_cast< Coordinate(*)( Quaternion< Coordinate > const &, Quaternion< Coordinate > const & )>
        (&visr::rbbl::angle), py::arg( "op1" ), py::arg( "op2" ) )
    .def( "dot", static_cast< Coordinate(*)( Quaternion< Coordinate > const &, Quaternion< Coordinate > const & )>
       (&visr::rbbl::dot), py::arg( "op1" ), py::arg( "op2" ) )
     .def( "interpolateSpherical", static_cast< Quaternion< Coordinate >(*)( Quaternion< Coordinate > const &,
        Quaternion< Coordinate > const &, Coordinate tInterp )>(&visr::rbbl::interpolateSpherical),
        py::arg( "pos0" ), py::arg( "pos1" ), py::arg( "tInterp" ) )
    ;
  }

} // unnamed namespace

void exportQuaternion( pybind11::module & m )
{
  exportQuaternion<float>( m, "Quaternion" );
  exportQuaternion<double>( m, "QuaternionDouble" );
}

} // namespace python
} // namepace rbbl
} // namespace visr
