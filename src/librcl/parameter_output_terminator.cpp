/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "parameter_output_terminator.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/communication_protocol_type.hpp>
#include <libvisr/parameter_config_base.hpp>
#include <libvisr/parameter_port_base.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/polymorphic_parameter_input.hpp>

namespace visr
{
namespace rcl
{
ParameterOutputTerminator::ParameterOutputTerminator(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    visr::ParameterType parameterType,
    visr::CommunicationProtocolType protocolType,
    visr::ParameterConfigBase const & parameterConfig )
 : visr::AtomicComponent( context, name, parent )
 , mInput( "in", *this, parameterType, protocolType, parameterConfig )
{
}

ParameterOutputTerminator::ParameterOutputTerminator(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    visr::ParameterPortBase const & templatePort )
 : ParameterOutputTerminator( context,
                              name,
                              parent,
                              templatePort.parameterType(),
                              templatePort.protocolType(),
                              templatePort.parameterConfig() )
{
}

ParameterOutputTerminator::~ParameterOutputTerminator() = default;

void ParameterOutputTerminator::process()
{
  // Do nothing.
  // TODO: Decide whether to implement a default behaviour for known
  // protocol types, e.g., emptying the input in case of MessageQueueProtocol.
}

} // namespace rcl
} // namespace visr
