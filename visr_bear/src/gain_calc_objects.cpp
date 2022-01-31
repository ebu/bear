#include "gain_calc_objects.hpp"

#include <libvisr/time.hpp>

#include "listener_adaptation.hpp"

namespace bear {

GainCalcObjects::GainCalcObjects(const SignalFlowContext &ctx,
                                 const char *name,
                                 CompositeComponent *parent,
                                 const ConfigImpl &config,
                                 std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      sample_rate(config.sample_rate),
      num_objects(config.num_objects_channels),
      metadata_in("metadata_in", *this, pml::EmptyParameterConfig()),
      gains_out("gains_out",
                *this,
                pml::MatrixParameterConfig(2 * panner->num_gains(), config.num_objects_channels)),
      listener_in("listener_in", *this, pml::EmptyParameterConfig()),
      temp_direct(panner->num_gains()),
      temp_diffuse(panner->num_gains()),
      temp_direct_a(panner->num_gains()),
      temp_diffuse_a(panner->num_gains()),
      temp_direct_b(panner->num_gains()),
      temp_diffuse_b(panner->num_gains()),
      per_object_data(config.num_objects_channels, (size_t)panner->num_gains())
{
}

size_t GainCalcObjects::to_sample(const Time &t)
{
  return boost::rational_cast<size_t>(t * (int64_t)sample_rate);
}

GainCalcObjects::Point::Point(size_t n_gains) : direct_cache(n_gains), diffuse_cache(n_gains) {}

void GainCalcObjects::Point::calc_gains(GainCalcObjects &parent, DirectDiffuse<Ref<VectorXd>> gains)
{
  if (!cache_valid) {
    adapt_otm(otm, adapted_otm, listener);
    parent.panner->calc_objects_gains(adapted_otm.type_metadata, {direct_cache, diffuse_cache});
    cache_valid = true;
  }
  gains.direct = direct_cache;
  gains.diffuse = diffuse_cache;
}

void GainCalcObjects::Point::set_otm(const ObjectsInput &new_otm)
{
  otm = new_otm;
  cache_valid = false;
}

void GainCalcObjects::Point::set_listener(const ListenerImpl &new_listener)
{
  if (!listeners_approx_equal(listener, new_listener)) {
    listener = new_listener;
    cache_valid = false;
  }
}

GainCalcObjects::PerObject::PerObject(size_t n_gains) : a(n_gains), b(n_gains) {}

void GainCalcObjects::PerObject::update(const ObjectsInput &block)
{
  if (block.rtime && block.duration) {
    Time default_interpolation_length = (first_block || last_block_end != *block.rtime) ? 0 : *block.duration;
    Time interpolation_length = block.interpolationLength
                                    ? std::min(*block.interpolationLength, default_interpolation_length)
                                    : default_interpolation_length;

    // will only look at a if there is some interpolation length
    if (interpolation_length) a = b;
    b.set_otm(block);
    a.time = *block.rtime;
    b.time = *block.rtime + interpolation_length;

    last_block_end = *block.rtime + *block.duration;
    first_block = false;
    infinite_block = false;
  } else {
    // if infinite_block is set, we only care about the otm in b
    b.set_otm(block);
    first_block = true;
    infinite_block = true;
  }
}

void GainCalcObjects::PerObject::set_listener(const ListenerImpl &listener)
{
  a.set_listener(listener);
  b.set_listener(listener);
}

void GainCalcObjects::PerObject::calc_gains(GainCalcObjects &parent,
                                            Time t,
                                            DirectDiffuse<Ref<VectorXd>> gains)
{
  if (infinite_block) {
    b.calc_gains(parent, gains);
  } else if (t > last_block_end) {
    gains.direct.setZero();
    gains.diffuse.setZero();
  } else if (t >= b.time) {
    b.calc_gains(parent, gains);
  } else if (t >= a.time) {
    a.calc_gains(parent, {parent.temp_direct_a, parent.temp_diffuse_a});
    b.calc_gains(parent, {parent.temp_direct_b, parent.temp_diffuse_b});

    // t_b - t_a == 0 is handled by previous case
    double p = boost::rational_cast<double>((t - a.time) / (b.time - a.time));

    gains.direct = parent.temp_direct_a * (1 - p) + parent.temp_direct_b * p;
    gains.diffuse = parent.temp_diffuse_a * (1 - p) + parent.temp_diffuse_b * p;
  } else
    throw std::logic_error("block shouldn't have been processed yet");
}

void GainCalcObjects::process()
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
    per_object_data.at(i).calc_gains(*this, block_end, {temp_direct, temp_diffuse});
    for (size_t j = 0; j < panner->num_gains(); j++) {
      gains_out.data()(j, i) = temp_direct(j);
      gains_out.data()(panner->num_gains() + j, i) = temp_diffuse(j);
    }
  }
}

}  // namespace bear
