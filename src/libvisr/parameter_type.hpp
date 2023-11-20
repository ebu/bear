/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PARAMETER_TYPE_HPP_INCLUDED
#define VISR_PARAMETER_TYPE_HPP_INCLUDED

#include "detail/compile_time_hash_fnv1.hpp"

#include <cstddef>
#include <cstdint>

namespace visr
{

using ParameterType = uint64_t;

/**
 * Type trait to translate a type to its corresponding ID and the
 * config type.
 * Specializations must provide an unnamed enum that contains an
 * enumerator 'id' initialised to the type id.
 * In addition, it must define a type alias "Type" to the config type of this
 * parameter type.
 * (using ConfigType = XXX)
 * @note the enum trick is to avoid undefined references.
 * @see macro DEFINE_PARAMETER_TYPE
 */
template< typename Parameter >
struct ParameterToId {};

/**
 * Type trait for compile-time translation between parameter type id
 * types and the corresponding class type.
 * Template specializations must define a type alias "Type" to the
 * class, e.g., "using Type = XXX
 * @see macro DEFINE_PARAMETER_TYPE
 */ 
template< ParameterType Id >
struct IdToParameter;

/**
 * Compile-time translation from parameters to the corresponding parameter config type
 */
template< typename Parameter >
struct ParameterToConfigType {};

} // namespace visr

/**
 * Macro to register compile-time translations between the parameter
 * class type, the corresponding type id and the type of the
 * configuration data type.
 * @param ParameterClassType The type of the parameter class
 * @param ParameterId Numeric parameter id (hash value)
 * @param ParameterConfigType The clann type of the configuration
 * type.
 * @note The translation is compile-time, so it must be visible
 * (included) at the point of usage.
 * @note This registration is independent of the run-time parameter
 * lookup and instantiation performed in ParameterFactory.
 */
#define DEFINE_PARAMETER_TYPE( ParameterClassType, ParameterId, ParameterConfigType)\
namespace visr { \
template<> \
struct ParameterToId< ParameterClassType > \
{\
public:\
  enum : ParameterType { id =  ParameterId };\
  using ConfigType = ParameterConfigType; \
};\
  template<> \
struct IdToParameter< ParameterId > \
{ \
public: \
  using Type = ParameterClassType; \
}; \
  template<> \
struct ParameterToConfigType< ParameterClassType > \
{\
public:\
  using ConfigType = ParameterConfigType;\
}; \
}

#endif // #ifndef VISR_PARAMETER_TYPE_HPP_INCLUDED
