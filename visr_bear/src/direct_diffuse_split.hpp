#pragma once
#include <libpml/matrix_parameter.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>

#include "bear/api.hpp"
#include "dsp.hpp"
#include "panner.hpp"
#include "parameters.hpp"

namespace bear {
using namespace visr;

class DirectDiffuseSplit : public AtomicComponent {
 public:
  explicit DirectDiffuseSplit(const SignalFlowContext &ctx,
                              const char *name,
                              CompositeComponent *parent,
                              const ConfigImpl &config,
                              std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;
  size_t num_objects;

  ParameterInput<pml::SharedDataProtocol, MatrixParameter<float>> gains_in;
  ParameterOutput<pml::SharedDataProtocol, MatrixParameter<float>> direct_gains_out;
  ParameterOutput<pml::SharedDataProtocol, MatrixParameter<float>> diffuse_gains_out;
};

}  // namespace bear
