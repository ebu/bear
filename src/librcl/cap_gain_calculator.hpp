/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_CAP_GAIN_CALCULATOR_HPP_INCLUDED
#define VISR_LIBRCL_CAP_GAIN_CALCULATOR_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>

#include <libpml/double_buffering_protocol.hpp>
#include <libpml/shared_data_protocol.hpp>
#include <libpml/listener_position.hpp>
#include <libpml/matrix_parameter.hpp>
#include <libpml/object_vector.hpp>

#include <libpanning/LoudspeakerArray.h>
#include <libpanning/CAP.h>
#include <libpanning/XYZ.h>

#include <vector>

namespace visr
{
// forward declarations
namespace objectmodel
{
class ObjectVector;
}
namespace efl
{
template< typename SampleType > class BasicMatrix;
}
namespace panning
{
class CAP_VBAP;
}
namespace pml
{
class ListenerPosition;
}

namespace rcl
{

/**
 * Audio component for calculating the gains for a variety of panning algorithms from a set of audio object descriptions.
 */
class  VISR_RCL_LIBRARY_SYMBOL CAPGainCalculator: public AtomicComponent
{
public:
  /**
   * Type of the gain coefficients. We use the same type as
   */
  using CoefficientType = visr::SampleType;
    
    enum class PanningMode
    {
        Nothing = 0, ///< Not used as a real choice, just for bit-testing.
        LF = 1,
        HF = 2
    };
    
    
  /**
   * Constructor.
   * @param context SignalFlowContext object holding parameters as sampling freqequency and period (block size).
   * @param name The name of the component. Must be unique within the containing AudioSignalFlow.
   * @param parent A containing composite component, or nullptr in case of a top-level component.
   * @param numberOfObjects The number of VBAP objects to be processed.
   * @param arrayConfig The array configuration object.
   * @param panningMode whether the CAP gains are calculated in LF or HF mode.
   * @param hfGainOutput Whether VBIP gains are vomputed in parallel and sent through an optional parameter port ""

   */
  explicit CAPGainCalculator( SignalFlowContext const & context,
                              char const * name,
                              CompositeComponent * parent,
                              std::size_t numberOfObjects, 
                              panning::LoudspeakerArray const & arrayConfig,
                              PanningMode panningMode = PanningMode::LF,
                              bool hfGainOutput = false
                             );

  /**
   * Disabled (deleted) copy constructor
   */
  CAPGainCalculator( CAPGainCalculator const & ) = delete;


  /**
   * Destructor.
   */
  ~CAPGainCalculator();

  /**
   * Virtual function implementing the processing of the component.
   */
  void process() override;

  /**
   * Set a threshold (in form of an arccos value) whether the listener position is considered near 
   * to a triplet boundary.
   * This is used only when the high-frequency (VBIP) gain output is activated.
   */
  void setNearTripletBoundaryCosTheta( SampleType ct );

private:
  /**
   * Internal process function, called by the virtual method process()
   * @param objects The object vector containing the audio scene
   * @param[out] gainMatrix Matrix (numberOfLoudspeakers x numberOfObjects) to hold the computed gains.
   */
  void process( objectmodel::ObjectVector const & objects,
                efl::BasicMatrix<CoefficientType> & gainMatrix );

  /**
   * The number of audio objects handled by this object.
   */
  std::size_t const mNumberOfObjects;

  /**
   * The number of panning loudspeakers.
   * @note This excludes any potential subwoofers (which are not handled by the panning algorithm)
   */
  std::size_t const mNumberOfLoudspeakers;

  /**
   * A vector to hold the source position data.
   * @todo: replace this by a variable-sized vector;
   */
  std::vector<panning::XYZ> mSourcePositions;

  /**
   * The calculator object to generate the panning matrix coefficients.
   */
  panning::CAP mCapCalculator;

  /**
   * Optional calculator for high-frequency gains.
   * It is instantiated if the constructor parameter "hfGainOutput" is set.
   * Uses a special variant of VBAP adapted for CAP layouts.
   */
  std::unique_ptr< panning::CAP_VBAP> mVbipCalculator;

  
  /**
   * The levels of the object channels in linear scale.
   */
  std::valarray<objectmodel::LevelType> mLevels;

  ParameterInput<pml::DoubleBufferingProtocol, pml::ObjectVector > mObjectVectorInput;

  ParameterInput<pml::DoubleBufferingProtocol, pml::ListenerPosition > mListenerPositionInput;

  using GainOutput = ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<CoefficientType> >;
  
  GainOutput mGainOutput;

  /**
   * Optional parameter output to transmit VBIP gains for high frequencies.
   * The port is instantiated it the ctor parameter "hfGainOutput" is set to true.
   * The gains are computed by a special variant of VBAP adapted for CAP layouts.
   */
  std::unique_ptr<GainOutput> mHfGainOutput;

  /**
   * Temporarily used data buffer to hold the computed VBIP gains.
   */
  mutable std::vector<Afloat> mTempHfGains;
};

} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_CAP_GAIN_CALCULATOR_HPP_INCLUDED
