#include "select_brir.hpp"

#include <Eigen/Geometry>
#include <boost/math/constants/constants.hpp>

#include "utils.hpp"

namespace bear {
SelectBRIR::SelectBRIR(const SignalFlowContext &ctx,
                       const char *name,
                       CompositeComponent *parent,
                       const ConfigImpl &,
                       std::shared_ptr<Panner> panner_)
    : AtomicComponent(ctx, name, parent),
      panner(std::move(panner_)),
      views(panner->get_views()),
      listener_in("listener_in", *this, pml::EmptyParameterConfig()),
      brir_index_out("brir_index_out", *this, pml::EmptyParameterConfig()),
      listener_out("listener_out", *this, pml::EmptyParameterConfig())
{
  views.rowwise().normalize();

  for (const auto &view : views.rowwise()) {
    // calculate a rotation from the view vector, assuming it's a rotation around z
    bear_assert(std::abs(view.z()) < 1e-6, "expected BRIR rotations around z only");
    double z = std::sin(-std::atan2(view.x(), view.y()) / 2.0);
    double w = std::sqrt(1.0 - z * z);
    Eigen::Quaterniond rot(w, 0.0, 0.0, z);

    view_rotations.push_back(rot);
  }
}

void SelectBRIR::process()
{
  if (listener_in.changed()) {
    Eigen::Vector3d look_vector = listener_in.data().look();

    // find the closest BRIR view, by maximum dot-product
    unsigned int max_dp_idx;
    (views * look_vector).maxCoeff(&max_dp_idx);

    brir_index_out.data() = max_dp_idx;
    brir_index_out.swapBuffers();

    // calculate the 'residual' listener with the BRIR rotation removed
    Eigen::Vector3d front{0.0, 1.0, 0.0};
    Eigen::Quaterniond brir_rot = Eigen::Quaterniond::FromTwoVectors(front, views.row(max_dp_idx));
    listener_out.data().position = listener_in.data().position;
    listener_out.data().orientation = view_rotations[max_dp_idx] * listener_in.data().orientation;
    listener_out.swapBuffers();

    listener_in.resetChanged();
  }
}
}  // namespace bear
