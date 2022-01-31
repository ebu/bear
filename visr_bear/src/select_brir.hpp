#pragma once
#include <libpml/double_buffering_protocol.hpp>
#include <libpml/scalar_parameter.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>
#include <vector>

#include "bear/api.hpp"
#include "panner.hpp"
#include "parameters.hpp"

namespace bear {
using namespace visr;

class SelectBRIR : public AtomicComponent {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit SelectBRIR(const SignalFlowContext &ctx,
                      const char *name,
                      CompositeComponent *parent,
                      const ConfigImpl &config,
                      std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;
  Eigen::MatrixX3d views;
  std::vector<Eigen::Quaterniond> view_rotations;

  ParameterInput<pml::DoubleBufferingProtocol, ListenerParameter> listener_in;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::ScalarParameter<unsigned int>> brir_index_out;
  ParameterOutput<pml::DoubleBufferingProtocol, ListenerParameter> listener_out;
};

}  // namespace bear
