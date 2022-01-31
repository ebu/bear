#pragma once
#include <Eigen/Core>
#include <libpml/double_buffering_protocol.hpp>
#include <libpml/matrix_parameter.hpp>
#include <libpml/message_queue_protocol.hpp>
#include <libpml/shared_data_protocol.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>
#include <vector>

#include "bear/api.hpp"
#include "panner.hpp"
#include "parameters.hpp"
#include "utils.hpp"

namespace bear {
using namespace visr;

class DirectSpeakersGainCalc : public AtomicComponent {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit DirectSpeakersGainCalc(const SignalFlowContext &ctx,
                                  const char *name,
                                  CompositeComponent *parent,
                                  const ConfigImpl &config,
                                  std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;
  size_t sample_rate;
  size_t num_objects;

  ParameterInput<pml::MessageQueueProtocol, ADMParameter<DirectSpeakersInput>> metadata_in;
  ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<SampleType>> gains_out;
  ParameterInput<pml::DoubleBufferingProtocol, ListenerParameter> listener_in;

  size_t to_sample(const Time &t);

  Eigen::VectorXd temp_gains;

  /// per-object data; new metadata is pushed in through update, and gains can
  /// be calculated at a given time.
  class PerObject {
   public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    PerObject(size_t n_gains);
    void update(const DirectSpeakersInput &block);
    void set_listener(const ListenerImpl &listener);
    void calc_gains(DirectSpeakersGainCalc &parent, Time t, Ref<VectorXd> gains);

   private:
    DirectSpeakersInput dstm;
    DirectSpeakersInput adapted_dstm;
    Time last_block_end = {0, 1};
    bool infinite_block = false;

    ListenerImpl listener;

    Eigen::VectorXd gains_cache;
    bool cache_valid = false;
  };

  std::vector<PerObject, Eigen::aligned_allocator<PerObject>> per_object_data;
};

}  // namespace bear
