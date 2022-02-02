// Copyright (c) 2014-2020 Institute of Sound and Vibration Research,
// University of Southampton and VISR contributors --- All rights reserved.
// Copyright (c) 2014-2020 Andreas Franck a.franck@soton.ac.uk --- All rights reserved.

#include <librbbl/lagrange_interpolator.hpp>

#include <python/libpythonbindinghelpers/container_access.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <array>
#include <string>
#include <sstream>
#include <vector>

namespace visr
{
namespace rbbl
{
namespace python
{

namespace py = pybind11;

namespace // unnamed 
{


template< template<typename> class Container, typename SampleType, std::size_t order >
void interpolateWrapper( LagrangeInterpolator< SampleType, order > & self,
  Container< SampleType > const & base,
  Container< SampleType > & result,
  std::size_t numSamples,
  SampleType startDelay, SampleType endDelay,
  SampleType startGain, SampleType endGain )
{
  SampleType const * basePtr = visr::python::bindinghelpers::ContainerAccess<Container, SampleType >
    ::constantPointer( base, numSamples, "base" );
  SampleType * resultPtr = visr::python::bindinghelpers::ContainerAccess<Container, SampleType >
    ::mutablePointer( result, numSamples, "result" );
  self.interpolate( basePtr, resultPtr, numSamples, startDelay, endDelay, startGain, endGain );
}


template< typename SampleType, std::size_t order >
void exportLagrangeInterpolator( pybind11::module & m, char const * className )
{
  py::class_< LagrangeInterpolator<SampleType, order >, FractionalDelayBase<SampleType> >( m, className )
    .def( py::init<std::size_t, std::size_t>(), py::arg("maxNumSamples"), py::arg("alignmentElements")=0 )
    .def( "methodDelay", &LagrangeInterpolator< SampleType, order>::methodDelay )
    .def( "interpolate", &interpolateWrapper<visr::python::bindinghelpers::PyArray, SampleType, order > )
    .def( "interpolate", &interpolateWrapper<visr::efl::BasicVector, SampleType, order > )
  ;
}

} // unnamed namespace

void exportLagrangeInterpolator( pybind11::module & m )
{
  exportLagrangeInterpolator< float, 0 >( m, "LagrangeInterpolatorOrder0Float" );
  exportLagrangeInterpolator< float, 1 >( m, "LagrangeInterpolatorOrder1Float" );
  exportLagrangeInterpolator< float, 3 >( m, "LagrangeInterpolatorOrder3Float" );
  exportLagrangeInterpolator< float, 5 >( m, "LagrangeInterpolatorOrder5Float" );
  exportLagrangeInterpolator< float, 7 >( m, "LagrangeInterpolatorOrder7Float" );
  exportLagrangeInterpolator< float, 9 >( m, "LagrangeInterpolatorOrder9Float" );

  exportLagrangeInterpolator< double, 0 >( m, "LagrangeInterpolatorOrder0Double" );
  exportLagrangeInterpolator< double, 1 >( m, "LagrangeInterpolatorOrder1Double" );
  exportLagrangeInterpolator< double, 3 >( m, "LagrangeInterpolatorOrder3Double" );
  exportLagrangeInterpolator< double, 5 >( m, "LagrangeInterpolatorOrder5Double" );
  exportLagrangeInterpolator< double, 7 >( m, "LagrangeInterpolatorOrder7Double" );
  exportLagrangeInterpolator< double, 9 >( m, "LagrangeInterpolatorOrder9Double" );
}

} // namespace python
} // namepace rbbl
} // namespace visr
