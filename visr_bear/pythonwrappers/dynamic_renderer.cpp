#include "dynamic_renderer.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "array_conversion.hpp"
#include "boost_variant.hpp"
#include "config_impl.hpp"

namespace bear {
namespace python {

  namespace py = pybind11;

  void export_dynamic_renderer(pybind11::module &m)
  {
    using namespace ear;

    struct RendererWrapper : public DynamicRenderer {
      RendererWrapper(size_t block_size, size_t max_size)
          : DynamicRenderer(block_size, max_size), block_size(block_size)
      {
      }
      size_t block_size;
    };

    py::class_<RendererWrapper>(m, "DynamicRenderer")
        .def(py::init<size_t, size_t>())
        .def("set_config", &RendererWrapper::set_config)
        .def("set_config_blocking", &RendererWrapper::set_config_blocking)
        // pybind11 exception translation only applies to exceptions currently
        // being thrown, so this is the easiest way to wrap get_error
        .def("throw_error",
             [](RendererWrapper &r) {
               std::exception_ptr e = r.get_error();
               if (e) std::rethrow_exception(e);
             })
        .def("is_running", &RendererWrapper::is_running)
        .def("add_objects_block", &RendererWrapper::add_objects_block)
        .def("add_direct_speakers_block", &RendererWrapper::add_direct_speakers_block)
        .def("add_hoa_block", &RendererWrapper::add_hoa_block)
        .def(
            "process",
            [](RendererWrapper &r,
               py::array_t<float, 0> objects_input,
               py::array_t<float, 0> direct_speakers_input,
               py::array_t<float, 0> hoa_input,
               py::array_t<float, 0> output) {
              std::vector<float *> objects_input_ptrs =
                  py_array_to_pointers_var(objects_input, r.block_size, "objects_input");

              std::vector<float *> direct_speakers_input_ptrs =
                  py_array_to_pointers_var(direct_speakers_input, r.block_size, "direct_speakers_input");
              std::vector<float *> hoa_input_ptrs =
                  py_array_to_pointers_var(hoa_input, r.block_size, "hoa_input");
              std::vector<float *> output_ptrs = py_array_to_pointers(output, 2, r.block_size, "output");

              r.process(objects_input_ptrs.size(),
                        objects_input_ptrs.data(),
                        direct_speakers_input_ptrs.size(),
                        direct_speakers_input_ptrs.data(),
                        hoa_input_ptrs.size(),
                        hoa_input_ptrs.data(),
                        output_ptrs.data());
            },
            py::arg("objects_input").noconvert(true),
            py::arg("direct_speakers_input").noconvert(true),
            py::arg("hoa_input").noconvert(true),
            py::arg("output").noconvert(true))
        .def("get_block_start_time", &RendererWrapper::get_block_start_time)
        .def("set_block_start_time", &RendererWrapper::set_block_start_time)
        .def("set_listener", &RendererWrapper::set_listener);
    ;
  }
}  // namespace python
}  // namespace bear
