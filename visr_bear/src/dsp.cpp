#include "dsp.hpp"

#include <libvisr/signal_flow_context.hpp>

namespace bear {

DSP::DSP(const SignalFlowContext &ctx,
         const char *name,
         CompositeComponent *parent,
         const ConfigImpl &config,
         std::shared_ptr<Panner> panner_)
    : CompositeComponent(ctx, name, parent),
      panner(std::move(panner_)),

      object_ear_index(config.num_objects_channels, 2u),
      convolver_index(panner->num_virtual_loudspeakers(), 2u),
      brir_index(panner->num_views(), panner->num_virtual_loudspeakers(), 2u),

      objects_in("objects_in", *this, config.num_objects_channels),
      direct_speakers_in("direct_speakers_in", *this, config.num_direct_speakers_channels),
      hoa_in("hoa_in", *this, config.num_hoa_channels),
      out("out", *this, 2),

      objects_direct_path(ctx,
                          "objects_direct_path",
                          this,
                          config.num_objects_channels,
                          panner->num_virtual_loudspeakers(),
                          panner->get_default_direct_delay()),
      direct_delays_in(
          "direct_delays_in", *this, pml::VectorParameterConfig(2 * config.num_objects_channels)),
      direct_gains_in("direct_gains_in",
                      *this,
                      pml::MatrixParameterConfig(panner->num_gains(), config.num_objects_channels)),

      diffuse_gains(ctx, "diffuse_gains", this),
      diffuse_gains_in("diffuse_gains_in",
                       *this,
                       pml::MatrixParameterConfig(panner->num_gains(), config.num_objects_channels)),

      direct_speakers_path(ctx,
                           "direct_speakers_path",
                           this,
                           config.num_direct_speakers_channels,
                           panner->num_virtual_loudspeakers(),
                           panner->get_default_direct_delay()),
      direct_speakers_delays_in("direct_speakers_delays_in",
                                *this,
                                pml::VectorParameterConfig(2 * config.num_direct_speakers_channels)),
      direct_speakers_gains_in(
          "direct_speakers_gains_in",
          *this,
          pml::MatrixParameterConfig(panner->num_gains(), config.num_direct_speakers_channels)),

      add_brir_inputs(ctx,
                      "add_brir_inputs",
                      this,
                      /* width = */ 2 * panner->num_virtual_loudspeakers(),
                      /* numInputs = */ 3),

      decorrelators(ctx,
                    "decorrelators",
                    this,
                    /* numberOfInputs = */ panner->num_virtual_loudspeakers(),
                    /* numberOfOutputs = */ panner->num_virtual_loudspeakers(),
                    /* filterLength = */ panner->decorrelator_length(),
                    /* maxFilters = */ panner->num_virtual_loudspeakers(),
                    /* maxRoutings = */ panner->num_virtual_loudspeakers(),
                    /* filters = */ efl::BasicMatrix<SampleType>(),
                    /* routings = */ rbbl::FilterRoutingList(),
                    /* controlInputs = */ rcl::FirFilterMatrix::ControlPortConfig::None,
                    /* fftImplementation = */ config.fft_implementation.c_str()),
      brirs(ctx,
            "brirs",
            this,
            /* numberOfInputs = */ 2 * panner->num_virtual_loudspeakers(),
            /* numberOfOutputs = */ 2,
            /* filterLength = */ panner->brir_length(),
            /* maxFilters = */ panner->num_views() * 2 * panner->num_virtual_loudspeakers(),
            /* maxRoutings = */ 2 * panner->num_virtual_loudspeakers(),
            /* numberOfInterpolants = */ 1,
            /* transitionSamples = */ ctx.period(),
            /* filters = */ initial_brir_filters(),
            /* initialInterpolants = */ initial_brir_interpolants(),
            /* routings = */ initial_brir_routings(),
            /* controlInputs = */ rcl::InterpolatingFirFilterMatrix::ControlPortConfig::Interpolants,
            /* fftImplementation = */ config.fft_implementation.c_str()),
      brir_interpolation_controller(ctx, "brir_interpolation_controller", this, config, panner),
      brir_index_in("brir_index_in", *this, pml::EmptyParameterConfig()),

      static_delays_in(
          "static_delays_in", *this, pml::VectorParameterConfig(2 * panner->num_virtual_loudspeakers())),
      static_delays(ctx, "static_delays", this),
      hoa_gains_in("hoa_gains_in",
                   *this,
                   pml::MatrixParameterConfig(panner->n_hoa_channels(), config.num_hoa_channels)),
      hoa_matrix(ctx, "hoa_matrix", this),
      hoa_irs(ctx,
              "hoa_irs",
              this,
              /* numberOfInputs = */ panner->n_hoa_channels(),
              /* numberOfOutputs = */ 2,
              /* filterLength = */ panner->hoa_ir_length(),
              /* maxFilters = */ panner->n_hoa_channels() * 2,
              /* maxRoutings = */ panner->n_hoa_channels() * 2,
              /* filters = */ efl::BasicMatrix<SampleType>(),
              /* routings = */ rbbl::FilterRoutingList(),
              /* controlInputs = */ rcl::FirFilterMatrix::ControlPortConfig::None,
              /* fftImplementation = */ config.fft_implementation.c_str()),
      hoa_delays(ctx, "hoa_delays", this),
      add_hoa(ctx,
              "add_hoa",
              this,
              /* width = */ 2,
              /* numInputs = */ 2)

{
  // objects direct path

  audioConnection(objects_in, objects_direct_path.audioPort("in"));
  audioConnection(objects_direct_path.audioPort("out"), add_brir_inputs.audioPort("in0"));
  parameterConnection(direct_gains_in, objects_direct_path.parameterPort("gains_in"));
  parameterConnection(direct_delays_in, objects_direct_path.parameterPort("delays_in"));

  // diffuse path

  diffuse_gains.setup(/* numberOfInputs = */ config.num_objects_channels,
                      /* numberOfOutputs = */ panner->num_virtual_loudspeakers(),
                      /* interpolationSteps = */ period());

  // setup decorrelators
  for (size_t vs = 0; vs < panner->num_virtual_loudspeakers(); vs++) {
    decorrelators.setFilter(vs, panner->get_decorrelator(vs), panner->decorrelator_length());
    decorrelators.addRouting(vs, vs, vs);
  }

  static_delays.setup(
      /* numberOfChannels = */ 2 * panner->num_virtual_loudspeakers(),
      /* interpolationSteps = */ period(),
      /* maximumDelaySeconds = */ 1.0,  // TODO: reduce this
      /* interpolationMethod = */ "lagrangeOrder3",
      /* methodDelayPolicy = */ rcl::DelayVector::MethodDelayPolicy::Add,
      /* controlInputs = */ rcl::DelayVector::ControlPortConfig::Delay,
      /* initialDelaySeconds = */ panner->get_default_static_delay());

  audioConnection(objects_in, diffuse_gains.audioPort("in"));
  parameterConnection(diffuse_gains_in, diffuse_gains.parameterPort("gainInput"));
  audioConnection(diffuse_gains.audioPort("out"), decorrelators.audioPort("in"));

  // decorrelators -> static delays
  for (size_t ear = 0; ear < 2; ear++)
    for (size_t vs = 0; vs < panner->num_virtual_loudspeakers(); vs++)
      audioConnection(
          decorrelators.audioPort("out"), {vs}, static_delays.audioPort("in"), {convolver_index(vs, ear)});

  parameterConnection(static_delays_in, static_delays.parameterPort("delayInput"));

  audioConnection(static_delays.audioPort("out"), add_brir_inputs.audioPort("in1"));

  // direct speakers

  audioConnection(direct_speakers_in, direct_speakers_path.audioPort("in"));
  audioConnection(direct_speakers_path.audioPort("out"), add_brir_inputs.audioPort("in2"));
  parameterConnection(direct_speakers_gains_in, direct_speakers_path.parameterPort("gains_in"));
  parameterConnection(direct_speakers_delays_in, direct_speakers_path.parameterPort("delays_in"));

  // BRIRs

  audioConnection(add_brir_inputs.audioPort("out"), brirs.audioPort("in"));
  audioConnection(brirs.audioPort("out"), add_hoa.audioPort("in0"));

  parameterConnection(brir_index_in, brir_interpolation_controller.parameterPort("brir_index_in"));
  parameterConnection(brir_interpolation_controller.parameterPort("interpolants_out"),
                      brirs.parameterPort("interpolantInput"));

  // hoa

  hoa_matrix.setup(/* numberOfInputs = */ config.num_hoa_channels,
                   /* numberOfOutputs = */ panner->n_hoa_channels(),
                   /* interpolationSteps = */ period());
  // hoa irs
  for (size_t hoa_channel = 0; hoa_channel < panner->n_hoa_channels(); hoa_channel++) {
    for (size_t ear = 0; ear < 2; ear++) {
      size_t filter_idx = hoa_channel * 2 + ear;
      hoa_irs.setFilter(filter_idx, panner->get_hoa_ir(hoa_channel, ear), panner->hoa_ir_length());
      hoa_irs.addRouting(hoa_channel, ear, filter_idx);
    }
  }

  hoa_delays.setup(
      /* numberOfChannels = */ 2,
      /* interpolationSteps = */ period(),
      /* maximumDelaySeconds = */ 1.0,  // TODO: reduce this
      /* interpolationMethod = */ "lagrangeOrder3",
      /* methodDelayPolicy = */ rcl::DelayVector::MethodDelayPolicy::Add,
      /* controlInputs = */ rcl::DelayVector::ControlPortConfig::None,
      /* initialDelaySeconds = */ panner->hoa_delay());

  parameterConnection(hoa_gains_in, hoa_matrix.parameterPort("gainInput"));
  audioConnection(hoa_in, hoa_matrix.audioPort("in"));
  audioConnection(hoa_matrix.audioPort("out"), hoa_irs.audioPort("in"));
  audioConnection(hoa_irs.audioPort("out"), hoa_delays.audioPort("in"));
  audioConnection(hoa_delays.audioPort("out"), add_hoa.audioPort("in1"));

  audioConnection(add_hoa.audioPort("out"), out);
}

rbbl::InterpolationParameterSet DSP::initial_brir_interpolants()
{
  rbbl::InterpolationParameterSet ips;
  for (size_t vs = 0; vs < panner->num_virtual_loudspeakers(); vs++)
    for (size_t ear = 0; ear < 2; ear++)
      ips.insert(rbbl::InterpolationParameter(convolver_index(vs, ear), {brir_index(0u, vs, ear)}, {1.0f}));
  return ips;
}

rbbl::FilterRoutingList DSP::initial_brir_routings()
{
  rbbl::FilterRoutingList routings;
  for (size_t vs = 0; vs < panner->num_virtual_loudspeakers(); vs++)
    for (size_t ear = 0; ear < 2; ear++)
      routings.addRouting(convolver_index(vs, ear), ear, convolver_index(vs, ear), 1.0);
  return routings;
}

efl::BasicMatrix<float> DSP::initial_brir_filters()
{
  efl::BasicMatrix<float> filters(panner->num_views() * 2 * panner->num_virtual_loudspeakers(),
                                  panner->brir_length());
  for (size_t view = 0; view < panner->num_views(); view++)
    for (size_t vs = 0; vs < panner->num_virtual_loudspeakers(); vs++)
      for (size_t ear = 0; ear < 2; ear++)
        filters.setRow(brir_index(view, vs, ear), panner->get_brir(view, vs, ear));

  return filters;
}

}  // namespace bear
