/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "parameter_input_terminator.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/communication_protocol_type.hpp>
#include <libvisr/parameter_config_base.hpp>
#include <libvisr/parameter_port_base.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/polymorphic_parameter_output.hpp>

namespace visr
{
namespace rcl
{

ParameterInputTerminator::ParameterInputTerminator(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    visr::ParameterType parameterType,
    visr::CommunicationProtocolType protocolType,
    visr::ParameterConfigBase const & parameterConfig )
 : visr::AtomicComponent( context, name, parent )
 , mOutput( "out", *this, parameterType, protocolType, parameterConfig )
{
}

ParameterInputTerminator::
ParameterInputTerminator( SignalFlowContext const & context,
                                      char const * name,
                                      CompositeComponent * parent,
                                      visr::ParameterPortBase const & templatePort )
 : ParameterInputTerminator( context, name, parent,
 templatePort.parameterType(),
 templatePort.protocolType(),
 templatePort.parameterConfig() )
{
}

ParameterInputTerminator::~ParameterInputTerminator() = default;

void ParameterInputTerminator::process()
{
  // Do nothing.
  // TODO: Decide whether to implement a default behaviour for known
  // protocol types, e.g., setting initial values.
}

} // namespace rcl
} // namespace visr
