#pragma once
#include <libpml/matrix_parameter.hpp>
#include <librcl/add.hpp>
#include <librcl/delay_vector.hpp>
#include <librcl/gain_matrix.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/composite_component.hpp>
#include <memory>

#include "bear/api.hpp"
#include "utils.hpp"

namespace bear {
using namespace visr;

class PerEarDelay : public CompositeComponent {
 public:
  explicit PerEarDelay(const SignalFlowContext &ctx,
                       const char *name,
                       CompositeComponent *parent,
                       std::size_t num_inputs,
                       std::size_t num_outputs,
                       double initial_delay);

 private:
  Indexer<2> object_ear_index;
  Indexer<2> convolver_index;

  AudioInput in;
  AudioOutput out;

  rcl::DelayVector delays;
  ParameterInput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> delays_in;
  rcl::GainMatrix gains_l;
  rcl::GainMatrix gains_r;
  ParameterInput<pml::SharedDataProtocol, pml::MatrixParameter<float>> gains_in;
};
}  // namespace bear
