/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpanning/VBAP.h>
#include <libpanning/defs.h>
#include <libpanning/LoudspeakerArray.h>
#include <libpml/biquad_parameter.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace visr
{
using panning::VBAP;
using panning::LoudspeakerArray;

namespace python
{
namespace panning
{

namespace py = pybind11;

void exportVBAP(pybind11::module & m)
{
  pybind11::class_<VBAP>(m, "VBAP")
    .def(pybind11::init<const LoudspeakerArray &, SampleType, SampleType, SampleType>(), pybind11::arg("loudspeakerArray"),
         pybind11::arg("listenerPosX")=0.0,
         pybind11::arg("listenerPosY")=0.0,
         pybind11::arg("listenerPosZ")=0.0)
    .def( "calculateGains",
          [](VBAP const & self, SampleType x, SampleType y, SampleType z, pybind11::array_t<SampleType> & gains, bool planeWave)
          { self.calculateGains(x, y, z, static_cast<SampleType*>(gains.request().ptr), planeWave); },
          py::arg("x"), py::arg("y"), py::arg("z"), py::arg("gains"), py::arg("planeWave") = false)
    .def( "calculateGainsUnNormalised",
          []( VBAP const & self, SampleType x , SampleType y, SampleType z, pybind11::array_t<SampleType> & gains, bool planeWave )
          { self.calculateGainsUnNormalised( x, y, z, static_cast<SampleType*>(gains.request().ptr), planeWave ); },
          py::arg("x"), py::arg("y"), py::arg("z"), py::arg("gains"), py::arg("planeWave")=false )
    .def("setListenerPosition", &VBAP::setListenerPosition, py::arg("x"), py::arg("y"), py::arg("z") )
    ;
}

} // namepace panning
} // namespace python
} // namespace visr
