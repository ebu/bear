#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "bear/api.hpp"
#include "ear_bits/hoa.hpp"
#include "test_config.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"

#if defined(__unix__) || defined(__unix)
#define RT_SCHEDULING_POSIX
#endif

#ifdef RT_SCHEDULING_POSIX
#include <pthread.h>
#include <sched.h>
#endif

using namespace bear;

/// set high-priority RT scheduling; restore after
class SetSchedRT {
 public:
  SetSchedRT()
  {
#ifdef RT_SCHEDULING_POSIX
    pthread_getschedparam(pthread_self(), &old_policy, &old_param);

    sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param)) {
      std::cerr << "pthread_setschedparam failed: " << std::strerror(errno) << '\n';
    }
#else
    std::cerr << "don't know how to set RT scheduling\n";
#endif
  }

  ~SetSchedRT()
  {
#ifdef RT_SCHEDULING_POSIX
    if (pthread_setschedparam(pthread_self(), old_policy, &old_param)) {
      std::cerr << "pthread_setschedparam failed: " << std::strerror(errno) << '\n';
    }
#endif
  }

 private:
#ifdef RT_SCHEDULING_POSIX
  sched_param old_param;
  int old_policy;
#endif
};

std::vector<std::vector<float>> make_buffers(size_t period, size_t n)
{
  std::vector<std::vector<float>> buffers;
  for (size_t i = 0; i < n; i++) {
    std::vector<float> buffer(period, 1.0);
    buffers.push_back(std::move(buffer));
  }

  return buffers;
}

std::vector<float *> make_ptrs(std::vector<std::vector<float>> &buffers)
{
  std::vector<float *> ptrs;
  for (auto &buffer : buffers) ptrs.push_back(buffer.data());
  return ptrs;
}

void fill_noise(std::vector<std::vector<float>> &buffers)
{
  std::mt19937 gen{};
  std::normal_distribution<float> d{0.0, 0.1};

  for (auto &buffer : buffers)
    for (float &sample : buffer) sample = d(gen);
}

struct BenchmarkConfig {
  Config config;
  size_t n_blocks;
  bool update_every_time;
  bool head_track;
  bool extent;
  size_t hoa_channels_per_block;
};

std::vector<double> run_benchmark(const BenchmarkConfig &c)
{
  Renderer renderer(c.config);

  // make I/O buffers

  auto objects_buffers = make_buffers(c.config.get_period_size(), c.config.get_num_objects_channels());
  fill_noise(objects_buffers);
  auto objects_ptrs = make_ptrs(objects_buffers);

  auto direct_speakers_buffers =
      make_buffers(c.config.get_period_size(), c.config.get_num_direct_speakers_channels());
  fill_noise(direct_speakers_buffers);
  auto direct_speakers_ptrs = make_ptrs(direct_speakers_buffers);

  auto hoa_buffers = make_buffers(c.config.get_period_size(), c.config.get_num_hoa_channels());
  fill_noise(hoa_buffers);
  auto hoa_ptrs = make_ptrs(hoa_buffers);

  auto output_buffers = make_buffers(c.config.get_period_size(), 2);
  auto output_ptrs = make_ptrs(output_buffers);

  // make objects metadata (and push if we're only using one block)

  std::mt19937 gen{};
  std::uniform_real_distribution<double> az_dist{-180.0, 180.0};

  std::mt19937 ext_gen{};
  std::uniform_real_distribution<double> ext_dist{0, 360};

  std::vector<std::vector<bear::ObjectsInput>> objects_inputs(c.config.get_num_objects_channels());

  auto make_oi_base = [&]() {
    bear::ObjectsInput oi;
    oi.type_metadata.position = ear::PolarPosition{az_dist(gen), 0.0, 1.0};
    if (c.extent) {
      oi.type_metadata.diffuse = 0.5;
      oi.type_metadata.width = ext_dist(ext_gen);
      oi.type_metadata.height = ext_dist(ext_gen);
    }
    return oi;
  };

  for (size_t obj = 0; obj < c.config.get_num_objects_channels(); obj++) {
    if (c.update_every_time) {
      for (size_t i = 0; i < c.n_blocks; i++) {
        bear::ObjectsInput oi = make_oi_base();
        oi.rtime = Time{i * c.config.get_period_size(), c.config.get_sample_rate()};
        oi.duration = Time{c.config.get_period_size(), c.config.get_sample_rate()};
        objects_inputs[obj].push_back(std::move(oi));
      }
    } else {
      bear::ObjectsInput oi = make_oi_base();
      renderer.add_objects_block(obj, oi);
    }
  }

  // push DS metadata

  for (size_t i = 0; i < c.config.get_num_direct_speakers_channels(); i++) {
    bear::DirectSpeakersInput block;
    block.type_metadata.position = ear::PolarSpeakerPosition{az_dist(gen), 0.0, 1.0};
    renderer.add_direct_speakers_block(i, block);
  }

  // push HOA metadata

  for (size_t i = 0; i < c.config.get_num_hoa_channels() / c.hoa_channels_per_block; i++) {
    bear::HOAInput block;
    block.type_metadata.normalization = "SN3D";
    for (size_t j = 0; j < c.hoa_channels_per_block; j++) {
      block.channels.push_back(i * c.hoa_channels_per_block + j);
      int n, m;
      std::tie(n, m) = bear::ear_bits::hoa::from_acn(j);
      block.type_metadata.orders.push_back(n);
      block.type_metadata.degrees.push_back(m);
    }
    renderer.add_hoa_block(i, block);
  }

  std::vector<double> times(c.n_blocks);

  {
    SetSchedRT sched_rt;

    for (size_t i = 0; i < c.n_blocks; i++) {
      auto start = std::chrono::steady_clock::now();

      if (c.update_every_time)
        for (size_t obj = 0; obj < c.config.get_num_objects_channels(); obj++)
          renderer.add_objects_block(obj, objects_inputs[obj][i]);

      renderer.process(objects_ptrs.data(), direct_speakers_ptrs.data(), hoa_ptrs.data(), output_ptrs.data());

      auto end = std::chrono::steady_clock::now();
      std::chrono::duration<double> diff = end - start;
      times[i] = diff.count();

      // give other processes a chance
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  return times;
}

void run_benchmark_print(const BenchmarkConfig &c, size_t run)
{
  std::vector<double> times = run_benchmark(c);

  rapidjson::Document d(rapidjson::kObjectType);

  rapidjson::Value config_json(rapidjson::kObjectType);
  config_json.AddMember("update_every_time", c.update_every_time, d.GetAllocator());
  config_json.AddMember("head_track", c.head_track, d.GetAllocator());
  config_json.AddMember("extent", c.extent, d.GetAllocator());
  config_json.AddMember("hoa_channels_per_block", c.hoa_channels_per_block, d.GetAllocator());

  config_json.AddMember("sample_rate", c.config.get_sample_rate(), d.GetAllocator());
  config_json.AddMember(
      "data_path", rapidjson::Value(c.config.get_data_path(), d.GetAllocator()), d.GetAllocator());
  config_json.AddMember("period_size", c.config.get_period_size(), d.GetAllocator());
  config_json.AddMember("fft_implementation",
                        rapidjson::Value(c.config.get_fft_implementation(), d.GetAllocator()),
                        d.GetAllocator());
  config_json.AddMember("num_objects_channels", c.config.get_num_objects_channels(), d.GetAllocator());
  config_json.AddMember(
      "num_direct_speakers_channels", c.config.get_num_direct_speakers_channels(), d.GetAllocator());
  config_json.AddMember("num_hoa_channels", c.config.get_num_hoa_channels(), d.GetAllocator());

  d.AddMember("config", std::move(config_json), d.GetAllocator());

  d.AddMember("run", run, d.GetAllocator());

  rapidjson::Value times_json(rapidjson::kArrayType);
  for (double time : times) times_json.PushBack(time, d.GetAllocator());

  d.AddMember("times", std::move(times_json), d.GetAllocator());

  rapidjson::OStreamWrapper osw(std::cout);
  rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
  d.Accept(writer);
  std::cout << "\n";
}

int main(int argc, char **argv)
{
  for (size_t run = 0; run < 5; run++) {
    for (size_t period : {128, 512, 2048}) {
      auto run_bench = [&](size_t num_objects_channels,
                           size_t num_direct_speakers_channels,
                           size_t num_hoa_channels,
                           bool update_every_time = false,
                           bool extent = false,
                           const std::string &fft_implementation = "ffts") {
        BenchmarkConfig c;
        c.config.set_num_objects_channels(num_objects_channels);
        c.config.set_num_direct_speakers_channels(num_direct_speakers_channels);
        c.config.set_num_hoa_channels(num_hoa_channels);
        c.config.set_period_size(period);
        c.config.set_data_path(DEFAULT_TENSORFILE_NAME);
        c.config.set_fft_implementation(fft_implementation);

        c.n_blocks = 15;
        c.update_every_time = update_every_time;
        c.extent = extent;
        c.head_track = false;
        c.hoa_channels_per_block = 4;

        run_benchmark_print(c, run);
      };

      // non-default FFTs
      for (const std::string &fft_implementation : {"kissfft"})
        for (int num_objects_p = -1; num_objects_p <= 6; num_objects_p++)
          run_bench(num_objects_p >= 0 ? 1 << num_objects_p : 0, 0, 0, false, false, fft_implementation);

      for (bool update_every_time : {false, true})
        for (bool extent : {false, true})
          for (int num_objects_p = -1; num_objects_p <= 6; num_objects_p++)
            run_bench(num_objects_p >= 0 ? 1 << num_objects_p : 0, 0, 0, update_every_time, extent);

      for (int num_direct_speakers_p = 0; num_direct_speakers_p <= 6; num_direct_speakers_p++)
        run_bench(0, num_direct_speakers_p >= 0 ? 1 << num_direct_speakers_p : 0, 0);

      for (int num_hoa_p = 0; num_hoa_p <= 4; num_hoa_p++)
        run_bench(0, 0, num_hoa_p >= 0 ? 4 * (1 << num_hoa_p) : 0);
    }
  }
}
