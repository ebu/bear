#pragma once
#include <libpml/matrix_parameter.hpp>
#include <libpml/shared_data_protocol.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>

#include "bear/api.hpp"
#include "dsp.hpp"
#include "panner.hpp"
#include "parameters.hpp"
#include "utils.hpp"

namespace bear {
using namespace visr;

class GainCalcHOA : public AtomicComponent {
 public:
  explicit GainCalcHOA(const SignalFlowContext &ctx,
                       const char *name,
                       CompositeComponent *parent,
                       const ConfigImpl &config,
                       std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;
  size_t sample_rate;

  ParameterInput<pml::MessageQueueProtocol, ADMParameter<HOAInput>> metadata_in;
  ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<SampleType>> gains_out;
  ParameterInput<pml::DoubleBufferingProtocol, ListenerParameter> listener_in;

  size_t to_sample(const Time &t);
  void update_for(std::size_t stream, const HOAInput &input);

  struct PerChannel {
    PerChannel(size_t n_hoa_channels);

    size_t stream = 0;
    Time last_block_end{0, 1};
    bool infinite_block = false;
    Eigen::VectorXd to_hoa;
    bool changed = false;
  };

  std::vector<PerChannel> per_channel_data;
  Eigen::MatrixXd sh_rotation_matrix;
  size_t num_hoa_channels;
  size_t order;
};

}  // namespace bear
