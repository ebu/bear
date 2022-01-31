#include <chrono>

#include "catch2/catch.hpp"
#include "constructor_thread.hpp"
#include "dynamic_renderer.hpp"
#include "test_config.h"

using namespace bear;
using namespace std::chrono_literals;

TEST_CASE("test_ConstructorThread")
{
  ConstructorThread t;

  Config config;
  config.set_num_direct_speakers_channels(1);
  config.set_period_size(512);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);

  for (int i = 0; i < 3; i++) {
    while (!t.start(config)) std::this_thread::sleep_for(10ms);

    boost::optional<Renderer> result;
    while (!(result = t.get_result())) std::this_thread::sleep_for(10ms);

    Renderer renderer;
    renderer = std::move(result.get());

    bear::DirectSpeakersInput block;
    block.type_metadata.position = ear::PolarSpeakerPosition{0.0, 0.0, 1.0};
    renderer.add_direct_speakers_block(0, block);
  }
}

TEST_CASE("test_ConstructorThread_swap")
{
  ConstructorThread t;

  Config config;
  config.set_num_direct_speakers_channels(1);
  config.set_period_size(512);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);

  boost::optional<Renderer> renderer;

  for (int i = 0; i < 3; i++) {
    while (!t.start(config)) std::this_thread::sleep_for(10ms);

    while (!t.try_swap_result(renderer)) std::this_thread::sleep_for(10ms);

    bear::DirectSpeakersInput block;
    block.type_metadata.position = ear::PolarSpeakerPosition{0.0, 0.0, 1.0};
    renderer->add_direct_speakers_block(0, block);
  }
}

TEST_CASE("test_DynamicRenderer_basic")
{
  // the real tests for this are in test_dynamic_renderer.py; this is just here
  // to make valgrinding it easier
  const size_t block_size = 512;
  DynamicRenderer r(block_size, 100);

  Config config;
  config.set_num_objects_channels(1);
  config.set_period_size(block_size);
  config.set_data_path(DEFAULT_TENSORFILE_NAME);

  r.set_config(config);

  bear::ObjectsInput block;
  block.type_metadata.position = ear::PolarPosition{0.0, 0.0, 1.0};
  bool pushed = r.add_objects_block(0, block);

  std::vector<float> input(block_size, 1.0);
  std::vector<float> output_l(block_size);
  std::vector<float> output_r(block_size);

  float *input_p[1] = {input.data()};
  float *output_p[2] = {output_l.data(), output_r.data()};

  int blocks_to_process = 5;
  while (blocks_to_process > 0) {
    if (!pushed) pushed = r.add_objects_block(0, block);
    r.process(1, input_p, 0, nullptr, 0, nullptr, output_p);

    printf("%f\n", std::abs(output_l[0]));

    for (float sample : output_l) {
      if (std::abs(sample) > 1e-5) {
        blocks_to_process--;
        break;
      }
    }
    std::this_thread::sleep_for(100ms);
  }
}

TEST_CASE("test_DynamicRenderer_error")
{
  const size_t block_size = 512;
  DynamicRenderer r(block_size, 100);

  Config config;
  config.set_num_objects_channels(1);
  config.set_period_size(block_size);
  config.set_data_path("does_not_exist.tf");
  r.set_config(config);

  bool found_error = false;
  for (size_t i = 0; i < 100 && !found_error; i++) {
    std::this_thread::sleep_for(100ms);

    std::exception_ptr e = r.get_error();
    if (e) found_error = true;
  }
  REQUIRE(found_error);
}
