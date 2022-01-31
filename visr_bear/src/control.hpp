#pragma once
#include <libpml/matrix_parameter.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>
#include <memory>

#include "bear/api.hpp"
#include "direct_delay_calc.hpp"
#include "direct_diffuse_split.hpp"
#include "direct_speakers_delay_calc.hpp"
#include "direct_speakers_gain_calc.hpp"
#include "direct_speakers_gain_norm.hpp"
#include "gain_calc_hoa.hpp"
#include "gain_calc_objects.hpp"
#include "gain_norm.hpp"
#include "panner.hpp"
#include "select_brir.hpp"
#include "static_delay_calc.hpp"
#include "utils.hpp"

namespace bear {
using namespace visr;

class Control : public CompositeComponent {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit Control(const SignalFlowContext &ctx,
                   const char *name,
                   CompositeComponent *parent,
                   const ConfigImpl &config,
                   std::shared_ptr<Panner> panner);

 private:
  std::shared_ptr<Panner> panner;
  GainCalcObjects gain_calc;
  DirectDiffuseSplit direct_diffuse_split;
  DirectDelayCalc direct_delay_calc;
  StaticDelayCalc static_delay_calc;
  SelectBRIR select_brir;
  std::unique_ptr<GainNorm> gain_norm;

  DirectSpeakersGainCalc direct_speakers_gain_calc;
  DirectSpeakersDelayCalc direct_speakers_delay_calc;
  std::unique_ptr<DirectSpeakersGainNorm> direct_speakers_gain_norm;

  GainCalcHOA gain_calc_hoa;

  ParameterInput<pml::MessageQueueProtocol, ADMParameter<ObjectsInput>> metadata_in;
  ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<float>> direct_gains_out;
  ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<float>> diffuse_gains_out;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> direct_delays_out;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> static_delays_out;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::ScalarParameter<unsigned int>> brir_index_out;

  ParameterInput<pml::MessageQueueProtocol, ADMParameter<DirectSpeakersInput>> direct_speakers_metadata_in;
  ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<float>> direct_speakers_gains_out;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::VectorParameter<float>> direct_speakers_delays_out;

  ParameterInput<pml::MessageQueueProtocol, ADMParameter<HOAInput>> hoa_metadata_in;
  ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<float>> hoa_gains_out;

  ParameterInput<pml::DoubleBufferingProtocol, ListenerParameter> listener_in;
};
}  // namespace bear
