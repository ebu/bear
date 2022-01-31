#include "direct_diffuse_split.hpp"

namespace bear {
DirectDiffuseSplit::DirectDiffuseSplit(const SignalFlowContext &ctx,
                                       const char *name,
                                       CompositeComponent *parent,
                                       const ConfigImpl &config,
                                       std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      num_objects(config.num_objects_channels),
      gains_in("gains_in",
               *this,
               pml::MatrixParameterConfig(panner->num_gains() * 2, config.num_objects_channels)),
      direct_gains_out("direct_gains_out",
                       *this,
                       pml::MatrixParameterConfig(panner->num_gains(), config.num_objects_channels)),
      diffuse_gains_out("diffuse_gains_out",
                        *this,
                        pml::MatrixParameterConfig(panner->num_gains(), config.num_objects_channels))
{
}

void DirectDiffuseSplit::process()
{
  for (size_t object_i = 0; object_i < num_objects; object_i++) {
    for (size_t gain_i = 0; gain_i < panner->num_gains(); gain_i++) {
      direct_gains_out.data()(gain_i, object_i) = gains_in.data()(gain_i, object_i);
      diffuse_gains_out.data()(gain_i, object_i) = gains_in.data()(panner->num_gains() + gain_i, object_i);
    }
  }
}

}  // namespace bear
