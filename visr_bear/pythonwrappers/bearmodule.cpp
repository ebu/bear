/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <pybind11/pybind11.h>

namespace bear
{
namespace python
{
void export_api(pybind11::module &m);
void export_debug_sink(pybind11::module &m);
void export_internals(pybind11::module &m);
void export_dynamic_renderer(pybind11::module &m);
} // namespace python
} // namespace bear

PYBIND11_MODULE(visr_bear, m)
{
  // TODO: split this into multiple modules, as VISR python bindings are only
  // required for testing
  pybind11::module::import("efl");
  pybind11::module::import("visr");
  pybind11::module::import("pml");

  using namespace bear::python;

  auto api_mod = m.def_submodule("api");
  export_api(api_mod);
  export_debug_sink(m);
  export_internals(m);
  export_dynamic_renderer(m);
}
