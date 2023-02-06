#pragma once
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <string>
#include <vector>

namespace bear {
namespace python {

  // convert a python array of samples with a variable number of channels to a vector of channel pointers
  inline std::vector<float *> py_array_to_pointers_var(pybind11::array_t<float, 0> &arr,
                                                       size_t n_samples,
                                                       const std::string &name)
  {
    namespace py = pybind11;

    if (arr.ndim() != 2) throw py::value_error(name + " must have 2 dimensions");
    if ((size_t)arr.shape(1) != n_samples) throw py::value_error(name + " has wrong length");

    // check sample stride. in degenerate cases, the stride of contiguous
    // arrays no longer matches the item size, see
    // https://numpy.org/doc/1.24/reference/arrays.ndarray.html#internal-memory-layout-of-an-ndarray
    bool stride_invalid = arr.shape(0) == 0 || arr.shape(1) == 1;
    if (!stride_invalid && (size_t)arr.strides(1) != sizeof(float))
      throw py::value_error(name + " must have stride of 1 along samples");

    std::vector<float *> ptrs((size_t)arr.shape(0));
    for (size_t i = 0; i < ptrs.size(); i++) ptrs[i] = arr.mutable_data(i, 0);
    return ptrs;
  }

  // convert a python array of samples with a fixed number of channels to a vector of channel pointers
  inline std::vector<float *> py_array_to_pointers(pybind11::array_t<float, 0> &arr,
                                                   size_t n_channels,
                                                   size_t n_samples,
                                                   const std::string &name)
  {
    std::vector<float *> ptrs = py_array_to_pointers_var(arr, n_samples, name);

    if (ptrs.size() != n_channels) throw pybind11::value_error(name + " has wrong number of channels");

    return ptrs;
  }
}  // namespace python
}  // namespace bear
