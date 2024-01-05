#include "tensorfile.hpp"

namespace bear {
/// get the version number of a BEAR data file
int get_data_file_version(tensorfile::TensorFile &tf);
/// check that the data file version is supported by this library
void check_data_file_version(tensorfile::TensorFile &tf);
};  // namespace bear
