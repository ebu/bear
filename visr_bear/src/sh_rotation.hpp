#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>

namespace bear {
void quaternion_to_sh_rotation_matrix(const Eigen::Quaterniond &q,
                                      int order,
                                      Eigen::Ref<Eigen::MatrixXd> sh_mat);
}  // namespace bear
