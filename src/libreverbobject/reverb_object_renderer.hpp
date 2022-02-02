/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_REVERBOBJECT_REVERB_OBJECT_RENDERER_HPP_INCLUDED
#define VISR_REVERBOBJECT_REVERB_OBJECT_RENDERER_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/component.hpp>
#include <libvisr/composite_component.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/parameter_input.hpp>

#include <librcl/add.hpp>
#include <librcl/delay_vector.hpp>
#include <librcl/gain_matrix.hpp>
#include <librcl/signal_routing.hpp>

#include "late_reverb_filter_calculator.hpp"
#include "reverb_parameter_calculator.hpp"

#include <libefl/basic_matrix.hpp>

#include <libpml/listener_position.hpp>
#include <libpml/double_buffering_protocol.hpp>
#include <libpml/object_vector.hpp>

#include <memory>
#include <string>

namespace visr
{

// Forward declaration
namespace rcl
{
class FirFilterMatrix;
class BiquadIirFilter;
}

namespace reverbobject
{

/**
 * Rendering subgraph for reverb objects.
 * Usually used as part of a larger renderer that handles object
 * volume + EQ, signal routing and the direct part of the renderer.
 */
class VISR_REVERBOBJECT_LIBRARY_SYMBOL ReverbObjectRenderer: public CompositeComponent
{
public:
  /**
   * Constructor to create, initialise and interconnect the processing components.
   * @param context Configuration object holding basic execution parameters.
   * @param name Name of the component.
   * @param parent Pointer to containing component (if there is one). A value of \p nullptr signals that this is a top-level component.
   * @param reverbConfig JSON-formatted string to hold the reverb-specific configuration.
   *        - numReverbObjects (integer) The maximum number of reverb objects (at a given time)
   *        - lateReverbFilterLength (floating-point) The length of the late reverberation filter (in seconds)
   *        - discreteReflectionsPerObject (integer) The number of discrete reflections per reverb object.
   *        - lateReverbDecorrelationFilters (string) Absolute or relative file path (relative to start directory of the renderer) to a multichannel audio file (typically WAV)
   *          containing the filter coefficients for the decorrelation of the late part.
   *        - lateReverbCrossfadeTime (floating-point number) Crossfade time between reverb tails in case of a switch. A value of 0.0 triggers a more efficient implementation, at the cost of a 
   *          hard switch with potential clicks or a transient loudness boosts.
   * @param arrayConfig Array configuration object to describe the reproduction system.
   * @param numberOfObjectSignals Total number of object audio signals that might carry reverb objects.
   */
  explicit ReverbObjectRenderer( SignalFlowContext const & context,
                                 char const * name,
                                 CompositeComponent * parent,
                                 std::string const & reverbConfig,
                                 panning::LoudspeakerArray const & arrayConfig, 
                                 std::size_t numberOfObjectSignals );

  ~ReverbObjectRenderer();

private:
  AudioInput mObjectSignalInput;

  AudioOutput mLoudspeakerOutput;

  ParameterInput< pml::DoubleBufferingProtocol, pml::ObjectVector > mObjectVector;
   
  ReverbParameterCalculator mReverbParameterCalculator;

  rcl::SignalRouting mReverbSignalRouting;

  rcl::DelayVector mDiscreteReverbDelay;

  std::unique_ptr< rcl::BiquadIirFilter > mDiscreteReverbReflFilters;

  rcl::GainMatrix mDiscreteReverbPanningMatrix;

  std::unique_ptr<LateReverbFilterCalculator> mLateReverbFilterCalculator;
    
  /**
   * Overall gain and delay for the source signals going into the late
   * reverberation part.
   * This is used to apply the object level, but should also apply the
   * onset delay.
   */
  rcl::DelayVector mLateReverbGainDelay;

  /**
   * FIR filter matrix for the late reverberation tails, with one channel per 
   * reverb object channel.
   * Depending on the configuration, this can be poulated with either a rcl::CrossfadingFirFilterMatrix
   * (with crossfade) or a rcl::FirFilterMatrix (lateReverbCrossfadeTime=0.0) hard switching, but less expensive)
   */
  std::unique_ptr< visr::Component > mLateReverbFilter;

  std::unique_ptr< rcl::FirFilterMatrix > mLateDiffusionFilter;

  rcl::Add mReverbMix;
};

} // namespace reverbobject
} // namespace visr

#endif // VISR_REVERBOBJECT_REVERB_OBJECT_RENDERER_HPP_INCLUDED
