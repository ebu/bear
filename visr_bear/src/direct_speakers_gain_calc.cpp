#include "direct_speakers_gain_calc.hpp"

#include <libvisr/time.hpp>

#include "listener_adaptation.hpp"

namespace bear {

DirectSpeakersGainCalc::DirectSpeakersGainCalc(const SignalFlowContext &ctx,
                                               const char *name,
                                               CompositeComponent *parent,
                                               const ConfigImpl &config,
                                               std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      sample_rate(config.sample_rate),
      num_objects(config.num_direct_speakers_channels),
      metadata_in("metadata_in", *this, pml::EmptyParameterConfig()),
      gains_out("gains_out", *this, pml::MatrixParameterConfig(panner->num_gains(), num_objects)),
      listener_in("listener_in", *this, pml::EmptyParameterConfig()),
      temp_gains(panner->num_gains()),
      per_object_data(num_objects, (size_t)panner->num_gains())
{
}

size_t DirectSpeakersGainCalc::to_sample(const Time &t)
{
  return boost::rational_cast<size_t>(t * (int64_t)sample_rate);
}

DirectSpeakersGainCalc::PerObject::PerObject(size_t n_gains) : gains_cache(n_gains) {}

void DirectSpeakersGainCalc::PerObject::set_listener(const ListenerImpl &new_listener)
{
  if (!listeners_approx_equal(listener, new_listener)) {
    listener = new_listener;
    cache_valid = false;
  }
}

void DirectSpeakersGainCalc::PerObject::update(const DirectSpeakersInput &block)
{
  dstm = block;
  cache_valid = false;

  if (block.rtime && block.duration) {
    last_block_end = *block.rtime + *block.duration;
    infinite_block = false;
  } else {
    infinite_block = true;
  }
}

void DirectSpeakersGainCalc::PerObject::calc_gains(DirectSpeakersGainCalc &parent,
                                                   Time t,
                                                   Ref<VectorXd> gains)
{
  if (infinite_block || t <= last_block_end) {
    if (!cache_valid) {
      adapt_dstm(dstm, adapted_dstm, listener);
      parent.panner->calc_direct_speakers_gains(adapted_dstm.type_metadata, gains_cache);
      cache_valid = true;
    }
    gains = gains_cache;
  } else {
    gains.setZero();
  }
}

void DirectSpeakersGainCalc::process()
{
  if (listener_in.changed()) {
    for (size_t i = 0; i < num_objects; i++) per_object_data.at(i).set_listener(listener_in.data());
    listener_in.resetChanged();
  }

  while (!metadata_in.empty()) {
    const auto &param = metadata_in.front();
    per_object_data.at(param.index).update(param.value);

    metadata_in.pop();
  }

  Time block_end{time().sampleCount() + period(), sample_rate};

  for (size_t i = 0; i < num_objects; i++) {
    per_object_data.at(i).calc_gains(*this, block_end, temp_gains);
    for (size_t j = 0; j < panner->num_gains(); j++) {
      gains_out.data()(j, i) = temp_gains(j);
    }
  }
}

}  // namespace bear
