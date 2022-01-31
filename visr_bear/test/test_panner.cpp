#include "catch2/catch.hpp"
#include "ear/common_types.hpp"
#include "ear/metadata.hpp"
#include "panner.hpp"
#include "test_config.h"

using namespace Eigen;
TEST_CASE("basic")
{
  bear::Panner panner(DEFAULT_TENSORFILE_NAME);

  ear::ObjectsTypeMetadata otm;
  otm.position = ear::PolarPosition{0.0, 0.0, 1.0};

  VectorXd direct_gains(panner.num_gains());
  VectorXd diffuse_gains(panner.num_gains());
  panner.calc_objects_gains(otm, {direct_gains, diffuse_gains});

  VectorXd direct_vs_gains(panner.num_virtual_loudspeakers());
  VectorXd diffuse_vs_gains(panner.num_virtual_loudspeakers());

  panner.get_vs_gains({direct_gains, diffuse_gains}, {direct_vs_gains, diffuse_vs_gains});

  auto direct_delays = panner.get_direct_delays({direct_gains, diffuse_gains});
  REQUIRE(direct_delays.left >= 0.0);
  REQUIRE(direct_delays.right >= 0.0);
  REQUIRE(std::abs(direct_delays.left - direct_delays.right) < 1e-3);

  VectorXd left_diffuse_delays(panner.num_virtual_loudspeakers());
  VectorXd right_diffuse_delays(panner.num_virtual_loudspeakers());

  panner.get_diffuse_delays({left_diffuse_delays, right_diffuse_delays});
}

TEST_CASE("gain_norm")
{
  bear::Panner panner(DEFAULT_TENSORFILE_NAME);

  ear::ObjectsTypeMetadata otm;
  for (int diffuse_i = 0; diffuse_i <= 2; diffuse_i++) {
    otm.diffuse = diffuse_i / 2;
    for (int az = 0; az <= 360; az += 5) {
      otm.position = ear::PolarPosition{(double)az, 0.0, 1.0};

      VectorXd direct_gains(panner.num_gains());
      VectorXd diffuse_gains(panner.num_gains());
      panner.calc_objects_gains(otm, {direct_gains, diffuse_gains});

      auto direct_delays = panner.get_direct_delays({direct_gains, diffuse_gains});

      double comp_gain = panner.compensation_gain({direct_gains, diffuse_gains}, direct_delays, {0});

      REQUIRE(comp_gain > 0.5);
      REQUIRE(comp_gain < 2.0);
    }
  }
}
