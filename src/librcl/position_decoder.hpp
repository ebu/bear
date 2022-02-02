/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_POSITION_DECODER_HPP_INCLUDED
#define VISR_LIBRCL_POSITION_DECODER_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>

#include <libpml/listener_position.hpp>
#include <libpml/message_queue_protocol.hpp>
#include <libpml/double_buffering_protocol.hpp>
#include <libpml/string_parameter.hpp>

#include <libpanning/XYZ.h>

namespace visr
{
namespace rcl
{

/**
 * Component to decode listener position data from messages (typically received from a network).
 * This component has neither audio inputs or outputs.
 */
class VISR_RCL_LIBRARY_SYMBOL PositionDecoder: public AtomicComponent
{
public:
  /**
   * Constructor.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p nullptr in case of a top-level component.
   * @param positionOffset Offset position of the tracking coordinate system relative to the reference corrdinate system.
   * Optional parameter, defaults to (0,0,0).
   * @param orientationRotation Orientation of the tracking coordinate system relative to the reference (renderer) coordinate system.
   * The quaternion describes the rotation from the reference to the tracking coordinate system.
   */
  explicit PositionDecoder( SignalFlowContext const & context,
                            char const * name,
                            CompositeComponent * parent,
                            pml::ListenerPosition::PositionType const & positionOffset
                              = pml::ListenerPosition::PositionType(),
                            pml::ListenerPosition::OrientationQuaternion const & orientationRotation
                              = pml::ListenerPosition::OrientationQuaternion{1.0f, 0.0f, 0.0f, 0.0f} );

  explicit PositionDecoder( SignalFlowContext const & context,
                            char const * name,
                            CompositeComponent * parent,
                            pml:: ListenerPosition::PositionType const & positionOffset,
                            pml:: ListenerPosition::OrientationYPR const & orientationRotation );

  /**
   * Disabled (deleted) copy constructor
   */
  PositionDecoder( PositionDecoder const & ) = delete;

  /**
   * Destructor.
   */
  ~PositionDecoder();

  /**
   * The process function. Decodes all messages arriving through the "messageInput" parameter input port and sends
   * them through the parameter output "positionOutput".
   */
  void process() override;

private:
  pml::ListenerPosition translatePosition(const pml::ListenerPosition &pos);

  ParameterInput< pml::MessageQueueProtocol, pml::StringParameter > mDatagramInput;
  ParameterOutput< pml::DoubleBufferingProtocol, pml::ListenerPosition > mPositionOutput;

  /**
   * Offset position of the tracking coordinate system relative to the reference coordinate system.
   */
  pml::ListenerPosition::PositionType const cOffsetPosition;

  /**
   * Rotation quaternnion describing the rotation from the reference (renderer) coordinate system to the tracker coordinate system.
   */
  pml::ListenerPosition::OrientationQuaternion const cOrientationRotation;
};

} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_POSITION_DECODER_HPP_INCLUDED
