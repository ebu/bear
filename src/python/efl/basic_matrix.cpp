/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/basic_matrix.hpp>

#include <libvisr/constants.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <complex>

namespace visr
{
namespace efl
{
namespace python
{

template<typename DataType>
void exportBasicMatrix( pybind11::module & m, char const * className )
{
  pybind11::class_<efl::BasicMatrix< DataType > >(m, className, pybind11::buffer_protocol() )
  .def_buffer([](efl::BasicMatrix<DataType> &mp) -> pybind11::buffer_info
  {
    return pybind11::buffer_info( mp.data(),
     sizeof( DataType ),
     pybind11::format_descriptor<DataType>::format(),
     2,
     { mp.numberOfRows(), mp.numberOfColumns() },
     { sizeof( DataType ) * mp.stride(), sizeof( DataType ) } );
  } )
  .def( pybind11::init<std::size_t>(), pybind11::arg("alignment") = visr::cVectorAlignmentSamples )
  .def( pybind11::init<std::size_t, std::size_t, std::size_t>(), pybind11::arg( "numberOfRows" ), pybind11::arg( "numberOfColumns" ), pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples )
  .def( pybind11::init( []( pybind11::array_t<DataType> const & data, std::size_t alignment )
  {
    if( data.ndim() != 2 )
    {
      throw std::invalid_argument( "efl::BasicMatrix from numpy ndarray: Input array must be 2D" );
    }
    if( not data.dtype().is( pybind11::dtype::of<DataType>() ) )
    {
      throw std::invalid_argument( "efl::BasicMatrix from numpy ndarray: Input matrix has a different data type (dtype)." );
    }
    std::size_t const numRows = static_cast<pybind11::ssize_t>(data.shape()[0]);
    std::size_t const numCols = static_cast<pybind11::ssize_t>(data.shape()[1]);
    efl::BasicMatrix<DataType> * inst = new efl::BasicMatrix<DataType>( numRows, numCols, alignment );
    for( std::size_t rowIdx( 0 ); rowIdx < numRows; ++rowIdx )
    {
      for( std::size_t colIdx( 0 ); colIdx < numCols; ++colIdx )
      {
        (*inst)( rowIdx, colIdx ) = *static_cast<DataType const *>(data.data( rowIdx, colIdx ));
      }
    }
    return inst;
  }), pybind11::arg( "data" ), pybind11::arg( "alignment" ) = visr::cVectorAlignmentSamples )
  .def_property_readonly( "numberOfRows", &efl::BasicMatrix<DataType>::numberOfRows )
  .def_property_readonly( "numberOfColumns", &efl::BasicMatrix<DataType>::numberOfColumns )
    .def_property_readonly( "alignment", &efl::BasicMatrix<DataType>::alignmentElements, "Return the alignment of the numeric data (in number of elements)." )

  .def( "resize", &efl::BasicMatrix<DataType>::resize, pybind11::arg("numberOfRows"), pybind11::arg("numberOfColumns") )
  .def( "zeroFill", &efl::BasicMatrix<DataType>::zeroFill )
  .def( "__getitem__", []( BasicMatrix<DataType> const & vp, pybind11::tuple idx ) { return vp.at( idx[0].cast<std::size_t>(), idx[1].cast<std::size_t>() ); }, pybind11::arg( "index" ) )
  .def( "__setitem__", []( BasicMatrix<DataType> & vp, pybind11::tuple idx, DataType val ) { vp.at( idx[0].cast<std::size_t>(), idx[1].cast<std::size_t>() ) = val; }, pybind11::arg( "index" ), pybind11::arg( "value" ) )
  ;
}

void exportBasicMatrices( pybind11::module & m)
{
  exportBasicMatrix<float>( m, "BasicMatrixFloat" );
  exportBasicMatrix<double>( m, "BasicMatrixDouble" );
  exportBasicMatrix<std::complex<float> >( m, "BasicMatrixComplexFloat" );
  exportBasicMatrix<std::complex<double> >( m, "BasicMatrixComplexDouble" );
}

} // namepace pml
} // namespace python
} // namespace visr
