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

namespace py = pybind11;

/**
 * Construct a Numpy array from a visr::efl::BasicMatrix.
 * @todo Consider moving to a library (visr::python::bindinghelpers?)
 */
template<typename DataType>
py::array_t<DataType> ndArrayFromBasicMatrix( visr::efl::BasicMatrix<DataType> const & mtx )
{
  std::size_t const numRows{ mtx.numberOfRows() };
  std::size_t const numColumns{ mtx.numberOfColumns() };
#if 1
  py::array::StridesContainer const strides( 
    { static_cast<ssize_t>(mtx.stride()) * sizeof(DataType), sizeof(DataType) } );
  py::array_t<DataType> res( { numRows, numColumns }, strides, mtx.data() );
#else
  // Use slow, element-wise copying instead
  py::array_t<DataType> res( { numRows, numColumns } );
  for( std::size_t rowIdx{0}; rowIdx < numRows; ++rowIdx )
  {
    for( std::size_t colIdx{0}; colIdx < numColumns; ++colIdx )
    {
      res.mutable_at(rowIdx, colIdx)= mtx( rowIdx, colIdx );
    }
  }
#endif
  return res;
}

} // namepace bindinghelpers
} // namespace python
} // namespace visr
