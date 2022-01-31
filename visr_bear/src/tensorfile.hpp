#pragma once
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "rapidjson/document.h"

namespace tensorfile {

class format_error : public std::runtime_error {
 public:
  explicit format_error(const std::string &what) : std::runtime_error("tensorfile format error: " + what) {}
};

class MMap;

namespace detail {
  inline size_t calc_index(const size_t *strides) { return 0u; }

  template <typename Index, typename... Indices>
  inline size_t calc_index(const size_t *strides, Index index, Indices... indices)
  {
    return *strides * index + calc_index(strides + 1, indices...);
  }
}  // namespace detail

/// base for n-dimensional arrays of various types, just holds the metadata,
/// not the data because that is type-specific
class NDArray {
 public:
  const std::string &dtype() const { return dtype_; };
  size_t ndim() const { return shape_.size(); };

  const std::vector<size_t> &shape() const { return shape_; };
  size_t shape(size_t i) const { return shape_[i]; };

  const std::vector<size_t> &strides() const { return strides_; };
  size_t stride(size_t i) const { return strides_[i]; };

  template <typename... Indices>
  inline size_t idx(Indices... indices) const
  {
    assert(sizeof...(indices) == strides_.size());
    return detail::calc_index(strides_.data(), indices...);
  }

  virtual ~NDArray() {}

 protected:
  NDArray(std::string dtype, std::vector<size_t> shape, std::vector<size_t> strides)
      : dtype_(std::move(dtype)), shape_(std::move(shape)), strides_(std::move(strides))
  {
    if (shape_.size() != strides_.size()) throw std::invalid_argument("size of shape and strides must match");
  }
  std::string dtype_;
  std::vector<size_t> shape_;
  std::vector<size_t> strides_;
};

/// concrete n-dimensional array with a specific type
template <typename T>
struct NDArrayT : public NDArray {
 public:
  const T *data() const { return base_ptr; }

  template <typename... Indices>
  inline const T &operator()(Indices... indices) const
  {
    return base_ptr[idx(indices...)];
  }

  T *base_ptr;

 protected:
  NDArrayT(std::string dtype, std::vector<size_t> shape, std::vector<size_t> strides, T *base_ptr)
      : NDArray(std::move(dtype), std::move(shape), std::move(strides)), base_ptr(base_ptr)
  {
  }
};

class TensorFile {
 public:
  TensorFile(std::shared_ptr<MMap> mmap, rapidjson::Document metadata)
      : metadata(std::move(metadata)), mmap(std::move(mmap))
  {
  }
  rapidjson::Document metadata;

  std::shared_ptr<NDArray> unpack(const rapidjson::Value &v) const;

  template <typename T>
  std::shared_ptr<NDArrayT<T>> unpack(const rapidjson::Value &v) const
  {
    return std::dynamic_pointer_cast<NDArrayT<T>>(unpack(v));
  }

 private:
  std::shared_ptr<MMap> mmap;
};

TensorFile read(const std::string &path);

}  // namespace tensorfile
