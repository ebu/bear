/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpanningdsp/types.hpp>
#include <libpanningdsp/initialise_parameter_library.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Forward declarations
namespace visr
{
namespace panningdsp
{
namespace python
{
void exportPanningGainMatrix( py::module & m );
void exportPanningMatrixParameter(py::module & m);
} // namespace python
} // namespace panningdsp
} // namespace visr

PYBIND11_MODULE(panningdsp, m)
{
  py::module::import( "visr" );
  py::module::import( "efl" );
  py::module::import( "pml" );

  m.doc() =
R"(VISR component library for panning-specific processing.
)";

  visr::panningdsp::initialiseParameterLibrary();

  using namespace visr::panningdsp::python;
  // Define the literal constant.
  m.attr( "timeStampInfinity" ) = py::int_( visr::panningdsp::cTimeStampInfinity );

  exportPanningGainMatrix( m );
  exportPanningMatrixParameter(m);
}
