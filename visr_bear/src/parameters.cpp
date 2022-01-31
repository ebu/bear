#include "parameters.hpp"

#include <libvisr/parameter_factory.hpp>

namespace bear {

void init_parameters()
{
  static visr::ParameterRegistrar<ADMParameterObjectsInput,
                                  ADMParameterDirectSpeakersInput,
                                  ADMParameterHOAInput,
                                  ListenerParameter>
      bear_parameter_registrar;
}
}  // namespace bear
