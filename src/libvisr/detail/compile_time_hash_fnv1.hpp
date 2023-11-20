/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_DETAIL_COMPILE_TIME_HASH_FNV1_HPP_INCLUDED
#define VISR_DETAIL_COMPILE_TIME_HASH_FNV1_HPP_INCLUDED

#include <string>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace visr
{
namespace detail
{
  // Forward declaration of internal implementation function.
  constexpr uint64_t fnv1( uint64_t h, const char* s );

  /**
  * Compute a hash value (64bit unsigned int) from a
  * string.
  * This function is 'constexpr, i.e., can be used at compile time.
  * Adapted from https://github.com/elbeno/constexpr
  * Licence of original code: MIT
  * @param s String to be hashed. Must be a compile-time literal to enable use as a constexpr function.
  */
  constexpr uint64_t compileTimeHashFNV1( const char* s )
  {
    return true ?
      detail::fnv1( 14695981039346656037ull, s ) :
      throw std::logic_error("FNV1 hash failed.");
  }

  /**
   * Internal implementation function
   */
  constexpr uint64_t fnv1( uint64_t h, const char* s )
  {
// Disable "integral constant overflow" warning in Microsoft Visual C++ compiler.
#ifdef _MSC_VER
//#pragma warning( push )
#pragma warning( disable : 4307)
#endif
    return (*s == 0) ? h :
      fnv1( static_cast<uint64_t>(h * 1099511628211ull) ^
        *s, s + 1 );
#ifdef _MSC_VER
//#pragma warning( pop ) 
#endif
  }


} // namespace detail
} // namespace visr

#endif // #ifndef VISR_DETAIL_COMPILE_TIME_HASH_FNV1_HPP_INCLUDED
