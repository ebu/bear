/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librbbl/position_3d.hpp>
#include <librbbl/quaternion.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <array>
#include <string>
//#include <sstream>
//#include <vector>

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
  void exportPosition3D( pybind11::module & m, char const * className )
  {
    // Typedef to make typecasts below slightly less ugly.
    using ConstAccessor = Coordinate(Position3D< Coordinate >::*)() const;
    py::class_< Position3D< Coordinate > >( m, className, py::buffer_protocol() )
      .def( py::init<>(), "Default constructor" )
      .def( py::init<Coordinate, Coordinate, Coordinate >(),
         py::arg("x"), py::arg("y"), py::arg("z") )
      .def( py::init([]( std::array< Coordinate, 3 > const & vec )
        { return new Position3D< Coordinate >{ vec[0], vec[1], vec[2]}; } ),
        py::arg( "vec" ))
      .def_property( "x", static_cast<ConstAccessor>(&Position3D< Coordinate >::x),
        &Position3D< Coordinate >::setX )
      .def_property( "y", static_cast<ConstAccessor>(&Position3D< Coordinate >::y),
        &Position3D< Coordinate >::setY )
      .def_property( "z", static_cast<ConstAccessor>(&Position3D< Coordinate >::z),
        &Position3D< Coordinate >::setZ )
      .def_buffer( []( Position3D< Coordinate > & pos) -> py::buffer_info
        {
          return py::buffer_info(
            &pos.x(),                                    /* Pointer to buffer */
            sizeof( Coordinate),                         /* Size of one scalar */
            py::format_descriptor<Coordinate>::format(), /* Python struct-style format descriptor */
            1,                                           /* Number of dimensions */
            { 3 },                                       /* Buffer dimensions */
            { sizeof( Coordinate) }                      /* Strides (in bytes) for each index */
          );
        } )
      .def( "__getitem__", []( Position3D< Coordinate > const & pos, std::size_t idx )
         { return pos[ idx ]; }, py::arg( "index" ) )
      .def( "__setitem__", []( Position3D< Coordinate > & pos, std::size_t idx, Coordinate val )
         { pos[ idx ] = val; }, py::arg( "index" ), py::arg( "value" ) )
      .def( "set", &Position3D< Coordinate >::set )
      .def( "set", []( Position3D< Coordinate > & self, 
         std::array< Coordinate, 3 > const & vec )
         { self.set( vec[0], vec[1], vec[2] ); },
        py::arg( "vec" ) )
      .def( "norm", &Position3D< Coordinate >::norm )
      .def( "normSquare", &Position3D< Coordinate >::normSquare )
      .def( "normalise", &Position3D< Coordinate >::normalise,
        py::arg( "silentDivideByZero" ) = false )
      .def( -py::self ) // Unary minus (negation) operator
      .def( py::self += py::self )
      .def( py::self -= py::self )
      .def( py::self *= Coordinate() )
      .def( py::self + py::self )
      .def( py::self - py::self )
      .def( Coordinate() * py::self )
      .def( py::self / Coordinate() )
      .def( "rotate", &Position3D< Coordinate >::rotate, py::arg( "rot" ) )
      .def( "translate", &Position3D< Coordinate >::translate, py::arg( "shift" ) )
      .def( "transform", &Position3D< Coordinate >::transform, py::arg( "rot" ),
        py::arg( "shift" ) )
    ;
    m
     .def( "angle", static_cast<Coordinate (*)( Position3D< Coordinate > const&, Position3D< Coordinate > const &)>(&visr::rbbl::angle), py::arg( "lhs" ), py::arg( "rhs" ) )
     .def( "angleNormalised", static_cast<Coordinate (*)( Position3D< Coordinate > const&, Position3D< Coordinate > const &)>(&visr::rbbl::angleNormalised), py::arg( "lhs" ), py::arg( "rhs" ) )
     .def( "normalise", static_cast< Position3D< Coordinate > (*)(
        Position3D< Coordinate > const &, bool )>(
          &visr::rbbl::normalise), py::arg( "val" ), py::arg( "silentDivideByZero" ) = false )
     .def( "dot", static_cast<Coordinate (*)( Position3D< Coordinate > const&,
       Position3D< Coordinate > const &)>(&visr::rbbl::dot), py::arg( "lhs" ), py::arg( "rhs" ) )
     .def( "rotate", static_cast< Position3D< Coordinate > (*)(
        Position3D< Coordinate > const &, Quaternion< Coordinate > const & )>(
          &visr::rbbl::rotate), py::arg( "val" ), py::arg( "rotation" ) )
     .def( "transform", static_cast< Position3D< Coordinate > (*)(
        Position3D< Coordinate > const &, Quaternion< Coordinate > const &,
          Position3D< Coordinate > const & )>(
          &visr::rbbl::transform), py::arg( "val" ), py::arg( "rotation" ),
          py::arg( "shift" ) )
     .def( "interpolateSpherical", static_cast< Position3D< Coordinate >(*)(
        Position3D< Coordinate > const&, Position3D< Coordinate > const &, Coordinate ) >(
        &visr::rbbl::interpolateSpherical), py::arg( "pos0" ), py::arg( "pos1" ),
        py::arg( "tInterp" ) )
    ;
  }
} // unnamed namespace

void exportPosition3D( pybind11::module & m )
{
  exportPosition3D<float>( m, "Position3D" );
  exportPosition3D<double>( m, "Position3DDouble" );
}

} // namespace python
} // namepace rbbl
} // namespace visr
