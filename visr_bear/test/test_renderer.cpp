#include "bear/api.hpp"
#include "catch2/catch.hpp"
#include "test_config.h"

using namespace bear;

void test_period(size_t period)
{
  Config config;
  config.set_num_direct_speakers_channels(0);
  config.set_num_objects_channels(1);
  config.set_num_hoa_channels(0);
  config.set_period_size(period);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);
  Renderer renderer(config);

  std::vector<float> input(config.get_period_size(), 1.0);
  std::vector<float> output_l(config.get_period_size());
  std::vector<float> output_r(config.get_period_size());

  bear::ObjectsInput oi;
  oi.type_metadata.position = ear::PolarPosition{0.0, 0.0, 1.0};
  renderer.add_objects_block(0, oi);

  float *input_p[1] = {input.data()};
  float *output_p[2] = {output_l.data(), output_r.data()};
  renderer.process(input_p, nullptr, nullptr, output_p);
}

TEST_CASE("basic")
{
  test_period(128);
  test_period(256);
  test_period(512);
  test_period(1024);
  test_period(2048);
}

TEST_CASE("test_directspeakers")
{
  Config config;
  config.set_num_direct_speakers_channels(1);
  config.set_period_size(512);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);
  Renderer renderer(config);

  bear::DirectSpeakersInput block;
  block.type_metadata.position = ear::PolarSpeakerPosition{0.0, 0.0, 1.0};
  renderer.add_direct_speakers_block(0, block);

  std::vector<float> input(config.get_period_size(), 1.0);
  std::vector<float> output_l(config.get_period_size());
  std::vector<float> output_r(config.get_period_size());

  float *input_p[1] = {input.data()};
  float *output_p[2] = {output_l.data(), output_r.data()};
  renderer.process(nullptr, input_p, nullptr, output_p);
}

TEST_CASE("test_hoa")
{
  Config config;
  config.set_num_hoa_channels(1);
  config.set_period_size(512);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);
  Renderer renderer(config);

  bear::HOAInput block;
  block.type_metadata.orders = {0};
  block.type_metadata.degrees = {0};
  block.type_metadata.normalization = "SN3D";
  block.channels = {0};
  renderer.add_hoa_block(0, block);

  std::vector<float> input(config.get_period_size(), 1.0);
  std::vector<float> output_l(config.get_period_size());
  std::vector<float> output_r(config.get_period_size());

  float *input_p[1] = {input.data()};
  float *output_p[2] = {output_l.data(), output_r.data()};
  renderer.process(nullptr, nullptr, input_p, output_p);
}

TEST_CASE("fft_error")
{
  using namespace Catch::Matchers;

  Config config;
  config.set_num_objects_channels(1);
  config.set_period_size(512);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);
  config.set_fft_implementation("unavailable_fft");
  REQUIRE_THROWS_WITH(Renderer(config), Contains("The specified FFT wrapper does not exist"));
}
