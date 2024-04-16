#include "static_delay_calc.hpp"

namespace bear {
StaticDelayCalc::StaticDelayCalc(const SignalFlowContext &ctx,
                                 const char *name,
                                 CompositeComponent *parent,
                                 const ConfigImpl &,
                                 std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      brir_index_in("brir_index_in", *this, pml::EmptyParameterConfig()),
      static_delays_out(
          "static_delays_out", *this, pml::VectorParameterConfig(2 * panner->num_virtual_loudspeakers())),
      temp_left(panner->num_virtual_loudspeakers()),
      temp_right(panner->num_virtual_loudspeakers())
{
}

void StaticDelayCalc::process()
{
  if (brir_index_in.changed()) {
    panner->get_diffuse_delays({temp_left, temp_right}, {brir_index_in.data().value()});
    for (size_t i = 0; i < panner->num_virtual_loudspeakers(); i++) {
      static_delays_out.data().at(i * 2) = temp_left(i);
      static_delays_out.data().at(i * 2 + 1) = temp_right(i);
    }
    static_delays_out.swapBuffers();

    brir_index_in.resetChanged();
  }
}
}  // namespace bear
