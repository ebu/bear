#pragma once
#include <libpml/double_buffering_protocol.hpp>
#include <libpml/matrix_parameter.hpp>
#include <libpml/scalar_parameter.hpp>
#include <libpml/shared_data_protocol.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>

#include "config_impl.hpp"
#include "panner.hpp"

namespace bear {
using namespace visr;
using namespace visr::pml;

class DirectSpeakersGainNorm : public AtomicComponent {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit DirectSpeakersGainNorm(const SignalFlowContext &ctx,
                                  const char *name,
                                  CompositeComponent *parent,
                                  const ConfigImpl &config,
                                  std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;
  size_t num_objects;

  ParameterInput<pml::SharedDataProtocol, MatrixParameter<float>> gains_in;
  ParameterInput<pml::DoubleBufferingProtocol, pml::ScalarParameter<unsigned int>> brir_index_in;
  ParameterOutput<pml::SharedDataProtocol, MatrixParameter<float>> gains_out;

  Eigen::VectorXd temp_gains;
};

}  // namespace bear
