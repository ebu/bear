/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_ARMV7L_NEON_32BIT_INITIALISE_LIBRARY_HPP_INCLUDED
#define VISR_LIBEFL_ARMV7L_NEON_32BIT_INITIALISE_LIBRARY_HPP_INCLUDED

#include "../export_symbols.hpp"

namespace visr
{
namespace efl
{
namespace armv7l_neon_32bit
{

VISR_EFL_LIBRARY_SYMBOL bool initialiseLibrary( char const * processor = "" );

VISR_EFL_LIBRARY_SYMBOL bool uninitialiseLibrary();

} // namespace armv7l_neon_32bit
} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_ARMV7L_NEON_32BIT_INITIALISE_LIBRARY_HPP_INCLUDED
