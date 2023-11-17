/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librbbl/biquad_coefficient.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <array>
#include <string>
#include <sstream>
#include <vector>

namespace visr
{
namespace rbbl
{
namespace python
{

namespace py = pybind11;

namespace 
{
  template< typename CoeffType >
  void exportBiquadCoefficient( pybind11::module & m, char const * className )
  {
    py::class_<BiquadCoefficient<CoeffType> >( m, className )
      .def( py::init<>(), "Default constructor" )
      .def( py::init<BiquadCoefficient<CoeffType> const & >(), py::arg( "rhs" ) )
      .def( py::init(
        []( std::array<CoeffType, BiquadCoefficient<CoeffType>::cNumberOfCoeffs> const & val )
        {
          return new BiquadCoefficient<CoeffType>( val[0], val[1], val[2], val[3], val[4] );
        }), py::arg( "val" ) )
      .def_property_readonly_static( "numberOfCoefficients", []( py::object /*self*/ )
      {
        return BiquadCoefficient<CoeffType>::cNumberOfCoeffs;
      } )
      .def( py::init<CoeffType, CoeffType, CoeffType, CoeffType, CoeffType >(), 
        py::arg( "b0" ), py::arg( "b1" ), py::arg( "b2" ), py::arg( "a1" ), py::arg( "a2" ) )
      .def( py::init< std::initializer_list< CoeffType > const & >(), py::arg("coeffs") )
      //BiquadCoefficient & operator=( BiquadCoefficient const & rhs )
      .def_property( "data",
        []( BiquadCoefficient<CoeffType> const & self)
        {
          return std::vector<CoeffType>( self.data(), self.data() + BiquadCoefficient<CoeffType>::cNumberOfCoeffs); 
        },
        []( BiquadCoefficient<CoeffType> & self, std::array<CoeffType, BiquadCoefficient<CoeffType>::cNumberOfCoeffs> const val )
        {
          for( std::size_t idx(0); idx < BiquadCoefficient<CoeffType>::cNumberOfCoeffs ; ++idx )
          {
            self[idx] = val[idx];
          }
        } )
      .def_property( "b0", static_cast<CoeffType const &(BiquadCoefficient<CoeffType>::*)() const >(&BiquadCoefficient<CoeffType>::b0),
        []( BiquadCoefficient<CoeffType> & self, CoeffType val){ self.b0() = val; } )
      .def_property( "b1", static_cast<CoeffType const &(BiquadCoefficient<CoeffType>::*)() const >(&BiquadCoefficient<CoeffType>::b1),
        []( BiquadCoefficient<CoeffType> & self, CoeffType val ) { self.b1() = val; } )
      .def_property( "b2", static_cast<CoeffType const &(BiquadCoefficient<CoeffType>::*)() const >(&BiquadCoefficient<CoeffType>::b2),
        []( BiquadCoefficient<CoeffType> & self, CoeffType val ) { self.b2() = val; } )
       .def_property( "a1", static_cast<CoeffType const &(BiquadCoefficient<CoeffType>::*)() const >(&BiquadCoefficient<CoeffType>::a1),
        []( BiquadCoefficient<CoeffType> & self, CoeffType val ) { self.a1() = val; } )
      .def_property( "a2", static_cast<CoeffType const &(BiquadCoefficient<CoeffType>::*)() const >(&BiquadCoefficient<CoeffType>::a2),
        []( BiquadCoefficient<CoeffType> & self, CoeffType val ) { self.a2() = val; } )
      .def( "loadJson", []( BiquadCoefficient<CoeffType> & self, std::string const & jsonRep )
      {
        std::stringstream stream( jsonRep );
        self.loadJson( stream );
      } )
      .def_static( "fromJson", []( std::string const & str ){ return BiquadCoefficient<CoeffType>::fromJson( str ); } )
      .def_static( "fromXml", []( std::string const & str ){ return BiquadCoefficient<CoeffType>::fromXml( str ); } )
      .def( "loadXml", []( BiquadCoefficient<CoeffType> & self, std::string const & xmlRep )
      {
        std::stringstream stream( xmlRep );
        self.loadXml( stream );
      } )
      .def( "writeJson", []( BiquadCoefficient<CoeffType> & self ){ std::string jsonRep; self.writeJson( jsonRep ); return jsonRep; } )
      .def( "writeXml", []( BiquadCoefficient<CoeffType> & self ) { std::string xmlRep; self.writeXml( xmlRep ); return xmlRep; } )
      .def( "__str__",
              []( BiquadCoefficient< CoeffType > & self )
              {
                std::stringstream str;
                str << "b0: " << self.b0() << " b1: " << self.b1()
                    << " b2: " << self.b2() << " a1: " << self.a1()
                    << " a2: " << self.a2();
                return str.str();
              } )
      ;
  }

  template< typename CoeffType >
  void exportBiquadCoefficientList( pybind11::module & m, char const * className )
  {
    py::class_<BiquadCoefficientList<CoeffType> >( m, className )
      .def( py::init<>(), "Default constructor" )
      .def( py::init<std::size_t>(), py::arg( "initialSize" ), "Constructor with initial size and default elements" )
      .def( py::init< BiquadCoefficientList<CoeffType> const &>(), py::arg( "rhs" ), "Copy constructor" )
      .def( py::init< std::initializer_list<BiquadCoefficient<CoeffType> > const & >(), py::arg("initList" ), "Constructor from initialiser list" )
      .def( py::init(
        []( std::vector< BiquadCoefficient<CoeffType> > const & vec )
        {
          BiquadCoefficientList<CoeffType>* inst = new BiquadCoefficientList<CoeffType>( vec.size() );
          for( std::size_t idx(0); idx < vec.size(); ++idx )
          {
            inst->at(idx) = vec[idx];
          }
          return inst;
        } ), py::arg( "initList" ), "Constructor from list of biquads" )
      .def_static( "fromJson", []( std::string const & str ){ return BiquadCoefficientList<CoeffType>::fromJson( str ); } )
      .def_static( "fromXml", []( std::string const & str ){ return BiquadCoefficientList<CoeffType>::fromXml( str ); } )
      .def_property( "size", &BiquadCoefficientList<CoeffType>::size, &BiquadCoefficientList<CoeffType>::size )
      .def( "resize", &BiquadCoefficientList<CoeffType>::size )
      .def( "__len__", &BiquadCoefficientList<CoeffType>::size )
      .def( "__setitem__", []( BiquadCoefficientList<CoeffType> & self, std::size_t idx, BiquadCoefficient<CoeffType> const & val ){ self.at(idx) = val; } )
      .def( "__getitem__", static_cast<BiquadCoefficient<CoeffType> const &(BiquadCoefficientList<CoeffType>::*)(std::size_t)const>(&BiquadCoefficientList<CoeffType>::at ) )
      .def( "__iter__", []( BiquadCoefficientList<CoeffType> & lst ){ return py::make_iterator( lst.begin(), lst.end()); } ) 
      .def( "loadJson", static_cast< void( BiquadCoefficientList<CoeffType>::*)(std::string const &)>(&BiquadCoefficientList<CoeffType>::loadJson ) )
      .def( "loadXml", static_cast< void(BiquadCoefficientList<CoeffType>::*)(std::string const &)>(&BiquadCoefficientList<CoeffType>::loadJson) )
      .def( "writeJson", []( BiquadCoefficientList<CoeffType> const & self ) { std::string str; self.writeJson( str ); return self; } )
      .def( "writeXml", [](BiquadCoefficientList<CoeffType> const & self ){ std::string str; self.writeXml( str ); return self; } )
      ;
  }

  template< typename CoeffType >
  void exportBiquadCoefficientMatrix( pybind11::module & m, char const * className )
  {
    py::class_<BiquadCoefficientMatrix<CoeffType> >( m, className )
     .def( py::init<std::size_t, std::size_t>(), py::arg("numberOfFilters"), py::arg("numberOfBiquads") )
     .def_static( "fromJson", []( std::string const & str ){ return BiquadCoefficientMatrix<CoeffType>::fromJson( str ); } )
     .def_static( "fromXml", []( std::string const & str ){ return BiquadCoefficientMatrix<CoeffType>::fromXml( str ); } )
     .def_property_readonly( "numberOfFilters", &BiquadCoefficientMatrix<CoeffType>::numberOfFilters )
     .def_property_readonly( "numberOfSections", &BiquadCoefficientMatrix<CoeffType>::numberOfSections )
     .def( "resize", &BiquadCoefficientMatrix<CoeffType>::resize, py::arg("numberOfFilters"), py::arg( "numberOfBiquads" ) )
     .def( "__setitem__", []( BiquadCoefficientMatrix<CoeffType> & self, std::size_t idx, BiquadCoefficientList<CoeffType> const & val ){ self[idx] = val; } )
     .def( "__getitem__", static_cast<BiquadCoefficientList<CoeffType> const &(BiquadCoefficientMatrix<CoeffType>::*)( std::size_t) const >
      (&BiquadCoefficientMatrix<CoeffType>::operator[] ) )
      .def( "__setitem__", []( BiquadCoefficientMatrix<CoeffType> & self, pybind11::tuple idx, BiquadCoefficient<CoeffType> const & val )
        { self( idx[0].cast<std::size_t>(), idx[1].cast<std::size_t>() ) = val; } )
     .def( "__getitem__", []( BiquadCoefficientMatrix<CoeffType> & self, pybind11::tuple idx ) { return self( idx[0].cast<std::size_t>(), idx[1].cast<std::size_t>() ); }, pybind11::arg( "index" ) )
     .def( "setFilter", &BiquadCoefficientMatrix<CoeffType>::setFilter, py::arg( "filterIdx" ), py::arg( "newFilters" ) )
     .def( "loadJson", static_cast< void( BiquadCoefficientMatrix<CoeffType>::*)(std::string const &)>(&BiquadCoefficientMatrix<CoeffType>::loadJson ) )
     .def( "loadXml", static_cast< void(BiquadCoefficientMatrix<CoeffType>::*)(std::string const &)>(&BiquadCoefficientMatrix<CoeffType>::loadJson) )
     .def( "writeJson", []( BiquadCoefficientMatrix<CoeffType> const & self ) { std::string str; self.writeJson( str ); return self; } )
     .def( "writeXml", [](BiquadCoefficientMatrix<CoeffType> const & self ){ std::string str; self.writeXml( str ); return self; } )
    ;
  }

} // unnamed namespace

void exportBiquadCoefficients( pybind11::module & m )
{
  exportBiquadCoefficient<float>( m, "BiquadCoefficientFloat" );
  exportBiquadCoefficient<double>( m, "BiquadCoefficientDouble" );

  exportBiquadCoefficientList<float>( m, "BiquadCoefficientListFloat" );
  exportBiquadCoefficientList<double>( m, "BiquadCoefficientListDouble" );

  exportBiquadCoefficientMatrix<float>( m, "BiquadCoefficientMatrixFloat" );
  exportBiquadCoefficientMatrix<double>( m, "BiquadCoefficientMatrixDouble" );
}

} // namespace python
} // namepace rbbl
} // namespace visr
