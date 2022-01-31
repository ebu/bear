#pragma once
#include <libpml/double_buffering_protocol.hpp>
#include <libpml/scalar_parameter.hpp>
#include <libpml/vector_parameter.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>

#include "bear/api.hpp"
#include "panner.hpp"

namespace bear {
using namespace visr;

class StaticDelayCalc : public AtomicComponent {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit StaticDelayCalc(const SignalFlowContext &ctx,
                           const char *name,
                           CompositeComponent *parent,
                           const ConfigImpl &config,
                           std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;

  ParameterInput<pml::DoubleBufferingProtocol, pml::ScalarParameter<unsigned int>> brir_index_in;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> static_delays_out;

  Eigen::VectorXd temp_left;
  Eigen::VectorXd temp_right;
};

}  // namespace bear
