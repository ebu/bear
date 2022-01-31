#include "geom.hpp"

namespace bear {
// from libear/src/common/geom.cpp

double azimuth(Eigen::Vector3d position) { return -degrees(atan2(position[0], position[1])); }

double elevation(Eigen::Vector3d position)
{
  auto radius = hypot(position[0], position[1]);
  return degrees(atan2(position[2], radius));
}

double distance(Eigen::Vector3d position) { return position.norm(); }

Eigen::Vector3d cart(double azimuth, double elevation, double distance)
{
  return Eigen::Vector3d(sin(radians(-azimuth)) * cos(radians(elevation)) * distance,
                         cos(radians(-azimuth)) * cos(radians(elevation)) * distance,
                         sin(radians(elevation)) * distance);
};

Eigen::Vector3d toCartesianVector3d(ear::CartesianPosition position)
{
  return Eigen::Vector3d(position.X, position.Y, position.Z);
}

Eigen::Vector3d toCartesianVector3d(ear::PolarPosition position)
{
  return cart(position.azimuth, position.elevation, position.distance);
}

Eigen::Vector3d toCartesianVector3d(ear::CartesianSpeakerPosition position)
{
  return Eigen::Vector3d(position.X, position.Y, position.Z);
}

Eigen::Vector3d toCartesianVector3d(ear::PolarSpeakerPosition position)
{
  return cart(position.azimuth, position.elevation, position.distance);
}

}  // namespace bear
