#include "catch2/catch.hpp"
#include "tensorfile.hpp"
#include "test_config.h"

using namespace tensorfile;

template <typename T>
void check_one_multi_dim(const TensorFile &tenf, const nlohmann::json &v)
{
  auto ndarray = tenf.unpack(v);
  auto ndarray_f = std::dynamic_pointer_cast<NDArrayT<T>>(ndarray);
  REQUIRE(ndarray_f);
  REQUIRE(ndarray_f->ndim() == 3);
  REQUIRE(ndarray_f->shape(0) == 2);
  REQUIRE(ndarray_f->shape(1) == 4);
  REQUIRE(ndarray_f->shape(2) == 8);

  for (size_t i = 0; i < 2; i++)
    for (size_t j = 0; j < 4; j++)
      for (size_t k = 0; k < 8; k++) {
        REQUIRE((*ndarray_f)(i, j, k) == (T)((i << 5) + (j << 3) + k));
      }
}

void check_multi_dim(const TensorFile &tenf)
{
  check_one_multi_dim<float>(tenf, tenf.metadata["multi_dim"]["float32"]);
  check_one_multi_dim<double>(tenf, tenf.metadata["multi_dim"]["float64"]);
  check_one_multi_dim<uint64_t>(tenf, tenf.metadata["multi_dim"]["uint64"]);
  check_one_multi_dim<uint32_t>(tenf, tenf.metadata["multi_dim"]["uint32"]);
  check_one_multi_dim<uint16_t>(tenf, tenf.metadata["multi_dim"]["uint16"]);
  check_one_multi_dim<uint8_t>(tenf, tenf.metadata["multi_dim"]["uint8"]);
  check_one_multi_dim<int64_t>(tenf, tenf.metadata["multi_dim"]["int64"]);
  check_one_multi_dim<int32_t>(tenf, tenf.metadata["multi_dim"]["int32"]);
  check_one_multi_dim<int16_t>(tenf, tenf.metadata["multi_dim"]["int16"]);
  check_one_multi_dim<int8_t>(tenf, tenf.metadata["multi_dim"]["int8"]);
}

TEST_CASE("read_le_32")
{
  auto tenf = read(BUNDLED_TEST_FILES "/tensorfile_le_32.tenf");
  check_multi_dim(tenf);
}

TEST_CASE("read_be_32")
{
  auto tenf = read(BUNDLED_TEST_FILES "/tensorfile_be_32.tenf");
  check_multi_dim(tenf);
}

TEST_CASE("read_le_1")
{
  auto tenf = read(BUNDLED_TEST_FILES "/tensorfile_le_1.tenf");
  check_multi_dim(tenf);
}

TEST_CASE("read_be_1")
{
  auto tenf = read(BUNDLED_TEST_FILES "/tensorfile_be_1.tenf");
  check_multi_dim(tenf);
}
