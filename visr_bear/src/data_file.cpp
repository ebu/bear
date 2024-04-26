#include "data_file.hpp"

#include <string>

namespace bear {
int get_data_file_version(tensorfile::TensorFile &tf)
{
  auto it = tf.metadata.find("bear_data_version");
  if (it == tf.metadata.end()) return 0;

  if (!it->is_number_integer()) throw std::runtime_error("version must be an integer");

  return it->template get<int>();
}

void check_data_file_version(tensorfile::TensorFile &tf)
{
  int version = get_data_file_version(tf);
  if (version > 0)
    throw std::runtime_error("data file version mismatch: expected 0, got " + std::to_string(version));
}
}  // namespace bear
