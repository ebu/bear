#include "sh_rotation.hpp"

namespace {
Eigen::Matrix3d quaternion_to_rotation_mat(const Eigen::Quaterniond &q)
{
  // this quaternion is in ADM coords
  Eigen::Matrix3d adm_matrix = q.normalized().toRotationMatrix().transpose();
  // ADM dims: x - right, y - front, z - up
  // HOA dims: x - front, y - left,  z - up
  // ACN order (HOA):   y, z, x  (left, up, front)
  // HOA to ADM: (-y, x, z)
  // ADM to HOA: ( y,-x, z)
  // re-order dimensions
  Eigen::Matrix3d rotation_matrix;
  rotation_matrix(0, 0) = adm_matrix(0, 0);
  rotation_matrix(0, 1) = -adm_matrix(2, 0);
  rotation_matrix(0, 2) = -adm_matrix(1, 0);
  rotation_matrix(1, 0) = -adm_matrix(0, 2);
  rotation_matrix(1, 1) = adm_matrix(2, 2);
  rotation_matrix(1, 2) = adm_matrix(1, 2);
  rotation_matrix(2, 0) = -adm_matrix(0, 1);
  rotation_matrix(2, 1) = adm_matrix(2, 1);
  rotation_matrix(2, 2) = adm_matrix(1, 1);

  return rotation_matrix;
}

inline double KroneckerDelta(int a, int b) { return (a == b) ? 1. : 0.; }

// functions to compute terms U, V, W of Eq.8.1 (Table II)
// function to compute term P of U,V,W (Table II)
double P(int i, int l, int mu, int m_, const Eigen::Matrix3d &R_1, Eigen::MatrixXd &R_lm1)
{
  double ri1 = R_1(i + 1, 2);
  double rim1 = R_1(i + 1, 0);
  double ri0 = R_1(i + 1, 1);

  if (m_ == -l)
    return ri1 * R_lm1(mu + l - 1, 0) + rim1 * R_lm1(mu + l - 1, 2 * l - 2);
  else {
    if (m_ == l)
      return ri1 * R_lm1(mu + l - 1, 2 * l - 2) - rim1 * R_lm1(mu + l - 1, 0);
    else
      return ri0 * R_lm1(mu + l - 1, m_ + l - 1);
  }
}

double U(int l, int m, int n, const Eigen::Matrix3d &R_1, Eigen::MatrixXd &R_lm1)
{
  return P(0, l, m, n, R_1, R_lm1);
}

double V(int l, int m, int n, const Eigen::Matrix3d &R_1, Eigen::MatrixXd &R_lm1)
{
  if (m == 0) {
    double p0 = P(1, l, 1, n, R_1, R_lm1);
    double p1 = P(-1, l, -1, n, R_1, R_lm1);
    return p0 + p1;
  } else {
    if (m > 0) {
      double d = KroneckerDelta(m, 1);
      double p0 = P(1, l, m - 1, n, R_1, R_lm1);
      double p1 = P(-1, l, -m + 1, n, R_1, R_lm1);
      return p0 * sqrt(1. + d) - p1 * (1. - d);
    } else {
      double d = KroneckerDelta(m, -1);
      double p0 = P(1, l, m + 1, n, R_1, R_lm1);
      double p1 = P(-1, l, -m - 1, n, R_1, R_lm1);
      return p0 * (1. - d) + p1 * sqrt(1. + d);
    }
  }
}

double W(int l, int m, int n, const Eigen::Matrix3d &R_1, Eigen::MatrixXd &R_lm1)
{
  if (m != 0) {
    if (m > 0) {
      double p0 = P(1, l, m + 1, n, R_1, R_lm1);
      double p1 = P(-1, l, -m - 1, n, R_1, R_lm1);
      return p0 + p1;
    } else {
      double p0 = P(1, l, m - 1, n, R_1, R_lm1);
      double p1 = P(-1, l, -m + 1, n, R_1, R_lm1);
      return p0 - p1;
    }
  } else
    return 0.0;
}

void CalculateTransform(const Eigen::Matrix3d &rotation_matrix,
                        int order,
                        Eigen::Ref<Eigen::MatrixXd> sh_transform)
{
  // Ivanic, J., Ruedenberg, K. (1996). Rotation Matrices for Real
  // Spherical Harmonics. Direct Determination by Recursion.
  // The Journal of Physical Chemistry

  // short-hand reference to first-order rotation matrix
  const Eigen::Matrix3d &R_1 = rotation_matrix;
  sh_transform.setZero();
  // zeroth order is invariant
  sh_transform(0, 0) = 1.;
  // set first order rotation
  sh_transform.block(1, 1, 3, 3) = R_1;

  Eigen::MatrixXd R_lm1 = R_1;

  // recursively generate higher orders
  for (int l = 2; l <= order; l++) {
    Eigen::MatrixXd R_l = Eigen::MatrixXd::Zero(2 * l + 1, 2 * l + 1);
    for (int m = -l; m <= l; m++) {
      for (int n = -l; n <= l; n++) {
        // Table I
        double d = KroneckerDelta(m, 0);
        double denom = 0.;
        if (abs(n) == l)
          denom = (2 * l) * (2 * l - 1);
        else
          denom = (l * l - n * n);

        double u = sqrt((l * l - m * m) / denom);
        double v = sqrt((1. + d) * (l + abs(m) - 1.) * (l + abs(m)) / denom) * (1. - 2. * d) * 0.5;
        double w = sqrt((l - abs(m) - 1.) * (l - abs(m)) / denom) * (1. - d) * (-0.5);

        if (u != 0.) u *= U(l, m, n, R_1, R_lm1);
        if (v != 0.) v *= V(l, m, n, R_1, R_lm1);
        if (w != 0.) w *= W(l, m, n, R_1, R_lm1);

        R_l(m + l, n + l) = u + v + w;
      }
    }
    sh_transform.block(l * l, l * l, 2 * l + 1, 2 * l + 1) = R_l;
    R_lm1 = R_l;
  }
  // threshold coefficients to make rotation more efficient
  double zero_threshold = 1e-5;
  for (double &x : sh_transform.reshaped()) {
    if (fabs(x) < zero_threshold) x = 0.0;
  }
}
}  // namespace

namespace bear {

void quaternion_to_sh_rotation_matrix(const Eigen::Quaterniond &q,
                                      int order,
                                      Eigen::Ref<Eigen::MatrixXd> sh_transform)
{
  if (order <= 0) throw std::invalid_argument("order must be +ve");

  size_t n_channels = (order + 1) * (order + 1);
  if (sh_transform.rows() != n_channels || sh_transform.cols() != n_channels)
    throw std::invalid_argument("sh_transform is not correct size");

  Eigen::Matrix3d rotation_matrix = quaternion_to_rotation_mat(q);
  CalculateTransform(rotation_matrix, order, sh_transform);
}

}  // namespace bear
