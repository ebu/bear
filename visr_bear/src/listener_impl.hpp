#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace bear {

struct ListenerImpl {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  // position of the listener relative to the origin
  // position_rel_listener = position_rel_origin - position
  Eigen::Vector3d position = {0, 0, 0};
  // translate from world coordinates to listener coordinates
  // position_rel_listener = orientation * position
  // position = orientation' * position_rel_listener
  //
  // beware: Eigen::Quaterniond constructor is {w, x, y, z} but coefficients
  // are stored as {x, y, z, w}!
  Eigen::Quaterniond orientation = {1, 0, 0, 0};

  Eigen::Vector3d look() const { return orientation.conjugate() * Eigen::Vector3d{0.0, 1.0, 0.0}; }
  Eigen::Vector3d right() const { return orientation.conjugate() * Eigen::Vector3d{1.0, 0.0, 0.0}; }
  Eigen::Vector3d up() const { return orientation.conjugate() * Eigen::Vector3d{0.0, 0.0, 1.0}; }
};

bool listeners_approx_equal(const ListenerImpl &a, const ListenerImpl &b);

}  // namespace bear
