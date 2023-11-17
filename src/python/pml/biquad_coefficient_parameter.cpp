/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpml/biquad_parameter.hpp>
#include <libpml/empty_parameter_config.hpp>
#include <libpml/matrix_parameter_config.hpp>
#include <libpml/vector_parameter_config.hpp>

#include <libvisr/parameter_base.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <array>
#include <vector>

namespace visr
{
namespace py = pybind11;

using pml::BiquadCoefficientParameter;
using pml::BiquadCoefficientListParameter;
using pml::BiquadCoefficientMatrixParameter;
//using pml::BiquadParameterMatrix; // deprecated name!
using pml::EmptyParameterConfig;
using pml::MatrixParameterConfig;
using pml::VectorParameterConfig;

namespace python
{
namespace pml
{
template< typename DataType >
void exportBiquadCoefficientParameter( pybind11::module & m,
                                       char const * className )
{
  py::class_< BiquadCoefficientParameter< DataType >,
      rbbl::BiquadCoefficient< DataType >, ParameterBase >(
      m, className )
      // Note: See pybind11 documentation for the way the implicit 'self'
      // argument is stripped by using a lambda function.
      .def_property_readonly_static(
          "staticType", []( pybind11::object /*self*/ )
          { return BiquadCoefficientParameter< DataType >::staticType(); } )
      // We have to repeat the rbbl::BiquadCoefficient constructor bindings here,
      // because there's apparently no way to inherit them in the style of C++'s
      // "using Base::Base"
      .def( py::init<>(), "Default constructor" )
      .def( py::init< visr::ParameterConfigBase const & >(),
            py::arg( "config" ) )
      .def( py::init< visr::pml::EmptyParameterConfig const & >(),
            py::arg( "config" ) )
      .def( py::init< visr::rbbl::BiquadCoefficient< DataType > const & >(),
            py::arg( "rhs" ) )
      .def( py::init< DataType, DataType, DataType, DataType, DataType >(),
            py::arg( "b0" ), py::arg( "b1" ), py::arg( "b2" ), py::arg( "a1" ),
            py::arg( "a2" ) )
      .def( py::init(
                []( std::array< DataType, rbbl::BiquadCoefficient< DataType >::
                                               cNumberOfCoeffs > const & val )
                {
                  return new BiquadCoefficientParameter< DataType >(
                      val[ 0 ], val[ 1 ], val[ 2 ], val[ 3 ], val[ 4 ] );
                } ),
            py::arg( "val" ) )
      ;
}

template< typename DataType >
void exportBiquadCoefficientListParameter( pybind11::module & m,
                                           char const * className )
{
  py::class_< visr::pml::BiquadCoefficientListParameter< DataType >,
              rbbl::BiquadCoefficientList< DataType >, ParameterBase >(
      m, className )
      // Note: See pybind11 documentation for the way the implicit 'self'
      // argument is stripped by using a lambda function.
      .def_property_readonly_static(
          "staticType", []( pybind11::object /*self*/ )
          { return BiquadCoefficientListParameter< DataType >::staticType(); } )
      // We have to repeat the rbbl::BiquadCoefficient constructor bindings
      // here, because there's apparently no way to inherit them in the style of
      // C++'s "using Base::Base"
      .def( py::init<>(), "Default constructor" )
      .def( py::init< visr::ParameterConfigBase const & >() )
      .def( py::init< visr::pml::EmptyParameterConfig const & >(),
            py::arg( "config" ) )
      .def( py::init< visr::rbbl::BiquadCoefficientList< DataType > const & >(),
            py::arg( "rhs" ) )
      .def( py::init< std::size_t >(), py::arg( "size" ),
            "Construct with given number of default elements." )
      .def( py::init(
                []( std::vector< rbbl::BiquadCoefficient< DataType > > const &
                        vec )
                {
                  BiquadCoefficientListParameter< DataType > * newList =
                      new BiquadCoefficientListParameter< DataType >(
                          vec.size() );
                  std::copy( vec.begin(), vec.end(), newList->begin() );
                  return newList;
                } ),
            py::arg( "val" ),
            "Construct from Python list of biquad coefficients." );
}

template< typename DataType >
void exportBiquadCoefficientMatrixParameter( pybind11::module & m,
                                             char const * className )
{
  py::class_< visr::pml::BiquadCoefficientMatrixParameter< DataType >,
              rbbl::BiquadCoefficientMatrix< DataType >, ParameterBase >(
      m, className )
      .def_property_readonly_static(
          "staticType",
          []( pybind11::object /*self*/ ) {
            return BiquadCoefficientMatrixParameter< DataType >::staticType();
          } )
      // We have to repeat the rbbl::BiquadCoefficientMatrix constructor
      // bindings here, because there's apparently no way to inherit them in the
      // style of C++'s "using Base::Base"
      .def( py::init<>(), "Default constructor" )
      .def( py::init< visr::ParameterConfigBase const & >() )
      .def( py::init< visr::pml::EmptyParameterConfig const & >(),
            py::arg( "config" ) )
      //// Note: this ctor produces a cryptic compile error (on MSVC)
      .def(
          py::init< visr::rbbl::BiquadCoefficientMatrix< DataType > const & >(),
          py::arg( "rhs" ) )
      .def( py::init< std::size_t, std::size_t >(), py::arg( "numberOfRows" ),
            py::arg( "numberOfColumns" ),
            "Construct matrix of a given size and default-constructed "
            "elements." )
      .def( py::init(
                []( 
                    std::vector< std::vector< rbbl::BiquadCoefficient< DataType > > > const &
                        mtx )
                {
                  std::size_t const numRows = mtx.size();
                  std::size_t const numCols = numRows == 0 ? 0 : mtx[ 0 ].size();
                  if( numRows > 1 )
                  {
                    for( std::size_t row(1); row < numRows; ++row )
                    {
                      if( mtx[ row ].size() != numCols )
                      {
                        throw std::invalid_argument( "BiquadCoefficientMatrixParameter: Rows have differing lengths." );
                      }
                    }
                  }
                  BiquadCoefficientMatrixParameter< DataType > * newMtx =
                      new BiquadCoefficientMatrixParameter< DataType >(
                          numRows, numCols );
                  for( std::size_t row( 0 ); row < numRows; ++row )
                  {
                    for( std::size_t col( 0 ); col < numCols; ++col )
                    {
                      ( *newMtx )( row, col ) = mtx[row][col];
                    }
                  }
                  return newMtx;
                }),
            py::arg( "val" ),
            "Construct from Python list of biquad coefficients." )
              ;
}


void exportBiquadCoefficientParameter( pybind11::module & m )
{
  exportBiquadCoefficientParameter< float >(
      m, "BiquadCoefficientParameterFloat" );
  exportBiquadCoefficientParameter< double >(
      m, "BiquadCoefficientParameterDouble" );

  exportBiquadCoefficientListParameter< float >(
      m, "BiquadCoefficientListParameterFloat" );
  exportBiquadCoefficientListParameter< double >(
      m, "BiquadCoefficientListParameterDouble" );

  exportBiquadCoefficientMatrixParameter< float >(
      m, "BiquadCoefficientMatrixParameterFloat" );
  exportBiquadCoefficientMatrixParameter< double >(
      m, "BiquadCoefficientMatrixParameterDouble" );
}

} // namespace pml
} // namespace python
} // namespace visr
