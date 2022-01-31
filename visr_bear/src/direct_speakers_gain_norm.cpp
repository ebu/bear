#include "direct_speakers_gain_norm.hpp"

namespace bear {
DirectSpeakersGainNorm::DirectSpeakersGainNorm(const SignalFlowContext &ctx,
                                               const char *name,
                                               CompositeComponent *parent,
                                               const ConfigImpl &config,
                                               std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      num_objects(config.num_direct_speakers_channels),
      gains_in("gains_in", *this, pml::MatrixParameterConfig(panner->num_gains(), num_objects)),
      brir_index_in("brir_index_in", *this, pml::EmptyParameterConfig()),
      gains_out("gains_out", *this, pml::MatrixParameterConfig(panner->num_gains(), num_objects)),
      temp_gains(panner->num_gains())
{
}

void DirectSpeakersGainNorm::process()
{
  unsigned int brir_index = brir_index_in.data().value();

  for (size_t object_i = 0; object_i < num_objects; object_i++) {
    for (size_t gain_i = 0; gain_i < panner->num_gains(); gain_i++) {
      temp_gains(gain_i) = gains_in.data()(gain_i, object_i);
    }

    double comp = panner->compensation_gain_direct(temp_gains, {brir_index});

    for (size_t i = 0; i < panner->num_gains(); i++)
      gains_out.data().at(i, object_i) = comp * gains_in.data().at(i, object_i);
  }
}

}  // namespace bear
