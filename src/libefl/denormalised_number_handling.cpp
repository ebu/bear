/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "denormalised_number_handling.hpp"

#if (defined VISR_SYSTEM_PROCESSOR_x86) or (defined VISR_SYSTEM_PROCESSOR_x86_64)
#define INTEL_PLATFORM // Convenience macro for use within this compilation unit
#include <xmmintrin.h>
#endif

#include <cstdint>

namespace visr
{
namespace efl
{

#ifdef INTEL_PLATFORM
// only for Intel X86/X86_64 platforms
namespace // unnamed
{
  using ControlWord = uint32_t;

  static const ControlWord cDenormsAreZeroMask = 1 << 6;

  static const ControlWord cFlushToZeroMask = 1 << 15;

  static const ControlWord cCwMask = ~(cDenormsAreZeroMask bitor cFlushToZeroMask);

} // unnamed namespace
#endif
  
DenormalisedNumbers::State DenormalisedNumbers::setDenormHandling( )
{
  return setDenormHandling( State( true, true ) );
}

void DenormalisedNumbers::resetDenormHandling( State stateToRestore )
{
  setDenormHandling( stateToRestore );
}

DenormalisedNumbers::State DenormalisedNumbers::setDenormHandling( State newState )
{
#ifdef INTEL_PLATFORM 
  ControlWord const oldCW = _mm_getcsr( );
  bool const oldDAZ = (oldCW bitand cDenormsAreZeroMask) != 0;
  bool const oldFTZ = (oldCW bitand cFlushToZeroMask) != 0;

  ControlWord newCW = oldCW bitand cCwMask;
  if( newState.denormalsAreZero )
  {
    newCW = newCW bitor cDenormsAreZeroMask;
  }
  if( newState.flushToZero )
  {
    newCW = newCW bitor cFlushToZeroMask;
  }
  _mm_setcsr( newCW );
  return State( oldFTZ, oldDAZ );
#else // #ifdef INTEL_PLATFORM
  // Do nothing, return default object.
  return State();
#endif
}

} // namespace efl
} // namespace visr
