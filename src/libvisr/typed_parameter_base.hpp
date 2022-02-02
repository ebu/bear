/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_TYPED_PARAMETER_BASE_HPP_INCLUDED
#define VISR_TYPED_PARAMETER_BASE_HPP_INCLUDED

#include "parameter_base.hpp"
#include "parameter_type.hpp"

#include <ciso646>
#include <exception>

namespace visr
{

/**
 * Base class template for parameter data with an associated parameter type id and configuration data type.
 * This models the Curiously Recurring template pattern to provide methods as type(), clone() or a virtual assignment method 
 * in all template instantiations.
 */
template< typename ConcreteParameterType,
          class ParameterConfigT,
          ParameterType typeId >
class TypedParameterBase: public ParameterBase
{
public:
  using ParameterConfigType = ParameterConfigT;

  /**
   * Default constructor
   */
  TypedParameterBase() = default;

  /**
   * Destructor (virtual)
   */
  virtual ~TypedParameterBase() = default;

  /**
   * Return the static type of the parameter type (not the dynamic type associated with an instance)
   */
  static const constexpr ParameterType staticType()
  {
    return typeId;
  }

  /**
   * Return the dynamic parameter type of the object.
   */
  /*virtual*/ ParameterType type() final
  {
    return staticType();
  }

  /**
   * Clone method, i.e., virtual constructor.
   */
  /*virtual*/ std::unique_ptr<ParameterBase> clone() const final
  {
    return std::unique_ptr<ParameterBase>(
      new ConcreteParameterType(static_cast<ConcreteParameterType const &>(*this) ) );
  }

  /**
   * Virtual assignment method.
   * @param rhs The object to be copied.
   * @throw std::invalid_argument if the type of rhs does not match this object's type.
   */
  void assign( ParameterBase const & rhs ) override
  {
    ConcreteParameterType const * rhsTyped = dynamic_cast<ConcreteParameterType const *>(&rhs);
    if( not rhsTyped )
    {
      throw std::invalid_argument( "Assign: Types are not compatible.");
    }
    static_cast<ConcreteParameterType&>(*this).operator=( *rhsTyped );
  }
};

} // namespace visr

// TODO: Check whether we can provide the lookup template specializations
// ParameterToId, IdToParameter, and ParameterToConfigType for all types here.
// Problem: We do not see the derived type here, only the base of the actual parameter type.

#endif // #ifndef VISR_TYPED_PARAMETER_BASE_HPP_INCLUDED
