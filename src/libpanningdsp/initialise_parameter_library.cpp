/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "initialise_parameter_library.hpp"

#include "panning_matrix_parameter.hpp"

#include <libvisr/parameter_factory.hpp>

namespace visr
{
namespace panningdsp
{

void initialiseParameterLibrary()
{
  static ParameterRegistrar<
    PanningMatrixParameter
  >
 sParameterRegistrar;
}

} // namespace panningdsp
} // namespace visr
