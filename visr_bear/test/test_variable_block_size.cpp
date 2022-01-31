#include "bear/api.hpp"
#include "bear/variable_block_size.hpp"
#include "catch2/catch.hpp"
#include "test_config.h"

using namespace bear;

std::array<std::vector<float>, 2> get_output_basic(const Config &config)
{
  auto renderer = std::make_shared<Renderer>(config);

  std::vector<float> input(config.get_period_size(), 1.0);
  std::vector<float> output_l(config.get_period_size());
  std::vector<float> output_r(config.get_period_size());

  bear::ObjectsInput oi;
  oi.type_metadata.position = ear::PolarPosition{0.0, 0.0, 1.0};
  renderer->add_objects_block(0, oi);

  float *input_p[1] = {input.data()};
  float *output_p[2] = {output_l.data(), output_r.data()};
  renderer->process(input_p, nullptr, nullptr, output_p);

  return {output_l, output_r};
}

std::array<std::vector<float>, 2> get_output_variable(const Config &config)
{
  auto renderer = std::make_shared<Renderer>(config);
  VariableBlockSizeAdapter vbs(config, renderer);

  std::vector<float> input(config.get_period_size() * 2, 1.0);
  std::vector<float> output_l(config.get_period_size() * 2);
  std::vector<float> output_r(config.get_period_size() * 2);

  bear::ObjectsInput oi;
  oi.type_metadata.position = ear::PolarPosition{0.0, 0.0, 1.0};
  renderer->add_objects_block(0, oi);

  float *input_p[1] = {input.data()};
  float *output_p[2] = {output_l.data(), output_r.data()};
  vbs.process(config.get_period_size() * 2, input_p, nullptr, nullptr, output_p);
  return {output_l, output_r};
}

TEST_CASE("basic")
{
  Config config;
  config.set_num_objects_channels(1);
  config.set_period_size(512);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);

  auto res_normal = get_output_basic(config);
  auto res_vbs = get_output_variable(config);

  // VBS introduces a delay of 1 block, so the first block from the VBS version
  // should be silence, and the next should be the first block of the normal
  // output
  for (size_t channel = 0; channel < 2; channel++)
    for (size_t sample = 0; sample < config.get_period_size(); sample++) {
      REQUIRE(res_vbs[channel][sample] == Approx(0.0));
      REQUIRE(res_vbs[channel][config.get_period_size() + sample] == Approx(res_normal[channel][sample]));
    }
}

TEST_CASE("vbs_queue")
{
  const int block_len = 512;
  std::queue<bear::ObjectsInput> input_queue;
  for (size_t i = 0; i < 100; i++) {
    bear::ObjectsInput oi;
    oi.type_metadata.position = ear::PolarPosition{0.0, 0.0, 1.0};
    oi.rtime = Time{i * block_len, 48000};
    oi.duration = Time{block_len, 48000};
    input_queue.push(std::move(oi));
  }

  Config config;
  config.set_num_objects_channels(1);
  config.set_period_size(512);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);

  auto renderer = std::make_shared<Renderer>(config);
  VariableBlockSizeAdapter vbs(config, renderer);

  size_t ext_period_size = 1024;

  std::vector<float> input(ext_period_size, 1.0);
  std::vector<float> output_l(ext_period_size);
  std::vector<float> output_r(ext_period_size);
  float *input_p[1] = {input.data()};
  float *output_p[2] = {output_l.data(), output_r.data()};

  double out = 0.0;
  for (int i = 0; i < 100; i++) {
    int n_pushed = 0;
    while (input_queue.size())
      if (vbs.add_objects_block(ext_period_size, 0, input_queue.front())) {
        input_queue.pop();
        n_pushed++;
      } else
        break;
    REQUIRE((n_pushed == 2 || n_pushed == 0));

    vbs.process(ext_period_size, input_p, nullptr, nullptr, output_p);
    out = std::max({out, (double)fabs(output_l[0]), (double)fabs(output_r[0])});
  }
  REQUIRE(!input_queue.size());
  REQUIRE(out >= 0.01);
}
