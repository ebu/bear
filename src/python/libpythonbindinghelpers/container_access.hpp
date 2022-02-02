/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PYTHON_BINDING_HELPERS_CONTAINER_ACCESS_HPP_INCLUDED
#define VISR_PYTHON_BINDING_HELPERS_CONTAINER_ACCESS_HPP_INCLUDED

#include <libefl/basic_vector.hpp>

#include <libvisr/detail/compose_message_string.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace visr
{
namespace python
{
namespace bindinghelpers
{

/**
 * Alias name to discard extraneous default template parameters.
 * This workaround is to prevent problems if this type is used as a template template parameter.
 */
template<typename T>
using PyArray = pybind11::array_t<T>;

/**
 * Trait template to provide conversions from the supported numeric containers to raw pointers
 * (as required by the efl vector functions).
 * The base, unspecialized version is not implemented.
 * Specializations are required to implement two methods as static members:
 * - constantPointer to return a constant pointer to the array data
 * - mutablePointer to return a non-constant pointer to the array data
 */
template< template<typename> class ContainerType, typename T >
struct ContainerAccess;

template< typename T >
struct ContainerAccess< efl::BasicVector, T >
{
  /**
   * Retrieve a mutable (non-const) pointer to the underlying array data.
   * @param vec The container to work on
   * @param requestedSize The number of elements required in the array.
   * This is to check whether the container is large enought for the operation to be performed.
   * @param name Name of the container, to be used in error messages.
   */
  static T * const mutablePointer( efl::BasicVector<T> & vec, std::size_t requestedSize, char const * name )
  {
    if( vec.size() < requestedSize )
    {
      throw std::invalid_argument( "Vector function wrapper: Input array too small" );
    }
    T * const ptr = vec.data();
    return ptr;
  }

  static T const * const constantPointer( efl::BasicVector<T> const & vec, std::size_t requestedSize, char const * name )
  {
    if( vec.size() < requestedSize )
    {
      throw std::invalid_argument( "Vector function wrapper: Input array too small" );
    }
    T const * const ptr = vec.data();
    return ptr;
  }
};

template< typename T >
struct ContainerAccess< PyArray, T >
{
  static T * const mutablePointer( PyArray<T> & array, std::size_t requestedSize, char const * name )
  {
  pybind11::buffer_info const & info = array.request();
  if( info.ndim != 1)
  {
    throw std::invalid_argument( "Vector function wrapper: Input arrays must be 1D" );
  }
  if( info.strides[0] != sizeof(T) )
  {
    throw std::invalid_argument( "Vector function wrapper: Input arrays must be contiguous" );
  }
  if( info.shape[ 0 ] < static_cast< pybind11::ssize_t >( requestedSize ) )
  {
    throw std::invalid_argument( "Vector function wrapper: Input array too small" );
  }
  T * const ptr = array.mutable_data();
  return ptr;
  }

  static T const * const constantPointer( PyArray<T> const & array, std::size_t requestedSize, char const * name )
  {
  pybind11::buffer_info info = array.request();
  if( info.ndim != 1)
  {
    throw std::invalid_argument( "Vector function wrapper: Input arrays must be 1D" );
  }
  if( info.strides[0] != sizeof(T) )
  {
    throw std::invalid_argument( "Vector function wrapper: Input arrays must be contiguous" );
  }
  if( info.shape[ 0 ] < static_cast< pybind11::ssize_t >( requestedSize ) )
  {
    throw std::invalid_argument( "Vector function wrapper: Input array too small" );
  }
  T const * const ptr = array.data();
  return ptr;
  }
};

} // namespace bindinghelpers
} // namespace python
} // namespace visr

#endif // VISR_PYTHON_BINDING_HELPERS_CONTAINER_ACCESS_HPP_INCLUDED