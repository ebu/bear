#include "per_ear_delay.hpp"

namespace bear {
PerEarDelay::PerEarDelay(const SignalFlowContext &ctx,
                         const char *name,
                         CompositeComponent *parent,
                         size_t num_inputs,
                         size_t num_outputs,
                         double initial_delay)
    : CompositeComponent(ctx, name, parent),
      object_ear_index(num_inputs, 2u),
      convolver_index(num_outputs, 2u),
      in("in", *this, num_inputs),
      out("out", *this, 2 * num_outputs),

      delays(ctx, "delays", this),
      delays_in("delays_in", *this, pml::VectorParameterConfig(2 * num_inputs)),
      gains_l(ctx, "gains_l", this),
      gains_r(ctx, "gains_r", this),
      gains_in("gains_in", *this, pml::MatrixParameterConfig(num_outputs, num_inputs))
{
  delays.setup(
      /* numberOfChannels = */ 2 * num_inputs,
      /* interpolationSteps = */ period(),
      /* maximumDelaySeconds = */ 1.0,  // TODO: reduce this to match maximum required delay
      /* interpolationMethod = */ "lagrangeOrder3",
      /* methodDelayPolicy = */ rcl::DelayVector::MethodDelayPolicy::Add,
      /* controlInputs = */ rcl::DelayVector::ControlPortConfig::Delay,
      /* initialDelaySeconds = */ initial_delay);

  std::array<rcl::GainMatrix *, 2> gains{&gains_l, &gains_r};
  for (auto gains_i : gains)
    gains_i->setup(
        /* numberOfInputs = */ num_inputs,
        /* numberOfOutputs = */ num_outputs,
        /* interpolationSteps = */ period());

  // input -> delays
  for (size_t ear = 0; ear < 2; ear++)
    for (size_t object = 0; object < num_inputs; object++)
      audioConnection(in, {object}, delays.audioPort("in"), {object_ear_index(object, ear)});

  // delays -> gains
  for (size_t ear = 0; ear < 2; ear++)
    for (size_t object = 0; object < num_inputs; object++)
      audioConnection(
          delays.audioPort("out"), {object_ear_index(object, ear)}, gains[ear]->audioPort("in"), {object});

  // gains -> output
  for (size_t ear = 0; ear < 2; ear++)
    for (size_t vs = 0; vs < num_outputs; vs++)
      audioConnection(gains[ear]->audioPort("out"), {vs}, out, {convolver_index(vs, ear)});

  parameterConnection(gains_in, gains_l.parameterPort("gainInput"));
  parameterConnection(gains_in, gains_r.parameterPort("gainInput"));
  parameterConnection(delays_in, delays.parameterPort("delayInput"));
}
}  // namespace bear
