#include <pybind11/pybind11.h>

#include "../pythonwrappers/boost_variant.hpp"
#include "runner.hpp"

namespace py = pybind11;
using namespace bear::demo;

PYBIND11_MODULE(bear_demo, m)
{
  py::module::import("audiointerfaces");

  py::class_<Runner>(m, "Runner")
      .def(py::init<const std::string &, const std::string &, const bear::Config &>())
      .def("start", &Runner::start)
      .def("stop", &Runner::stop)
      .def("add_objects_block", &Runner::add_objects_block)
      .def("add_direct_speakers_block", &Runner::add_direct_speakers_block)
      .def("set_listener", &Runner::set_listener);
}
