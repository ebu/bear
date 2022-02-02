/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libefl/basic_vector.hpp> 

#include <libvisr/constants.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <complex>

namespace visr
{
namespace efl
{
namespace python
{

namespace py = pybind11;

namespace // unnamed
{

template<typename DataType>
void setData( BasicVector<DataType> & param, py::array_t<DataType> & vec )
{
  py::buffer_info info = vec.request();
  std::size_t const numElements = param.size();
  if( (info.ndim != 1) or (info.shape[0] != static_cast<py::ssize_t>(numElements)) )
  {
    throw std::invalid_argument( "VectorParameter.set(): array length does not match." );
  }
  // Perform copying
  // TODO: Replace by optimised implementation
  std::size_t const stride = info.strides[0] / sizeof( DataType );
  DataType const * srcPtr = vec.data();
  DataType * destPtr = param.data();
  for( std::size_t elIdx( 0 ); elIdx < numElements; ++elIdx, ++destPtr, srcPtr += stride )
  {
    *destPtr = *srcPtr;
  }
}

template<typename DataType>
void exportBasicVector( py::module & m, char const * className )
{
  py::class_<BasicVector< DataType> >(m, className, py::buffer_protocol() )
  .def_buffer([](BasicVector<DataType> &vp) -> py::buffer_info
  {
    return py::buffer_info( vp.data(),
     sizeof( DataType ),
     py::format_descriptor<DataType>::format(),
     1, { vp.size() }, { sizeof( DataType ) } );
  } )

  // Note: Clang compilers throw a 'narrowing conversion" error when following two constructors are bound with the py::init<>() method. Therefore we use the explicit form.
  //.def( py::init<std::size_t>() )
  //.def( py::init<std::size_t, std::size_t>() )
  .def( py::init( []( std::size_t alignment ) { return new BasicVector<DataType>( alignment ); }), py::arg("alignment") = visr::cVectorAlignmentSamples )
  .def( py::init( [](std::size_t size, std::size_t alignment ) { return new BasicVector<DataType>( size, alignment ); }), py::arg( "size" ), py::arg("alignment") = visr::cVectorAlignmentSamples )
  .def( py::init( [](py::array_t<DataType> const & data, std::size_t alignment)
  {
    if( data.ndim() != 1 )
    {
      throw std::invalid_argument( "BasicVector from numpy ndarray: Input array must be 1D" );
    }
    std::size_t const numElements = data.shape()[0];
    BasicVector<DataType> * inst = new BasicVector<DataType>( numElements, alignment);
    for( std::size_t elIdx(0); elIdx < numElements; ++elIdx )
    {
      inst->at(elIdx) = *static_cast<DataType const *>(data.data( elIdx ));
    }
    return inst;
  }), py::arg("data"), py::arg("alignment") = visr::cVectorAlignmentSamples, "Construct a BasicVector from a 1D Numpy array." )
  .def( py::init( []( std::vector<DataType> const & v, std::size_t alignment )
        {
          std::size_t const sz = v.size();
          BasicVector<DataType> * inst = new BasicVector<DataType>( sz, alignment );
          std::copy( v.begin(), v.end(), inst->data() );
          return inst;
        } ), py::arg( "values" ), py::arg( "alignment" ) = visr::cVectorAlignmentSamples, "Create a BasicVector from a Python list." )
    .def_property_readonly( "size", &BasicVector<DataType>::size )
    .def_property_readonly( "alignment", &BasicVector<DataType>::alignmentElements, "Return the alignment of the numeric data (in number of elements)." )
  .def( "resize", []( BasicVector<DataType> & vp, std::size_t newSize ) { vp.resize(newSize); }, py::arg( "numberOfElements" ) )
  .def( "zeroFill", []( BasicVector<DataType> & vp ) { vp.zeroFill(); } )
  .def( "set", &setData<DataType> )
  .def( "__getitem__", []( BasicVector<DataType> const & vec, std::size_t idx ) { return vec.at( idx ); }, py::arg( "index" ) )
  .def( "__setitem__", []( BasicVector<DataType> & vec, std::size_t idx, DataType val ) { vec.at( idx ) = val; }, py::arg( "index" ), py::arg( "value" ) )
  .def( "__iter__", []( BasicVector<DataType> & vec ){ return py::make_iterator(vec.data(), vec.data()+vec.size() ); }, "Return an iterator pair to the contents of the vector" )
  ;
}

} // namespace unnamed

void exportBasicVectors( py::module & m)
{
  exportBasicVector<float>( m, "BasicVectorFloat" );
  exportBasicVector<double>( m, "BasicVectorDouble" );
  exportBasicVector<std::complex<float> >( m, "BasicVectorComplexFloat" );
  exportBasicVector<std::complex<double> >( m, "BasicVectorComplexDouble" );
}

} // namespace python
} // namespace efl
} // namespace visr
