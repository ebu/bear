/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpml/vector_parameter.hpp> 
#include <libpml/vector_parameter_config.hpp> 

#include <libvisr/constants.hpp>
#include <libvisr/parameter_base.hpp>

#include <python/libpythonbindinghelpers/vector_from_ndarray.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace visr
{

using pml::VectorParameter;
using pml::VectorParameterConfig;

namespace python
{
namespace pml
{

namespace py = pybind11;

template<typename DataType>
void exportVectorParameter( py::module & m, char const * className )
{
  py::class_<VectorParameter< DataType>, ParameterBase, efl::BasicVector<DataType> >(m, className, py::buffer_protocol() )
  .def_property_readonly_static( "staticType", []( py::object /*self*/ ) { return VectorParameter<DataType>::staticType(); } )
  .def( pybind11::init<std::size_t>(), pybind11::arg("alignment") = visr::cVectorAlignmentSamples )
  .def( pybind11::init<std::size_t, std::size_t>(), pybind11::arg( "numberOfElements" ), pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples )
  .def( pybind11::init( []( pybind11::array const & data, std::size_t alignment )
  {
    visr::efl::BasicVector< DataType > raw = visr::python:: bindinghelpers::vectorFromNdArray< DataType >( data, alignment );
    VectorParameter<DataType>* inst = new VectorParameter<DataType>( raw.size(), alignment );
    inst->copy( raw );
    return inst;
  } ), pybind11::arg( "data" ), pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples )
  .def( pybind11::init<visr::pml::VectorParameterConfig>(), pybind11::arg("parameterConfig") )
  // all vector access functions are already defined in BasicVector<DataType>;
#ifdef VISR_PML_USE_SNDFILE_LIBRARY
  .def_static( "fromAudioFile", &VectorParameter<DataType>::fromAudioFile, pybind11::arg("file"), pybind11::arg("alignment") = visr::cVectorAlignmentSamples )
#endif
  .def_static( "fromTextFile", &VectorParameter<DataType>::fromTextFile, pybind11::arg( "file" ), pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples )
  ;
}

void exportVectorParameters( py::module & m)
{
  py::class_<VectorParameterConfig, ParameterConfigBase >( m, "VectorParameterConfig" )
    .def( py::init<std::size_t>(), py::arg("numberOfElements" ) )
    .def_property_readonly( "numberOfElements", &VectorParameterConfig::numberOfElements )
    .def( "compare", static_cast<bool(VectorParameterConfig::*)(VectorParameterConfig const&) const>(&VectorParameterConfig::compare),  py::arg("rhs") )
    .def( "compare", static_cast<bool(VectorParameterConfig::*)(ParameterConfigBase const&) const>(&VectorParameterConfig::compare),  py::arg("rhs") )
  ;

  exportVectorParameter<float>( m, "VectorParameterFloat" );
  exportVectorParameter<double>( m, "VectorParameterDouble" );
  exportVectorParameter< std::complex<float> >( m, "VectorParameterComplexFloat" );
  exportVectorParameter< std::complex<double> >( m, "VectorParameterComplexDouble" );
}

} // namepace pml
} // namespace python
} // namespace visr
