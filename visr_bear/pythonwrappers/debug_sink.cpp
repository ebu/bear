#include "debug_sink.hpp"

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>

namespace bear {
namespace python {

  namespace py = pybind11;

  void export_debug_sink(pybind11::module &m)
  {
    m.def("get_debug_sink", [](std::string name) { return bear::DebugStorage::get_instance().get(name); });
  }

}  // namespace python
}  // namespace bear
