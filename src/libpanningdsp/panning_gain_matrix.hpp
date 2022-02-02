/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBPANNINGDSP_PANNING_GAIN_MATRIX_HPP_INCLUDED
#define VISR_LIBPANNINGDSP_PANNING_GAIN_MATRIX_HPP_INCLUDED

#include "export_symbols.hpp"
#include "panning_matrix_parameter.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/parameter_input.hpp>

#include <libpml/double_buffering_protocol.hpp>

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>

#include <cstddef>
#include <memory>
#include <vector>

namespace visr
{
namespace panningdsp
{
/**
 * Audio component for applying panning gains to a multichannel input signal
 * (where each channel represents a monaural audio object) to a multichannel
 * output (where output channels represent loudspeaker signals).
 *
 * The component has one audio input (named "in") with width equal to the number
 * of audio objects, one audio output (named "out", widths equals the number of
 * loudspeakers), and a parameter input port (name "gainInput",  parameter type
 * panningdsp::PanningParameterMatrix, protocol pml.DoubleBufferingProtocol).
 *
 * Compared to the generic rcl.GainMatrix, this component has the following
 * differences:
 * - It accepts panningdsp.PanningMatrixParameter messages instead of
 * pml.MatrixParameter objects.
 * - It uses the DoubleBufferingProtocol instead of SharedData, that is, gain
 * data do not need to be updated constantly.
 * - Gain changes (termed 'transitions') can start at arbitrary time instants
 * (to sample accuracy) and can have arbitrary duration times (also down to
 * sample accuracy).
 * - This also enables intermediate gain changes (for example to implement the
 * 'jumpPosition' feature of ADM) by setting a transition time of zero.
 * * Transitions can be specified per object. This is achieved by using a
 * special value in the transition start time container of
 * PanningMatrixParameter messages. If this value is set for a specific object,
 * this is interpreted as 'no change'.
 * .
 * The semantics of panning parameter updates / transitions are as follows:
 * - All transitions are on a per-object basis and do not influence transitions
 * of objects. That is, in a PanningMatrixParameter, the start and transition
 * times for each object are independent, and an object is not affected if is
 * maked as inactive in a PanningMatrixParameter message.
 * - At every time instant, each object performs a linear transition between a
 * past and a target gain for each loudspeaker. In case the gain shall remain
 * constant, the previous and target gains are identical.
 * - In addition, each objects stores one transaction per object, consisting of
 * a start time, a transition time, and an end gain value. These future
 * transitions are communicated to the component through PanningMatrixParameter
 * objects, or passed at startup in the constructor overload that accepts a
 * PanningMatrixParameter object.
 * - When a PanningMatrixParameter message is received in the component (which
 * is checked at the beginning of an audio buffer), the start time of every
 * transaction in the parameter must be equal or greater than the current time.
 * Otherwise an error status is reported and the transition is ignored.
 * - Apart from this causality requirement, transistions can be communicated
 * with start times arbitrarily far in the future.
 * - When another transition is received while there is already stored
 * transition for this object, the previous one is overwritten (regardless of
 * time stamps etc.).
 * - When the target time of the present transition is reached, and there is no
 * stored transition for this object, the gain remains constant (indefinitely,
 * or if a new transition is received).
 * - If the start time of the stored (next) transition transition is reached, it
 * becomes the current transition.
 *   + If there is a current gain change in progress the new `previous gain`
 * value is determined by linearly interpolating between the former previous and
 * target gain, evaluated at the start time of the new transition.
 *   + The end time of the present transition is set to its start time plus its
 * duration. The target gain is set to the gain values contained in the
 *   + If duration of the new transaction is zero, the new target gain is set
 * immediately, and the transaction is completed immediately. This implies the
 * used gain values remains constant afterwards (becuse there is no pending
 * transition).
 * .
 * 
 * @note A future extension might allow for multiple transitions to be store for
 * each object. This would need a logic how to change the sequence of
 * transitions if the new one would alter existing ones.
 * @note Because the used DoubleBufferingProtocol allows at most one message per
 * audio processing buffer, this limits the number of gain changes that can be
 * transmitted. It also requires more careful scheduling when messages are sent
 * in order to achieve adjoining transitions. It would be possible to work
 * around that by (optionally) allowing for a MessageQueueProtocol
 * communication, but this would involve more run-time memory allocation.
 */
class VISR_PANNINGDSP_LIBRARY_SYMBOL PanningGainMatrix: public AtomicComponent
{
public:
  using SampleType = visr::SampleType;

  /**
   * Constructor.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component
   * @param numberOfObjects The number of audio objects to be panned, determines
   * the width of the audio input port.
   * @param numberOfLoudspeakers Number of loudspeakers the object is panned to,
   * determines the width of the audio output port. All panning gains are set to
   * zero initially, and no stored transactions are set.
   */
  PanningGainMatrix( SignalFlowContext const & context,
                     char const * name,
                     CompositeComponent * parent,
                     std::size_t numberOfObjects,
                     std::size_t numberOfLoudspeakers );

  /**
   * Constructor with an initial gain settings.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component
   * @param numberOfObjects The number of audio objects to be panned, determines
   * the width of the audio input port.
   * @param numberOfLoudspeakers Number of loudspeakers the object is panned to,
   * determines the width of the audio output port.
   * @param initialGains Mtrix of initial gains, dimension \p numberOfObjects x
   * \p numberOfLoudspeakers.
   */
  PanningGainMatrix( SignalFlowContext const & context,
                     char const * name,
                     CompositeComponent * parent,
                     std::size_t numberOfObjects,
                     std::size_t numberOfLoudspeakers,
                     efl::BasicMatrix< SampleType > const & initialGains );

  /**
   * Constructor with an initial gain settings and a set of initial transitions.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component
   * @param numberOfObjects The number of audio objects to be panned, determines
   * the width of the audio input port.
   * @param numberOfLoudspeakers Number of loudspeakers the object is panned to,
   * determines the width of the audio output port.
   * @param initialGains Mtrix of initial gains, dimension \p numberOfObjects x
   * \p numberOfLoudspeakers.
   * @param initialTransitions A set of initial transactions. This erforms the
   * same operation as if the parameter would be received by the component
   * within the first process() call.
   */
  PanningGainMatrix( SignalFlowContext const & context,
                     char const * name,
                     CompositeComponent * parent,
                     std::size_t numberOfObjects,
                     std::size_t numberOfLoudspeakers,
                     efl::BasicMatrix< SampleType > const & initialGains,
                     PanningMatrixParameter const & initialTransitions );

  /**
   * Destructor (virtual)
   */
  ~PanningGainMatrix() override;

  /**
   * Overriden process() function.
   */
  void process() override;

private:
  void processParameters();

  void processAudio();

  /**
   * Process the audio signal for a single 'slope', that is, a number
   * of samples with no change if the gain increase.
   */
  void processAudioSingleSlope( std::size_t objIdx,
                                TimeType currentTime,
                                std::size_t startIdx,
                                std::size_t duration,
                                bool accumulate,
                                std::size_t alignment );

  void updateTransitions( TimeType currentTime,
                          PanningMatrixParameter const & newParams );

  void updateTransition( std::size_t objIndex,
                         TimeType currentTime,
                         TimeType startTime,
                         TimeType duration,
                         SampleType const * gains );

  void scaleSignal( SampleType const * input,
                    SampleType * output,
                    std::size_t startIdx,
                    std::size_t duration,
                    SampleType gainStart,
                    SampleType gainInc,
                    bool accumulate,
                    std::size_t alignment );

  AudioInput mAudioInput;
  AudioOutput mAudioOutput;

  using GainInput =
      ParameterInput< pml::DoubleBufferingProtocol, PanningMatrixParameter >;

  GainInput mGainInput;

  TimeVector mPreviousTime;
  TimeVector mTargetTime;

  visr::efl::BasicMatrix< SampleType > mPreviousGains;
  visr::efl::BasicMatrix< SampleType > mTargetGains;

  /**
   * Number of stored future transitions.
   * For the moment this is a hard-coded number, but could be configurable in
   * the future.
   */
  static constexpr std::size_t cNumPendingTransitions = 1;

  /**
   * Fixed-length vector of 'stored' future transaction.
   * At the moment, this vector contains only a single element, but is kept
   * as vector to make the extension to multiple stored transactions easier.
   */
  std::vector< PanningMatrixParameter > mPendingTransitions;

  /**
   * Internal buffer used in the gain scaling.
   */
  visr::efl::BasicVector< SampleType > mScalingRamp;
};

} // namespace panningdsp
} // namespace visr

#endif // #ifndef VISR_LIBPANNINGDSP_PANNING_GAIN_MATRIX_HPP_INCLUDED
