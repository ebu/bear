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

class GainCalcObjects : public AtomicComponent {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit GainCalcObjects(const SignalFlowContext &ctx,
                           const char *name,
                           CompositeComponent *parent,
                           const ConfigImpl &config,
                           std::shared_ptr<Panner> panner);

  void process() override;

 private:
  std::shared_ptr<Panner> panner;
  size_t sample_rate;
  size_t num_objects;

  ParameterInput<pml::MessageQueueProtocol, ADMParameter<ObjectsInput>> metadata_in;
  ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<SampleType>> gains_out;
  ParameterInput<pml::DoubleBufferingProtocol, ListenerParameter> listener_in;

  size_t to_sample(const Time &t);

  Eigen::VectorXd temp_direct;
  Eigen::VectorXd temp_diffuse;
  Eigen::VectorXd temp_direct_a;
  Eigen::VectorXd temp_diffuse_a;
  Eigen::VectorXd temp_direct_b;
  Eigen::VectorXd temp_diffuse_b;

  /// Point in time with associated type metadata and listener data; this
  /// caches gains, which are invalidated when the listener or type-metadata
  /// are changed. If the listener is not updated, this means that during long
  /// blocks the gains are not re-calculated.
  class Point {
   public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Point(size_t n_gains);

    void calc_gains(GainCalcObjects &parent, DirectDiffuse<Ref<VectorXd>> gains);
    void set_otm(const ObjectsInput &otm);
    void set_listener(const ListenerImpl &listener);

    Time time;

   private:
    ObjectsInput otm;
    ObjectsInput adapted_otm;
    ListenerImpl listener;

    Eigen::VectorXd direct_cache;
    Eigen::VectorXd diffuse_cache;
    bool cache_valid = false;
  };

  /// per-object data; new metadata is pushed in through update, and gains can
  /// be calculated at a given time.
  class PerObject {
   public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    PerObject(size_t n_gains);
    void update(const ObjectsInput &block);
    void set_listener(const ListenerImpl &listener);
    void calc_gains(GainCalcObjects &parent, Time t, DirectDiffuse<Ref<VectorXd>> gains);

   private:
    // current metadata: interpolation from a to b (both containing a time and
    // parameters), then constant from b to last_block_end, unless
    // infinite_block, in which case b is used forever.
    Point a, b;
    Time last_block_end = {0, 1};
    bool infinite_block = false;

    /// should the next block have "first block" behaviour (no interpolation) -- is it the real
    /// first block, or was the last block infinite?
    bool first_block = true;
  };

  std::vector<PerObject, Eigen::aligned_allocator<PerObject>> per_object_data;
};

}  // namespace bear
