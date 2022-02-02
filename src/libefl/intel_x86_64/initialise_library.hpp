/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_INTEL_X86_64_INITIALISE_LIBRARY_HPP_INCLUDED
#define VISR_LIBEFL_INTEL_X86_64_INITIALISE_LIBRARY_HPP_INCLUDED

#include "../export_symbols.hpp"

namespace visr
{
namespace efl
{
namespace intel_x86_64
{

VISR_EFL_LIBRARY_SYMBOL bool initialiseLibrary( char const * processor = "" );

VISR_EFL_LIBRARY_SYMBOL bool uninitialiseLibrary();

} // namespace intel_x86_64
} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_INTEL_X86_64_INITIALISE_LIBRARY_HPP_INCLUDED
