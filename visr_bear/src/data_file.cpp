#include "data_file.hpp"

#include <string>

namespace bear {
int get_data_file_version(tensorfile::TensorFile &tf)
{
  auto it = tf.metadata.FindMember("bear_data_version");
  if (it == tf.metadata.MemberEnd()) return 0;

  if (!it->value.IsInt()) throw std::runtime_error("version must be an integer");

  return it->value.GetInt();
}

void check_data_file_version(tensorfile::TensorFile &tf)
{
  int version = get_data_file_version(tf);
  if (version > 0)
    throw std::runtime_error("data file version mismatch: expected 0, got " + std::to_string(version));
}
}  // namespace bear
