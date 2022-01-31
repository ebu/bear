#include "control.hpp"

namespace bear {
using namespace visr;
Control::Control(const SignalFlowContext &ctx,
                 const char *name,
                 CompositeComponent *parent,
                 const ConfigImpl &config,
                 std::shared_ptr<Panner> panner_)
    : CompositeComponent(ctx, name, parent),
      panner(std::move(panner_)),
      gain_calc(ctx, "gain_calc", this, config, panner),
      direct_diffuse_split(ctx, "direct_diffuse_split", this, config, panner),
      direct_delay_calc(ctx, "direct_delay_calc", this, config, panner),
      static_delay_calc(ctx, "static_delay_calc", this, config, panner),
      select_brir(ctx, "select_brir", this, config, panner),
      gain_norm(panner->has_gain_compensation()
                    ? std::make_unique<GainNorm>(ctx, "gain_norm", this, config, panner)
                    : std::unique_ptr<GainNorm>()),
      direct_speakers_gain_calc(ctx, "direct_speakers_gain_calc", this, config, panner),
      direct_speakers_delay_calc(ctx, "direct_speakers_delay_calc", this, config, panner),
      direct_speakers_gain_norm(panner->has_gain_compensation()
                                    ? std::make_unique<DirectSpeakersGainNorm>(
                                          ctx, "direct_speakers_gain_norm", this, config, panner)
                                    : std::unique_ptr<DirectSpeakersGainNorm>()),
      gain_calc_hoa(ctx, "gain_calc_hoa", this, config, panner),
      metadata_in("metadata_in", *this, pml::EmptyParameterConfig()),
      direct_gains_out("direct_gains_out",
                       *this,
                       pml::MatrixParameterConfig(panner->num_gains(), config.num_objects_channels)),
      diffuse_gains_out("diffuse_gains_out",
                        *this,
                        pml::MatrixParameterConfig(panner->num_gains(), config.num_objects_channels)),
      direct_delays_out(
          "direct_delays_out", *this, pml::VectorParameterConfig(2 * config.num_objects_channels)),
      static_delays_out(
          "static_delays_out", *this, pml::VectorParameterConfig(2 * panner->num_virtual_loudspeakers())),
      brir_index_out("brir_index_out", *this, pml::EmptyParameterConfig()),
      direct_speakers_metadata_in("direct_speakers_metadata_in", *this, pml::EmptyParameterConfig()),
      direct_speakers_gains_out("direct_speakers_gains_out",
                                *this,
                                pml::MatrixParameterConfig(panner->num_virtual_loudspeakers(),
                                                           config.num_direct_speakers_channels)),
      direct_speakers_delays_out("direct_speakers_delays_out",
                                 *this,
                                 pml::VectorParameterConfig(2 * config.num_direct_speakers_channels)),
      hoa_metadata_in("hoa_metadata_in", *this, pml::EmptyParameterConfig()),
      hoa_gains_out("hoa_gains_out",
                    *this,
                    pml::MatrixParameterConfig(panner->n_hoa_channels(), config.num_hoa_channels)),
      listener_in("listener_in", *this, pml::EmptyParameterConfig())
{
  // objects
  parameterConnection(metadata_in, gain_calc.parameterPort("metadata_in"));

  // connect gain_calc to {direct,diffuse}_gains_out, possibly via gain_norm
  if (panner->has_gain_compensation()) {
    parameterConnection(direct_delay_calc.parameterPort("direct_delays_out"),
                        gain_norm->parameterPort("direct_delays_in"));
    parameterConnection(select_brir.parameterPort("brir_index_out"),
                        gain_norm->parameterPort("brir_index_in"));

    parameterConnection(gain_calc.parameterPort("gains_out"), gain_norm->parameterPort("gains_in"));
    parameterConnection(gain_norm->parameterPort("gains_out"),
                        direct_diffuse_split.parameterPort("gains_in"));
  } else
    parameterConnection(gain_calc.parameterPort("gains_out"), direct_diffuse_split.parameterPort("gains_in"));
  parameterConnection(direct_diffuse_split.parameterPort("direct_gains_out"), direct_gains_out);
  parameterConnection(direct_diffuse_split.parameterPort("diffuse_gains_out"), diffuse_gains_out);

  parameterConnection(gain_calc.parameterPort("gains_out"), direct_delay_calc.parameterPort("gains_in"));
  parameterConnection(direct_delay_calc.parameterPort("direct_delays_out"), direct_delays_out);

  // shared
  parameterConnection(static_delay_calc.parameterPort("static_delays_out"), static_delays_out);

  // direct speakers
  parameterConnection(direct_speakers_metadata_in, direct_speakers_gain_calc.parameterPort("metadata_in"));
  if (panner->has_gain_compensation()) {
    parameterConnection(select_brir.parameterPort("brir_index_out"),
                        direct_speakers_gain_norm->parameterPort("brir_index_in"));

    parameterConnection(direct_speakers_gain_calc.parameterPort("gains_out"),
                        direct_speakers_gain_norm->parameterPort("gains_in"));
    parameterConnection(direct_speakers_gain_norm->parameterPort("gains_out"), direct_speakers_gains_out);
  } else
    parameterConnection(direct_speakers_gain_calc.parameterPort("gains_out"), direct_speakers_gains_out);
  parameterConnection(direct_speakers_gain_calc.parameterPort("gains_out"),
                      direct_speakers_delay_calc.parameterPort("gains_in"));
  parameterConnection(direct_speakers_delay_calc.parameterPort("direct_delays_out"),
                      direct_speakers_delays_out);

  // hoa
  parameterConnection(hoa_metadata_in, gain_calc_hoa.parameterPort("metadata_in"));
  parameterConnection(gain_calc_hoa.parameterPort("gains_out"), hoa_gains_out);

  // listener stuff
  parameterConnection(listener_in, select_brir.parameterPort("listener_in"));
  parameterConnection(listener_in, gain_calc_hoa.parameterPort("listener_in"));

  parameterConnection(select_brir.parameterPort("listener_out"), gain_calc.parameterPort("listener_in"));
  parameterConnection(select_brir.parameterPort("listener_out"),
                      direct_speakers_gain_calc.parameterPort("listener_in"));

  parameterConnection(select_brir.parameterPort("brir_index_out"),
                      direct_delay_calc.parameterPort("brir_index_in"));
  parameterConnection(select_brir.parameterPort("brir_index_out"),
                      static_delay_calc.parameterPort("brir_index_in"));
  parameterConnection(select_brir.parameterPort("brir_index_out"),
                      direct_speakers_delay_calc.parameterPort("brir_index_in"));
  parameterConnection(select_brir.parameterPort("brir_index_out"), brir_index_out);
}
}  // namespace bear
