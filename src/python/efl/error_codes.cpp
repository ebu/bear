/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/error_codes.hpp> 

#include <pybind11/pybind11.h>

namespace visr
{
namespace efl
{
namespace python
{

namespace py = pybind11;
  
void exportErrorCode( py::module & m )
{
  py::enum_<visr::efl::ErrorCode>( m, "ErrorCode" ) 
    .value( "noError", visr::efl::ErrorCode::noError )
    .value( "alignmentError", visr::efl::ErrorCode::alignmentError )
    .value( "arithmeticError", visr::efl::ErrorCode::arithmeticError )
    .value( "logicError", visr::efl::ErrorCode::logicError )
    // to be continued
    ;
}

} // namespace python
} // namespace efl
} // namespace visr
