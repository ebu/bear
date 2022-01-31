#pragma once
#include <libpml/double_buffering_protocol.hpp>
#include <libpml/interpolation_parameter.hpp>
#include <libpml/message_queue_protocol.hpp>
#include <libpml/scalar_parameter.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>

#include "bear/api.hpp"
#include "panner.hpp"
#include "utils.hpp"

namespace bear {
using namespace visr;
using namespace visr::pml;

class BRIRInterpolationController : public AtomicComponent {
 public:
  explicit BRIRInterpolationController(const SignalFlowContext &ctx,
                                       const char *name,
                                       CompositeComponent *parent,
                                       const ConfigImpl &config,
                                       std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;
  Indexer<2> convolver_index;
  Indexer<3> brir_index;
  ParameterInput<DoubleBufferingProtocol, ScalarParameter<unsigned int>> brir_index_in;
  ParameterOutput<MessageQueueProtocol, InterpolationParameter> interpolants_out;
};
}  // namespace bear
