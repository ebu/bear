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

namespace py = pybind11;

/**
 * Create a 1-D numpy.ndarray from an efl::BasicVector.
 * @tparam DataType The element data type.
 * @param vec A BasicVector data structure..
 * @return A numpy array containing a copy of the vector data.
 */
template<typename DataType>
py::array_t<DataType> ndArrayFromBasicVector( efl::BasicVector<DataType> const & vec )
{
  py::array_t<DataType> res( vec.size(), vec.data() );
  return res;
}

} // namepace bindinghelpers
} // namespace python
} // namespace visr
