#pragma once
#include <array>
#include <cstddef>
#include <stdexcept>

#include "config_impl.hpp"

namespace bear {
inline size_t num_input_channels(const ConfigImpl &config)
{
  return config.num_objects_channels + config.num_direct_speakers_channels + config.num_hoa_channels;
}

/// helper to convert between N-dimensional indices and a 1-dimensional
/// flattened representation
template <int N>
class Indexer {
 public:
  template <typename... Sizes>
  Indexer(Sizes... sizes_) : sizes{sizes_...}
  {
    size_t stride = 1;
    for (int i = N - 1; i >= 0; i--) {
      strides[i] = stride;
      stride *= sizes[i];
    }
  }

  template <typename... Indices>
  size_t operator()(Indices... indices)
  {
    size_t indices_arr[N] = {indices...};
    size_t idx = 0;
    for (size_t i = 0; i < N; i++) idx += strides[i] * indices_arr[i];
    return idx;
  }

  std::array<size_t, N> sizes;
  std::array<size_t, N> strides;
};

inline void bear_assert(bool condition, const std::string &message)
{
  if (!condition) throw std::logic_error(message);
}

}  // namespace bear
