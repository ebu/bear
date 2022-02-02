/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpml/listener_position.hpp>

#include <libpml/empty_parameter_config.hpp>


#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <ciso646>
#include <vector>
#include <sstream>

namespace visr
{

using pml::ListenerPosition;

namespace python
{
namespace pml
{

namespace py = pybind11;

void exportListenerPosition( pybind11::module & m)
{
  py::class_<ListenerPosition, ParameterBase> lp( m, "ListenerPosition" );

  py::enum_<ListenerPosition::RotationFormat>( lp, "RotationFormat" )
    .value( "YPR", ListenerPosition::RotationFormat::YPR )
    .value( "RotationVector", ListenerPosition::RotationFormat::RotationVector )
    .value( "Quaternion", ListenerPosition::RotationFormat::Quaternion )
  ;

  lp
    .def_property_readonly_static( "staticType", [](py::object /*self*/) {return ListenerPosition::staticType(); } )
    .def( py::init<visr::pml::EmptyParameterConfig const &>(), py::arg("config") = visr::pml::EmptyParameterConfig() )
    .def( py::init<ListenerPosition::PositionType const &, ListenerPosition::OrientationQuaternion const &>(), py::arg("position"),
      py::arg("quaternion") )
    .def( py::init<ListenerPosition::Coordinate, ListenerPosition::Coordinate, ListenerPosition::Coordinate,
      ListenerPosition::Coordinate, ListenerPosition::Coordinate, ListenerPosition::Coordinate>(), py::arg("x"), py::arg("y"), py::arg("z") = 0.0f,
      py::arg("yaw")=0.0f, py::arg("pitch")=0.0f, py::arg("roll") = 0.0f)
    .def( py::init<ListenerPosition::PositionType const &, ListenerPosition::OrientationYPR const &>(), py::arg("position"),
      py::arg("orientation") = ListenerPosition::OrientationYPR{ {0.0f,0.0f,0.0f} } )
    .def( py::init( []( std::array<ListenerPosition::Coordinate, 3> const & pos,
       std::array<ListenerPosition::Coordinate, 3> const & ypr )
       {
         return ListenerPosition( pos[0], pos[1], pos[2], ypr[0], ypr[1], ypr[2] );
      } ), py::arg( "pos" ), py::arg("orientation") = std::array<ListenerPosition::Coordinate, 3>{ {0.0f,0.0f,0.0f} } )
    .def( py::init<ListenerPosition const &>() ) // Copy constructor
    .def_static( "fromQuaternion",
      []( ListenerPosition::PositionType const & pos,
          ListenerPosition::OrientationQuaternion const & quat )
      { return ListenerPosition( pos, quat ); },
      py::arg( "position"), py::arg( "quaternion" ) )
    .def_static( "fromQuaternion",
      []( std::array<ListenerPosition::Coordinate, 3> const & pos,
          std::array<ListenerPosition::Coordinate, 4> const & quat )
      { return ListenerPosition( ListenerPosition::PositionType(pos[0], pos[1], pos[2] ),
         ListenerPosition::OrientationQuaternion( quat[0], quat[1], quat[2], quat[3] ) ); },
      py::arg( "position"), py::arg( "quaternion" ) )
    .def_static( "fromRotationVector", &ListenerPosition::fromRotationVector,
      py::arg( "position"), py::arg( "rotationVector" ), py::arg( "rotationAngle" ) )
    .def_static( "fromRotationVector",
      []( std::array<ListenerPosition::Coordinate, 3> const & pos,
          std::array<ListenerPosition::Coordinate, 3> const & rot, ListenerPosition::Coordinate angle )
      { return ListenerPosition::fromRotationVector( ListenerPosition::PositionType(pos[0], pos[1], pos[2] ),
         ListenerPosition::PositionType( rot[0], rot[1], rot[2] ), angle ); },
      py::arg( "position"), py::arg( "rotationVector" ), py::arg( "rotationAngle" ) )
    .def_static( "fromJson", static_cast<ListenerPosition(*)(std::string const &)>(&ListenerPosition::fromJson) )
    .def( "parseJson", static_cast<void(ListenerPosition::*)(std::string const &)>(&ListenerPosition::parseJson), py::arg("string") )
    .def( "writeJson", static_cast<std::string(ListenerPosition::*)(ListenerPosition::RotationFormat, bool)const>(&ListenerPosition::writeJson),
      py::arg("rotationFormat")=ListenerPosition::RotationFormat::Quaternion,
      py::arg("prettyPrint")=false )
    .def_property( "x", &ListenerPosition::x, &ListenerPosition::setX )
    .def_property( "y", &ListenerPosition::y, &ListenerPosition::setY )
    .def_property( "z", &ListenerPosition::z, &ListenerPosition::setZ )
    .def_property( "position", &ListenerPosition::position,
      static_cast<void(ListenerPosition::*)(ListenerPosition::PositionType const &)>(&ListenerPosition::setPosition) )
    .def_property_readonly( "yaw", &ListenerPosition::yaw)
    .def_property_readonly( "pitch", &ListenerPosition::pitch )
    .def_property_readonly( "roll", &ListenerPosition::roll )
    .def_property_readonly( "orientationRotationVector", &ListenerPosition::orientationRotationVector )
    .def_property_readonly( "orientationRotationAngle", &ListenerPosition::orientationRotationAngle )
    .def_property( "orientationQuaternion", &ListenerPosition::orientationQuaternion,
      &ListenerPosition::setOrientationQuaternion )
    .def_property( "orientationYPR", &ListenerPosition::orientationYPR, 
                   static_cast<void(ListenerPosition::*)(ListenerPosition::OrientationYPR const &)>(&ListenerPosition::setOrientationYPR) )
    .def( "setOrientationRotationVector", &ListenerPosition::setOrientationRotationVector, 
      py::arg( "rotationVector" ), py::arg( "rotationAngle" ) )
    .def_property( "faceId", &ListenerPosition::faceID, &ListenerPosition::setFaceID )
    .def_property( "timeStamp", &ListenerPosition::timeNs, &ListenerPosition::setTimeNs )
    .def_property( "numberOfListeners", &ListenerPosition::numberOfListeners,
       &ListenerPosition::setNumberOfListeners )
    .def( "translate", &ListenerPosition::translate, py::arg( "translationVector" ) )
    .def( "translate", []( ListenerPosition & self, std::array< ListenerPosition::Coordinate, 3 > const & pos )
      { self.translate( ListenerPosition::PositionType( pos[0], pos[1], pos[2] ) ); }, py::arg( "translationVector" ) )
    .def( "rotate", &ListenerPosition::rotate, py::arg( "rotationQuaternion" ) )
    .def( "rotateOrientation", &ListenerPosition::rotateOrientation, py::arg( "rotationQuaternion" ) )
    .def( "transform", &ListenerPosition::transform,  py::arg( "translationVector" ), py::arg( "rotationQuaternion" ) )
    .def("__str__", []( ListenerPosition const & pos )
         {
           std::stringstream desc;
           desc << pos;
           return desc.str();
         }, "Create a printable representation" )
    ;
}

} // namepace pml
} // namespace python
} // namespace visr
