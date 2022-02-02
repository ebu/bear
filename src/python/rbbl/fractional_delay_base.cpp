// Copyright (c) 2014-2020 Institute of Sound and Vibration Research,
// University of Southampton and VISR contributors --- All rights reserved.
// Copyright (c) 2014-2020 Andreas Franck a.franck@soton.ac.uk --- All rights reserved.

#include <librbbl/fractional_delay_base.hpp>

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

template < typename SampleType >
class FractionalDelayBaseWrapper: public FractionalDelayBase< SampleType >
{
public:
  virtual SampleType methodDelay() const override
  {
    PYBIND11_OVERLOAD_PURE( SampleType, FractionalDelayBase< SampleType >, methodDelay, );
  }

  virtual void interpolate( SampleType const * basePointer,
                            SampleType * result,
                            std::size_t numSamples,
                            SampleType startDelay, SampleType endDelay,
                            SampleType startGain, SampleType endGain ) override
  {
    PYBIND11_OVERLOAD_PURE( void, FractionalDelayBase< SampleType >, interpolate, basePointer, result, numSamples, startDelay, endDelay, startGain, endGain );
  }
};

namespace // unnamed 
{

template< template<typename> class Container, typename SampleType >
void interpolateWrapper( FractionalDelayBase< SampleType > & self,
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



template< typename SampleType >
void exportFractionalDelayBase( pybind11::module & m, char const * className )
{
  py::class_< FractionalDelayBase<SampleType >, FractionalDelayBaseWrapper<SampleType> >( m, className )
    .def( "methodDelay", &FractionalDelayBase< SampleType >::methodDelay )
    .def( "interpolate", &interpolateWrapper<visr::python::bindinghelpers::PyArray, SampleType > )
    .def( "interpolate", &interpolateWrapper<visr::efl::BasicVector, SampleType > )
  ;
}

} // unnamed namespace

void exportFractionalDelayBase( pybind11::module & m )
{
  exportFractionalDelayBase<float>( m, "FractionalDelayBaseFloat" );
  exportFractionalDelayBase<double>( m, "FractionalDelayBaseDouble" );
}

} // namespace python
} // namepace rbbl
} // namespace visr
