#include "../submodules/libear/src/hoa/hoa.hpp"
#include "../submodules/libear/tests/eigen_utils.hpp"
#include "catch2/catch.hpp"
#include "sh_rotation.hpp"

using namespace bear;

Eigen::VectorXd encode(int order, Eigen::Vector3d pos)
{
  double az = -std::atan2(pos(0), pos(1));
  double el = std::atan2(pos(2), std::hypot(pos(0), pos(1)));

  int nch = (order + 1) * (order + 1);
  Eigen::VectorXd res(nch);
  for (size_t acn = 0; acn < nch; acn++) {
    int n, m;
    std::tie(n, m) = ear::hoa::from_acn(acn);
    res(acn) = ear::hoa::sph_harm(n, m, az, el, ear::hoa::norm_N3D);
  }

  return res;
}

TEST_CASE("rotation")
{
  for (int order = 1; order < 4; order++) {
    for (int iter = 1; iter < 4; iter++) {
      Eigen::Vector4d qp = Eigen::Vector4d::Random();
      qp.normalize();
      Eigen::Quaterniond q(qp);

      size_t n_channels = (order + 1) * (order + 1);
      Eigen::MatrixXd M(n_channels, n_channels);
      quaternion_to_sh_rotation_matrix(q, order, M);

      for (size_t sample = 0; sample < 100; sample++) {
        Eigen::Vector3d pos = Eigen::Vector3d::Random();
        pos.normalize();
        Eigen::VectorXd sh = encode(order, pos);

        Eigen::VectorXd sh_rot = M * sh;
        Eigen::Vector3d pos_rot = q * pos;

        Eigen::VectorXd sh_rot_expected = encode(order, pos_rot);

        REQUIRE_THAT(sh_rot, IsApprox(sh_rot_expected));
      }
    }
  }
}

#ifdef CATCH_CONFIG_ENABLE_BENCHMARKING
TEST_CASE("bench", "[benchmark]")
{
  Eigen::Vector4d qp = Eigen::Vector4d::Random();
  qp.normalize();
  Eigen::Quaterniond q(qp);

  {
    size_t n_channels = 2 * 2;
    Eigen::MatrixXd result(n_channels, n_channels);
    BENCHMARK("calc_transform 1")
    {
      quaternion_to_sh_rotation_matrix(q, 1, result);
      return result.sum();
    };
  }

  {
    size_t n_channels = 3 * 3;
    Eigen::MatrixXd result(n_channels, n_channels);
    BENCHMARK("calc_transform 2")
    {
      quaternion_to_sh_rotation_matrix(q, 2, result);
      return result.sum();
    };
  }

  {
    size_t n_channels = 4 * 4;
    Eigen::MatrixXd result(n_channels, n_channels);
    BENCHMARK("calc_transform 3")
    {
      quaternion_to_sh_rotation_matrix(q, 3, result);
      return result.sum();
    };
  }

  {
    size_t n_channels = 5 * 5;
    Eigen::MatrixXd result(n_channels, n_channels);
    BENCHMARK("calc_transform 4")
    {
      quaternion_to_sh_rotation_matrix(q, 4, result);
      return result.sum();
    };
  }
}
#endif
