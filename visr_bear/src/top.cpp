#include "top.hpp"

#include <iostream>
#include <libvisr/signal_flow_context.hpp>

namespace bear {

Top::Top(const SignalFlowContext &ctx, const char *name, CompositeComponent *parent, const ConfigImpl &config)
    : CompositeComponent(ctx, name, parent),
      panner(std::make_shared<Panner>(config.data_path)),
      dsp(ctx, "dsp", this, config, panner),
      control(ctx, "control", this, config, panner),
      in("in", *this, num_input_channels(config)),
      out("out", *this, 2),
      objects_metadata_in("objects_metadata_in", *this, pml::EmptyParameterConfig()),
      direct_speakers_metadata_in("direct_speakers_metadata_in", *this, pml::EmptyParameterConfig()),
      hoa_metadata_in("hoa_metadata_in", *this, pml::EmptyParameterConfig()),
      listener_in("listener_in", *this, pml::EmptyParameterConfig())
{
  ChannelRange objects_range(0, config.num_objects_channels);
  audioConnection(in, objects_range, dsp.audioPort("objects_in"), objects_range);

  ChannelRange direct_speakers_range(objects_range.end(),
                                     objects_range.end() + config.num_direct_speakers_channels);
  ChannelRange just_direct_speakers_range(0, config.num_direct_speakers_channels);
  audioConnection(in, direct_speakers_range, dsp.audioPort("direct_speakers_in"), just_direct_speakers_range);

  ChannelRange hoa_range(direct_speakers_range.end(), direct_speakers_range.end() + config.num_hoa_channels);
  ChannelRange just_hoa_range(0, config.num_hoa_channels);
  audioConnection(in, hoa_range, dsp.audioPort("hoa_in"), just_hoa_range);

  audioConnection(dsp.audioPort("out"), out);

  parameterConnection(control.parameterPort("direct_gains_out"), dsp.parameterPort("direct_gains_in"));
  parameterConnection(control.parameterPort("diffuse_gains_out"), dsp.parameterPort("diffuse_gains_in"));
  parameterConnection(control.parameterPort("direct_delays_out"), dsp.parameterPort("direct_delays_in"));
  parameterConnection(control.parameterPort("static_delays_out"), dsp.parameterPort("static_delays_in"));
  parameterConnection(control.parameterPort("brir_index_out"), dsp.parameterPort("brir_index_in"));
  parameterConnection(control.parameterPort("hoa_gains_out"), dsp.parameterPort("hoa_gains_in"));

  parameterConnection(objects_metadata_in, control.parameterPort("metadata_in"));

  parameterConnection(direct_speakers_metadata_in, control.parameterPort("direct_speakers_metadata_in"));
  parameterConnection(control.parameterPort("direct_speakers_gains_out"),
                      dsp.parameterPort("direct_speakers_gains_in"));
  parameterConnection(control.parameterPort("direct_speakers_delays_out"),
                      dsp.parameterPort("direct_speakers_delays_in"));

  parameterConnection(hoa_metadata_in, control.parameterPort("hoa_metadata_in"));

  parameterConnection(listener_in, control.parameterPort("listener_in"));
}

}  // namespace bear
