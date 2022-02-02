/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_PARAMETER_INPUT_TERMINATOR_HPP_INCLUDED
#define VISR_LIBRCL_PARAMETER_INPUT_TERMINATOR_HPP_INCLUDED

#include "export_symbols.hpp"

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
/**
 * Component for terminating unused parameter input ports, thus eliminating
 * 'unconnected parameter input port' connection errors.
 * @warning This class is agnostic of the semantics of the port's communication
 * protocol. That is, it will not not perform any operation expected from a
 * terminator of a specifc communication protocol, for example setting an initial value.
 * This might lead to unintended behaviour, for example deadlocks.
 */
class VISR_RCL_LIBRARY_SYMBOL ParameterInputTerminator: public AtomicComponent
{
public:
  /**
   * Constructor.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component.
   * @param parameterType The parameter type of the terminator's output port.
   * @param protocolType The communication protocol type of the terminator's
   * output port.
   * @param parameterConfig The parameter configuration (e.g., dimensions)
   * of the output port.
   */
  explicit ParameterInputTerminator(
      SignalFlowContext const & context,
      char const * name,
      CompositeComponent * parent,
      visr::ParameterType parameterType,
      visr::CommunicationProtocolType protocolType,
      visr::ParameterConfigBase const & parameterConfig );

  /**
   * Convenience constructor that aeccpts the output port to be terminated as an
   * argument in order to deduce the port type and parameters.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component.
   * @param templatePort A parameter port used to obtain the desired port
   * type (parameter type, protocol type, and parameter configuration).
   * @note This constructor does *not* connect the terminator to the template
   * port, this has to be done explicitly in the containing component.
   */
  explicit ParameterInputTerminator(
      SignalFlowContext const & context,
      char const * name,
      CompositeComponent * parent,
      visr::ParameterPortBase const & templatePort );

  /**
   * Destructor.
   */
  ~ParameterInputTerminator() override;

  /**
   * The process function.
   * Does nothing.
   */
  void process() override;

private:
  /**
   * The parameter output port of the terminator, named "out".
   */
  visr::PolymorphicParameterOutput mOutput;
};

} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_PARAMETER_INPUT_TERMINATOR_HPP_INCLUDED
