#include "gain_norm.hpp"

namespace bear {
GainNorm::GainNorm(const SignalFlowContext &ctx,
                   const char *name,
                   CompositeComponent *parent,
                   const ConfigImpl &config,
                   std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      num_objects(config.num_objects_channels),
      gains_in("gains_in",
               *this,
               pml::MatrixParameterConfig(panner->num_gains() * 2, config.num_objects_channels)),
      brir_index_in("brir_index_in", *this, pml::EmptyParameterConfig()),
      direct_delays_in(
          "direct_delays_in", *this, pml::VectorParameterConfig(2 * config.num_objects_channels)),
      gains_out("gains_out",
                *this,
                pml::MatrixParameterConfig(panner->num_gains() * 2, config.num_objects_channels)),
      temp_direct(panner->num_gains()),
      temp_diffuse(panner->num_gains())
{
}

void GainNorm::process()
{
  unsigned int brir_index = brir_index_in.data().value();

  for (size_t object_i = 0; object_i < num_objects; object_i++) {
    for (size_t gain_i = 0; gain_i < panner->num_gains(); gain_i++) {
      temp_direct(gain_i) = gains_in.data()(gain_i, object_i);
      temp_diffuse(gain_i) = gains_in.data()(panner->num_gains() + gain_i, object_i);
    }

    LeftRight<double> delays{direct_delays_in.data().at(object_i * 2),
                             direct_delays_in.data().at(object_i * 2 + 1)};

    double comp = panner->compensation_gain({temp_direct, temp_diffuse}, delays, {brir_index});

    for (size_t i = 0; i < panner->num_gains() * 2; i++)
      gains_out.data().at(i, object_i) = comp * gains_in.data().at(i, object_i);
  }
}

}  // namespace bear
