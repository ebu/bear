/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/denormalised_number_handling.hpp>

#include <pybind11/pybind11.h>

#include <iomanip>
#include <sstream>
#include <string>

namespace visr
{
namespace efl
{
namespace python
{

namespace py = pybind11;

void exportDenormalisedNumberHandling( py::module & m)
{
  py::class_< DenormalisedNumbers > dnh( m, "DenormalisedNumbers" );
  
  py::class_< DenormalisedNumbers::State >( dnh, "State" )
    .def( py::init< bool, bool >(), py::arg( "flushToZero" ) = false,
      py::arg( "denormalsAreZero" ) = false )
    .def_property( "flushToZero",
      []( DenormalisedNumbers::State const & self ){ return self.flushToZero; },
      []( DenormalisedNumbers::State & self, bool val ){ self.flushToZero = val; })
    .def_property( "denormalsAreZero",
      []( DenormalisedNumbers::State const & self ){ return self.denormalsAreZero; },
      []( DenormalisedNumbers::State & self, bool val ){ self.denormalsAreZero = val; })
    .def( "__str__",
      [](DenormalisedNumbers::State const & self)
      {
        std::stringstream str;
        str << std::boolalpha << "flushToZero: " << self.flushToZero
            << " denormalsAreZero: " << self.denormalsAreZero;
        return str.str();
      })
  ;
  
  dnh
    .def_static( "setDenormHandling",
      []()
      {
        return DenormalisedNumbers::setDenormHandling();
      } )
    .def_static( "setDenormHandling",
      []( DenormalisedNumbers::State newState )
      {
        return DenormalisedNumbers::setDenormHandling( newState );
      }, py::arg( "newState" ) )
    .def_static( "setDenormHandling",
      []( bool ftz, bool daz )
      {
        DenormalisedNumbers::State const state{ ftz, daz };
        return DenormalisedNumbers::setDenormHandling( state );
      }, py::arg( "flushToZero" ) = false, py::arg( "denormalsAreZero" ) = false )
    .def_static( "resetDenormHandling",
      []( DenormalisedNumbers::State stateToRestore )
      {
        return DenormalisedNumbers::resetDenormHandling( stateToRestore );
      }, py::arg( "stateToRestore" ) )
  ;
}

} // namespace python
} // namespace efl
} // namespace visr
