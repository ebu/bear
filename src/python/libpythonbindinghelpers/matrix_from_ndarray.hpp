/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/basic_matrix.hpp> 

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace visr
{

namespace python
{
namespace bindinghelpers
{

/**
 * Create a efl::BasicMatrix from a 2-D numpy.ndarray.
 * @tparam DataType The element data type.
 * @param array A 2D ndarray with the Numpy equivalent of \p DataType.
 * @param alignment Element alignment of the resulting BasicMatrix, default 0.
 * @return Initialized matrix object.
 */
template<typename DataType>
efl::BasicMatrix<DataType> matrixFromNdArray( pybind11::array_t<DataType> const & array, std::size_t alignment = 0 )
{
  if( array.ndim() != 2 )
  {
    throw std::invalid_argument( "efl::BasicMatrix from numpy ndarray: Input array must be 2D" );
  }
  // Should not happen, because we use the typed array_t template type.
  if( not array.dtype().is( pybind11::dtype::of<DataType>() ) )
  {
    throw std::invalid_argument( "efl::BasicMatrix from numpy ndarray: Input matrix has a different data type (dtype)." );
  }
  std::size_t const numRows = static_cast<pybind11::ssize_t>(array.shape()[0]);
  std::size_t const numCols = static_cast<pybind11::ssize_t>(array.shape()[1]);
  efl::BasicMatrix<DataType> ret( numRows, numCols, alignment );
  for( std::size_t rowIdx( 0 ); rowIdx < numRows; ++rowIdx )
  {
    for( std::size_t colIdx( 0 ); colIdx < numCols; ++colIdx )
    {
      ret( rowIdx, colIdx ) = *static_cast<DataType const *>(array.data( rowIdx, colIdx ));
    }
  }
  return ret;
}

} // namepace bindinghelpers
} // namespace python
} // namespace visr
