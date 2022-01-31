#pragma once
#include <Eigen/Core>
#include <boost/math/constants/constants.hpp>

#include "ear/common_types.hpp"
#include "ear/metadata.hpp"

namespace bear {
// internal utilities from libear

template <typename T>
inline T radians(T d)
{
  return d * static_cast<T>(boost::math::constants::pi<double>() / 180.0);
}

template <typename T>
inline T degrees(T r)
{
  return r * static_cast<T>(180.0 / boost::math::constants::pi<double>());
}

double azimuth(Eigen::Vector3d position);
double elevation(Eigen::Vector3d position);
double distance(Eigen::Vector3d position);

Eigen::Vector3d cart(double azimuth, double elevation, double distance);

Eigen::Vector3d toCartesianVector3d(ear::PolarPosition position);
Eigen::Vector3d toCartesianVector3d(ear::CartesianPosition position);
Eigen::Vector3d toCartesianVector3d(ear::PolarSpeakerPosition position);
Eigen::Vector3d toCartesianVector3d(ear::CartesianSpeakerPosition position);

}  // namespace bear
