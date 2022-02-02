/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PANNINGDSP_INITIALISE_PARAMETER_LIBRARY_HPP_INCLUDED
#define VISR_PANNINGDSP_INITIALISE_PARAMETER_LIBRARY_HPP_INCLUDED

#include "export_symbols.hpp"

namespace visr
{
namespace panningdsp
{

/**
 * Register all contained parameter types and communication protocols in the respective factories.
 */
VISR_PANNINGDSP_LIBRARY_SYMBOL void initialiseParameterLibrary();

} // namespace panningdsp
} // namespace visr

#endif // VISR_PANNINGDSP_INITIALISE_PARAMETER_LIBRARY_HPP_INCLUDED
