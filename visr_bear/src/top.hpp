#pragma once
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/composite_component.hpp>
#include <memory>

#include "bear/api.hpp"
#include "control.hpp"
#include "dsp.hpp"
#include "panner.hpp"
#include "utils.hpp"

namespace bear {
using namespace visr;

class Top : public CompositeComponent {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  explicit Top(const SignalFlowContext &ctx,
               const char *name,
               CompositeComponent *parent,
               const ConfigImpl &config);

 private:
  std::shared_ptr<Panner> panner;
  DSP dsp;
  Control control;

  AudioInput in;
  AudioOutput out;
  ParameterInput<pml::MessageQueueProtocol, ADMParameter<ObjectsInput>> objects_metadata_in;
  ParameterInput<pml::MessageQueueProtocol, ADMParameter<DirectSpeakersInput>> direct_speakers_metadata_in;
  ParameterInput<pml::MessageQueueProtocol, ADMParameter<HOAInput>> hoa_metadata_in;
  ParameterInput<pml::DoubleBufferingProtocol, ListenerParameter> listener_in;
};
}  // namespace bear
