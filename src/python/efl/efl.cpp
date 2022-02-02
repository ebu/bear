/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/initialise_library.hpp>

#include <pybind11/pybind11.h>

namespace visr
{
namespace efl
{
namespace python
{
void exportBasicVectors( pybind11::module & m );
void exportBasicMatrices( pybind11::module & m );
void exportDenormalisedNumberHandling( pybind11::module & m );
void exportErrorCode( pybind11::module & m );
void exportFilterFunctions( pybind11::module & m );
void exportInitialiseLibrary( pybind11::module & m );
void exportVectorConversions( pybind11::module & m );
void exportVectorFunctions( pybind11::module & m );
} // namespace python
} // namespace efl
} // namespace visr

PYBIND11_MODULE( efl, m )
{
  using namespace visr::efl::python;
  // Initialise the library (set function dispatchers, etc.
  visr::efl::initialiseLibrary();
  exportBasicVectors( m );
  exportBasicMatrices( m );
  exportDenormalisedNumberHandling( m );
  exportErrorCode( m );
  exportFilterFunctions( m );
  exportInitialiseLibrary( m );
  exportVectorConversions( m );
  exportVectorFunctions( m );
}
