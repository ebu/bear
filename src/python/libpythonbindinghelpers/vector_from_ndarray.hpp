/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/basic_vector.hpp>

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
 * @note Dynamically converting from different dtypes would be more require a runtime dispatch to a
 * suitable conversion function.
 */
template<typename DataType>
efl::BasicVector<DataType> vectorFromNdArray( pybind11::array_t<DataType> const & array, std::size_t alignment = 0 )
{
  if( array.ndim() != 1 )
  {
    throw std::invalid_argument( "efl::BasicVector from numpy ndarray: Input array must be 1D" );
  }
  // Should not happen, because we use the typed array_t template type.
  if( not array.dtype().is( pybind11::dtype::of<DataType>() ) )
  {
    throw std::invalid_argument( "efl::BasicVector from numpy ndarray: Input matrix has a different data type (dtype)." );
  }
  std::size_t const numElements =  static_cast<pybind11::ssize_t>(array.shape()[0]);
  efl::BasicVector<DataType> ret( numElements, alignment );
  for( std::size_t elIdx( 0 ); elIdx < numElements; ++elIdx )
  {
    ret[ elIdx ] = *static_cast<DataType const *>(array.data( elIdx ));
  }
  return ret;
}

} // namepace bindinghelpers
} // namespace python
} // namespace visr
