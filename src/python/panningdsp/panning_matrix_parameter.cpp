/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpanningdsp/panning_matrix_parameter.hpp>

#include <libpml/matrix_parameter_config.hpp> 

#include <libefl/basic_matrix.hpp> 

#include <libvisr/constants.hpp>
#include <libvisr/parameter_base.hpp>

#include <python/libpythonbindinghelpers/matrix_from_ndarray.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <algorithm>

namespace py = pybind11;

namespace visr
{
using pml::MatrixParameterConfig;

namespace panningdsp
{
namespace python
{

namespace // unnamed
{

template< typename T >
efl::AlignedArray< T > arrayFromNdArray(py::array_t< T > const & array, std::size_t alignment = 0)
{
  if (array.ndim() != 1)
  {
    throw std::invalid_argument("efl::BasicVector from numpy ndarray: Input array must be 1D");
  }
  // Should not happen, because we use the typed array_t template type.
  if (not array.dtype().is(pybind11::dtype::of<T>()))
  {
    throw std::invalid_argument("efl::BasicVector from numpy ndarray: Input matrix has a different data type (dtype).");
  }
  std::size_t const numElements = static_cast<pybind11::ssize_t>(array.shape()[0]);
  efl::AlignedArray< T > ret( numElements, alignment );
  for (std::size_t elIdx(0); elIdx < numElements; ++elIdx)
  {
    ret[elIdx] = *static_cast<T const *>(array.data(elIdx));
  }
  return ret;
}

template< typename T >
py::array_t< T > arrayToNdArray(efl::AlignedArray< T > const & array )
{
  py::array_t< T > ret( array.size(), array.data() );
  // std::copy(array.data(), array.data() + array.size(), ret.mutable_data() );
  return ret;
}

template< typename T >
void copy(efl::AlignedArray< T > const & src, efl::AlignedArray< T > & dest)
{
  if (src.size() != dest.size())
  {
    throw std::invalid_argument( "Array size are different." );
  }
  std::copy(src.data(), src.data()+src.size(), dest.data() );
}

} // unnamed namespace


void exportPanningMatrixParameter(pybind11::module & m)
{
  using SampleType = panningdsp::SampleType;
  pybind11::class_< panningdsp::PanningMatrixParameter, ParameterBase >(m, "PanningMatrixParameter" )

    .def_property_readonly_static("staticType", [](pybind11::object /*self*/){ return PanningMatrixParameter::staticType(); } )
    .def(pybind11::init<std::size_t, std::size_t, std::size_t >(),
     pybind11::arg( "numberOfObjects" ), pybind11::arg( "numberOfLoudspeakers" ),
     pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples)
    .def(py::init([](py::array_t< SampleType > const & gains,
      py::array_t< TimeType > const & timeStamps,
      py::array_t< TimeType > const & transitionTimes,
      std::size_t alignment )
      {
        visr::efl::BasicMatrix<SampleType> const gainMtx
          = visr::python::bindinghelpers::matrixFromNdArray(gains, alignment);
        visr::efl::AlignedArray<TimeType> const timeStampArray
          = arrayFromNdArray< TimeType >( timeStamps, alignment );
        visr::efl::AlignedArray<TimeType> const & transitionTimesArray
          = arrayFromNdArray< TimeType >( transitionTimes, alignment );
        return new PanningMatrixParameter( gainMtx, timeStampArray, transitionTimesArray );
      } ),
       pybind11::arg( "gains" ), pybind11::arg( "timeStamps" ),
       pybind11::arg( "transitionTime" ), pybind11::arg( "alignment") = 0 )
    .def( pybind11::init< visr::ParameterConfigBase const & >(), py::arg( "config" ) )
    .def(pybind11::init< pml::MatrixParameterConfig const & >(), py::arg( "config" ) )
    .def_property_readonly( "numberOfObjects", &PanningMatrixParameter::numberOfObjects )
    .def_property_readonly( "numberOfLoudspeakers", &PanningMatrixParameter::numberOfLoudspeakers )
    .def_property_readonly( "alignmentElements", &PanningMatrixParameter::alignmentElements )
    .def_property( "gains", static_cast<GainMatrixType const &(PanningMatrixParameter::*)()const >(&PanningMatrixParameter::gains),
      [](PanningMatrixParameter & self, py::array_t< SampleType > const & gains )
      {
        GainMatrixType const gainMtx
          = visr::python::bindinghelpers::matrixFromNdArray < SampleType >( gains );
        self.gains().copy( gainMtx );
      } )
    .def_property( "timeStamps",
      [](PanningMatrixParameter & self)
      {
        return arrayToNdArray< TimeType >(self.timeStamps() );
      },
      [](PanningMatrixParameter & self, py::array_t< TimeType > const & timeStamps )
      {
        TimeVector tsVec = arrayFromNdArray< TimeType >( timeStamps );
        copy( tsVec, self.timeStamps() );
      })
      .def_property("transitionTimes",
          [](PanningMatrixParameter & self)
      {
        return arrayToNdArray< TimeType >(self.transitionTimes());
      },
          [](PanningMatrixParameter & self, py::array_t< TimeType > const & transitionTimes)
      {
        TimeVector ttVec = arrayFromNdArray< TimeType >(transitionTimes);
        copy( ttVec, self.transitionTimes() );
      })
    ;
}

} // namespace python
} // namespace panningdsp
} // namespace visr
