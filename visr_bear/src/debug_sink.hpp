#pragma once
#include <Eigen/Core>
#include <libvisr/atomic_component.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/signal_flow_context.hpp>
#include <map>
#include <memory>

namespace bear {
using namespace visr;

class DebugStorage {
 public:
  static DebugStorage &get_instance()
  {
    static DebugStorage debug_storage;
    return debug_storage;
  }

  Eigen::MatrixXf &get_write(std::string name, size_t num_channels, size_t block_size)
  {
    auto it = blocks.emplace(name, Eigen::MatrixXf{num_channels, block_size}).first;
    return it->second;
  }

  Eigen::MatrixXf &get(std::string name) { return blocks.at(name); }

  DebugStorage(DebugStorage const &) = delete;
  void operator=(DebugStorage const &) = delete;

 private:
  DebugStorage() {}

  std::map<std::string, Eigen::MatrixXf> blocks;
};

class DebugSink : public AtomicComponent {
 public:
  explicit DebugSink(const SignalFlowContext &ctx,
                     const char *name,
                     CompositeComponent *parent,
                     size_t n_channels)
      : AtomicComponent(ctx, name, parent),
        in("in", *this, n_channels),
        block(DebugStorage::get_instance().get_write(name, n_channels, ctx.period()))
  {
  }

  void process() override
  {
    for (size_t i = 0; i < in.width(); i++)
      for (size_t j = 0; j < period(); j++) block(i, j) = in.at(i)[j];
  }

 private:
  AudioInput in;
  Eigen::MatrixXf &block;
};

}  // namespace bear
