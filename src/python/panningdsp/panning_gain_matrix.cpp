/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpanningdsp/panning_gain_matrix.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <libefl/basic_matrix.hpp>

#include <python/libpythonbindinghelpers/matrix_from_ndarray.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

namespace visr
{
namespace panningdsp
{
namespace python
{

void exportPanningGainMatrix( py::module & m )
{
  using visr::panningdsp::PanningGainMatrix;

  py::class_<PanningGainMatrix, visr::AtomicComponent>
    pgm( m, "PanningGainMatrix" );
  pgm
   .def( py::init<visr::SignalFlowContext const&, char const *, visr::CompositeComponent*,
     std::size_t, std::size_t, efl::BasicMatrix<PanningGainMatrix::SampleType> const &>(),
      py::arg("context"), py::arg("name"), py::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
      py::arg( "numberOfObjects" ),
      py::arg( "numberOfLoudspeakers" ),
      py::arg( "initialGains" )
     )
    //.def(py::init<visr::SignalFlowContext const&, char const *, visr::CompositeComponent*,
    //  std::size_t, std::size_t >(),
    //  py::arg("context"), py::arg("name"), py::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
    //  py::arg("numberOfObjects"),
    //  py::arg("numberOfLoudspeakers"),
    //  "Construct a panning matrix with zero initial gains."
    //)

    .def(py::init(
      [](visr::SignalFlowContext const & context, char const * name, visr::CompositeComponent * parent,
         std::size_t numberOfObjects, std::size_t numberOfLoudspeakers, py::array_t< PanningGainMatrix::SampleType> const & gains)
         {
           efl::BasicMatrix<PanningGainMatrix::SampleType> const gainMtx
             = visr::python::bindinghelpers::matrixFromNdArray(gains);
           return new PanningGainMatrix(context, name, parent, numberOfObjects, numberOfLoudspeakers, gainMtx );
         }),
      py::arg("context"), py::arg("name"), py::arg("parent") = static_cast<visr::CompositeComponent*>(nullptr),
      py::arg("numberOfObjects"),
      py::arg("numberOfLoudspeakers"),
      py::arg("initialGains")
    )
    ;
}

} // namepace python
} // namespace panningdsp
} // namespace visr
