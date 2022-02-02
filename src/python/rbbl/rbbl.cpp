/* Copyright Institute of Sound and Vibration Research - All rights reserved */


#include <pybind11/pybind11.h>

// Forward declarations
namespace visr
{
namespace rbbl
{
namespace python
{
  void exportBiquadCoefficients( pybind11::module & m );
  void exportCoreConvolversUniform( pybind11::module & m );
  void exportCrossfadingConvolversUniform( pybind11::module & m );
  void exportFilterRouting( pybind11::module & m );
  void exportFractionalDelayBase( pybind11::module & m );
  void exportFractionalDelayFactory( pybind11::module & m );
  void exportInterpolationParameter( pybind11::module & m );
  void exportInterpolatingConvolversUniform( pybind11::module & m );
  void exportLagrangeInterpolator( pybind11::module & m );
  void exportMultichannelConvolversUniform( pybind11::module & m );
  void exportObjectChannelAllocator( pybind11::module & m );
  void exportParametricIirCoefficient( pybind11::module & m );
  void exportParametricIirCoefficientCalculator( pybind11::module & m );
  void exportPosition3D(pybind11::module & m );
  void exportQuaternion( pybind11::module & m );
  void exportSparseGainRouting( pybind11::module & m );
}
}
}

PYBIND11_MODULE( rbbl, m )
{
  using namespace visr::rbbl::python;
  exportBiquadCoefficients( m );
  exportFilterRouting( m ); // Needs to come before the convolvers
  exportFractionalDelayBase( m );
  exportFractionalDelayFactory( m );
  exportInterpolationParameter( m );
  exportCoreConvolversUniform( m );
  exportCrossfadingConvolversUniform( m );
  exportInterpolatingConvolversUniform( m );
  exportLagrangeInterpolator( m );
  exportMultichannelConvolversUniform( m );
  exportObjectChannelAllocator( m );
  exportParametricIirCoefficient( m );
  exportParametricIirCoefficientCalculator( m );
  exportPosition3D( m );
  exportQuaternion( m );
  exportSparseGainRouting( m );
}
