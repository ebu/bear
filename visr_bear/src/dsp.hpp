#pragma once
#include <libpml/matrix_parameter.hpp>
#include <librcl/add.hpp>
#include <librcl/delay_vector.hpp>
#include <librcl/fir_filter_matrix.hpp>
#include <librcl/gain_matrix.hpp>
#include <librcl/interpolating_fir_filter_matrix.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/composite_component.hpp>
#include <memory>

#include "bear/api.hpp"
#include "brir_interpolation_controller.hpp"
#include "panner.hpp"
#include "per_ear_delay.hpp"
#include "utils.hpp"

namespace bear {
using namespace visr;

class DSP : public CompositeComponent {
 public:
  explicit DSP(const SignalFlowContext &ctx,
               const char *name,
               CompositeComponent *parent,
               const ConfigImpl &config,
               std::shared_ptr<Panner> panner);

 private:
  rbbl::InterpolationParameterSet initial_brir_interpolants(const ConfigImpl &config);
  rbbl::FilterRoutingList initial_brir_routings(const ConfigImpl &config);
  efl::BasicMatrix<float> initial_brir_filters(const ConfigImpl &config);

  std::shared_ptr<Panner> panner;

  Indexer<2> object_ear_index;
  Indexer<2> convolver_index;
  Indexer<3> brir_index;

  AudioInput objects_in;
  AudioInput direct_speakers_in;
  AudioInput hoa_in;
  AudioOutput out;

  PerEarDelay objects_direct_path;
  ParameterInput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> direct_delays_in;
  ParameterInput<pml::SharedDataProtocol, pml::MatrixParameter<float>> direct_gains_in;

  rcl::GainMatrix diffuse_gains;
  ParameterInput<pml::SharedDataProtocol, pml::MatrixParameter<float>> diffuse_gains_in;

  PerEarDelay direct_speakers_path;
  ParameterInput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> direct_speakers_delays_in;
  ParameterInput<pml::SharedDataProtocol, pml::MatrixParameter<float>> direct_speakers_gains_in;

  rcl::Add add_brir_inputs;

  rcl::FirFilterMatrix decorrelators;
  rcl::InterpolatingFirFilterMatrix brirs;
  BRIRInterpolationController brir_interpolation_controller;
  ParameterInput<DoubleBufferingProtocol, ScalarParameter<unsigned int>> brir_index_in;

  ParameterInput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> static_delays_in;
  rcl::DelayVector static_delays;

  ParameterInput<pml::SharedDataProtocol, pml::MatrixParameter<SampleType>> hoa_gains_in;
  rcl::GainMatrix hoa_matrix;
  rcl::FirFilterMatrix hoa_irs;
  rcl::DelayVector hoa_delays;
  rcl::Add add_hoa;
};
}  // namespace bear
