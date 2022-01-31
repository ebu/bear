#include "direct_speakers_delay_calc.hpp"

#include "config_impl.hpp"

namespace bear {
DirectSpeakersDelayCalc::DirectSpeakersDelayCalc(const SignalFlowContext &ctx,
                                                 const char *name,
                                                 CompositeComponent *parent,
                                                 const ConfigImpl &config,
                                                 std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      num_objects(config.num_direct_speakers_channels),
      gains_in("gains_in",
               *this,
               pml::MatrixParameterConfig(panner->num_gains(), config.num_direct_speakers_channels)),
      brir_index_in("brir_index_in", *this, pml::EmptyParameterConfig()),
      direct_delays_out(
          "direct_delays_out", *this, pml::VectorParameterConfig(2 * config.num_direct_speakers_channels)),
      temp_gains(panner->num_gains())
{
}

void DirectSpeakersDelayCalc::process()
{
  unsigned int brir_index = brir_index_in.data().value();

  for (size_t object_i = 0; object_i < num_objects; object_i++) {
    for (size_t gain_i = 0; gain_i < panner->num_gains(); gain_i++) {
      temp_gains(gain_i) = gains_in.data()(gain_i, object_i);
    }
    auto delays = panner->get_direct_speakers_delays(temp_gains, {brir_index});
    // TODO: should use index helper in the panner, or perhaps created by utils?
    direct_delays_out.data().at(object_i * 2) = delays.left;
    direct_delays_out.data().at(object_i * 2 + 1) = delays.right;
  }

  direct_delays_out.swapBuffers(/* copyValue = */ true);
  brir_index_in.resetChanged();
}
}  // namespace bear
