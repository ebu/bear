// Copyright (c) 2014-2020 Institute of Sound and Vibration Research,
// University of Southampton and VISR contributors --- All rights reserved.
// Copyright (c) 2014-2020 Andreas Franck a.franck@soton.ac.uk --- All rights reserved.

#include <librbbl/fractional_delay_base.hpp>
#include <librbbl/fractional_delay_factory.hpp>

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

template< typename SampleType >
void exportFractionalDelayFactory( pybind11::module & m, char const * className )
{
  py::class_<FractionalDelayFactory< SampleType > >( m, className )
    .def_static( "create", &FractionalDelayFactory< SampleType >::create,
      py::arg( "name" ), py::arg( "maxNumSamples" ) )
    ;
}

} // namespace unnamed

void exportFractionalDelayFactory( pybind11::module & m )
{
  exportFractionalDelayFactory<float>( m, "FractionalDelayFactoryFloat" );
  exportFractionalDelayFactory<double>( m, "FractionalDelayFactoryDouble" );
}

} // namespace python
} // namepace rbbl
} // namespace visr
