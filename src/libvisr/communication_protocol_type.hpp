/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_COMMUNICATION_PROTOCOL_TYPE_HPP_INCLUDED
#define VISR_COMMUNICATION_PROTOCOL_TYPE_HPP_INCLUDED

#include "detail/compile_time_hash_fnv1.hpp"

#include <string>
#include <cstddef>
#include <cstdint>

namespace visr
{

using CommunicationProtocolType = std::uint64_t;

constexpr CommunicationProtocolType communicationProtocolTypeFromString( char const * typeString )
{
  return detail::compileTimeHashFNV1( typeString );
}


/**
 * Type trait (metaprogramming construct) for compile-time translation between a type and its corresponding ID.
 * Must be specialized by derived parameter types, with the specialized template definining an unnamed enum with a value 'id'
 * holding the type id of this type
 * For convenience the macro DEFINE_COMMUNICATION_PROTOCOL should be used.
 */
template<class CommunicationProtocolClass >
struct CommunicationProtocolToId {};

/**
 * Type trait (metaprogramming construct) for compile-time translation between a type id and the corresponding C++ type.
 * Must be specialized by derived parameter types.
 * his template also stores the registered name of the type.
 * For convenience the macro DEFINE_COMMUNICATION_PROTOCOL should be used.
 */
template< CommunicationProtocolType id>
struct IdToCommunicationProtocol {};

} // namespace visr

/**
 */
#define DEFINE_COMMUNICATION_PROTOCOL( CommunicationProtocolClassType, CommunicationProtocolId, CommunicationProtocolName )\
namespace visr \
{ \
   template<>\
   struct CommunicationProtocolToId< CommunicationProtocolClassType > \
    { \
    public: \
      enum : CommunicationProtocolType { id = CommunicationProtocolId };\
      static constexpr const char * name = CommunicationProtocolName; \
    }; \
}

#endif // #ifndef VISR_COMMUNICATION_PROTOCOL_TYPE_HPP_INCLUDED
