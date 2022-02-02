/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libvisr/parameter_base.hpp>
#include <libvisr/parameter_type.hpp>


#include <pybind11/pybind11.h>

#include <ciso646>
#include <iostream> // For debugging purposes only.

namespace visr
{
namespace python
{
namespace visr
{

/**
 * Wrapper class to get access to the full functionality
 * Apparently nor required anymore (and is troublesome when deducing the argument
 * type).
 */
class ParameterBaseWrapper: public ParameterBase
{
public:
  /**
   * Use base class constructors
   */
  using ParameterBase::ParameterBase;

  virtual void assign(  ParameterBase const & rhs ) override
  {
    PYBIND11_OVERLOAD_PURE( void, ParameterBase, assign, rhs );
  }

  // TODO: Fix this overload. This is possibly related to the problems when returning a smart pointer from a pybind11 method.
#if 0
  virtual std::unique_ptr<ParameterBase> clone() const override
  {
    PYBIND11_OVERLOAD_PURE( std::unique_ptr<ParameterBase>, ParameterBase, clone, );
  }
#endif
};

void exportParameterBase( pybind11::module& m )
{
  pybind11::class_<ParameterBase, ParameterBaseWrapper>( m, "ParameterBase" )
    .def( "type", &ParameterBase::type )
    .def( "assign", &ParameterBase::assign, pybind11::arg( "rhs" ) )
    .def( "clone", &ParameterBase::clone )
    .def( "__deepcopy__",
    []( ParameterBase const & self, pybind11::dict & memo )
    {
      return self.clone();
    } )
    .def( "__copy__", &ParameterBase::clone )
    ;
}

} // namepace visr
} // namespace python
} // namespace visr

