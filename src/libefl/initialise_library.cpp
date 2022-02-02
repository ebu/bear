/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "initialise_library.hpp"

#ifdef VISR_SYSTEM_PROCESSOR_x86_64
#include "intel_x86_64/initialise_library.hpp"
#endif

#ifdef VISR_SYSTEM_PROCESSOR_armv7l
#include "armv7l_neon_32bit/initialise_library.hpp"
#endif

namespace visr
{
namespace efl
{

bool initialiseLibrary( char const * processor /*= ""*/ )
{
#ifdef VISR_SYSTEM_PROCESSOR_x86_64
  return intel_x86_64::initialiseLibrary( processor );
#endif
#ifdef VISR_SYSTEM_PROCESSOR_armv7l
  return armv7l_neon_32bit::initialiseLibrary( processor );
#endif
  return true;
}

bool uninitialiseLibrary()
{
#ifdef VISR_SYSTEM_PROCESSOR_x86_64
  return intel_x86_64::uninitialiseLibrary();
#endif
#ifdef VISR_SYSTEM_PROCESSOR_armv7l
  return armv7l_neon_32bit::uninitialiseLibrary();
#endif
  return true;
}

} // namespace efl
} // namespace visr

