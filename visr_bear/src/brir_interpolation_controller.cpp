#include "brir_interpolation_controller.hpp"

namespace bear {
BRIRInterpolationController::BRIRInterpolationController(const SignalFlowContext &ctx,
                                                         const char *name,
                                                         CompositeComponent *parent,
                                                         const ConfigImpl &config,
                                                         std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      convolver_index(panner->num_virtual_loudspeakers(), 2u),
      brir_index(panner->num_views(), panner->num_virtual_loudspeakers(), 2u),
      brir_index_in("brir_index_in", *this, pml::EmptyParameterConfig()),
      interpolants_out("interpolants_out", *this, InterpolationParameterConfig(1))
{
}

void BRIRInterpolationController::process()
{
  if (brir_index_in.changed()) {
    unsigned int idx = brir_index_in.data().value();
    for (size_t vs = 0; vs < panner->num_virtual_loudspeakers(); vs++)
      for (size_t ear = 0; ear < 2; ear++)
        interpolants_out.enqueue(
            pml::InterpolationParameter(convolver_index(vs, ear), {brir_index(idx, vs, ear)}, {1.0f}));
    brir_index_in.resetChanged();
  }
}
}  // namespace bear
