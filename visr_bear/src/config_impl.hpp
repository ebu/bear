#pragma once
#include <cstddef>
#include <string>

namespace bear {
struct ConfigImpl {
  size_t num_objects_channels = 0;
  size_t num_direct_speakers_channels = 0;
  size_t num_hoa_channels = 0;
  size_t period_size = 0;
  size_t sample_rate = 48000;
  std::string data_path = "";
  std::string fft_implementation = "default";
};
};  // namespace bear
