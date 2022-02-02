/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpml/matrix_parameter.hpp> 
#include <libpml/matrix_parameter_config.hpp> 

#include <libefl/basic_matrix.hpp> 

#include <libvisr/constants.hpp>
#include <libvisr/parameter_base.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <complex>

namespace visr
{

using pml::MatrixParameter;
using pml::MatrixParameterConfig;

namespace python
{
namespace pml
{

template<typename DataType>
void exportMatrixParameter( pybind11::module & m, char const * className )
{
  pybind11::class_<MatrixParameter<DataType >, efl::BasicMatrix<DataType>, ParameterBase >(m, className, pybind11::buffer_protocol() )
    .def_buffer([](MatrixParameter<DataType> &mp) -> pybind11::buffer_info
                {
                  return pybind11::buffer_info( mp.data(),
                                                sizeof( DataType ),
                                                pybind11::format_descriptor<DataType>::format(),
                                                2,
                                                { mp.numberOfRows(), mp.numberOfColumns() },
                                                { sizeof( DataType ) * mp.stride(), sizeof( DataType ) } );
                }
      )
  .def( pybind11::init<std::size_t>(), pybind11::arg("alignment") = visr::cVectorAlignmentSamples )
  .def( pybind11::init<std::size_t, std::size_t, std::size_t>(), pybind11::arg( "numberOfRows" ), pybind11::arg( "numberOfColumns" ), pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples )
  // Note: See pybind11 documentation for the way the implicit 'self' argument is stripped by using a lambda function.
  .def_property_readonly_static( "staticType", [](pybind11::object /*self*/) {return MatrixParameter<DataType>::staticType(); } )
  .def( pybind11::init( []( pybind11::array const & data, std::size_t alignment)
  {
    if( data.ndim() != 2 )
    {
      throw std::invalid_argument( "MatrixParameter from numpy ndarray: Input array must be 2D" );
    }
    if( not data.dtype().is( pybind11::dtype::of<DataType>() ) )
    {
      throw std::invalid_argument( "MatrixParameter from numpy ndarray: Input matrix has a different data type (dtype)." );
    }
    std::size_t const numRows = static_cast<pybind11::ssize_t>(data.shape()[0]);
    std::size_t const numCols = static_cast<pybind11::ssize_t>(data.shape()[1]);
    MatrixParameter<DataType> * inst = new MatrixParameter<DataType>( numRows, numCols, alignment);
    for( std::size_t rowIdx(0); rowIdx < numRows; ++rowIdx )
    {
      for( std::size_t colIdx(0); colIdx < numCols; ++colIdx )
      {
        inst->at( rowIdx, colIdx ) = *static_cast<DataType const *>(data.data( rowIdx, colIdx ));
      }
    }
    return inst;
  }), pybind11::arg("data"), pybind11::arg("alignment") = visr::cVectorAlignmentSamples )
  .def( pybind11::init<visr::ParameterConfigBase const &>(), pybind11::arg( "config" ))
  .def( pybind11::init<MatrixParameterConfig const &>(), pybind11::arg( "config" ))
#ifdef VISR_PML_USE_SNDFILE_LIBRARY
  .def_static( "fromAudioFile", &MatrixParameter<DataType>::fromAudioFile, pybind11::arg("file"), pybind11::arg("alignment") = visr::cVectorAlignmentSamples )
#endif
  .def_static( "fromTextFile", &MatrixParameter<DataType>::fromTextFile, pybind11::arg( "file" ), pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples )
  ;
}

void exportMatrixParameters( pybind11::module & m)
{
  pybind11::class_<MatrixParameterConfig, ParameterConfigBase >( m, "MatrixParameterConfig" )
    .def( pybind11::init<std::size_t, std::size_t>(), pybind11::arg("numberOfRows" ), pybind11::arg("numberOfColumns") )
    .def_property_readonly( "numberOfRows", &MatrixParameterConfig::numberOfRows )
    .def_property_readonly( "numberOfColumns", &MatrixParameterConfig::numberOfColumns )
    .def( "compare", static_cast<bool(MatrixParameterConfig::*)(MatrixParameterConfig const&) const>(&MatrixParameterConfig::compare),  pybind11::arg("rhs") )
    .def( "compare", static_cast<bool(MatrixParameterConfig::*)(ParameterConfigBase const&) const>(&MatrixParameterConfig::compare),  pybind11::arg("rhs") )
  ;

  exportMatrixParameter<float>( m, "MatrixParameterFloat" );
  exportMatrixParameter<double>( m, "MatrixParameterDouble" );
  exportMatrixParameter<std::complex<float>>( m, "MatrixParameterComplexFloat" );
  exportMatrixParameter<std::complex<double>>( m, "MatrixParameterComplexDouble" );
}

} // namepace pml
} // namespace python
} // namespace visr
