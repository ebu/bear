/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_PANNING_CALCULATOR_HPP_INCLUDED
#define VISR_LIBRCL_PANNING_CALCULATOR_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>

#include <libobjectmodel/object.hpp> // needed basically for type definitions

#include <libpml/listener_position.hpp>

#include <libpanning/LoudspeakerArray.h>
#include <libpanning/VBAP.h>
#include <libpanning/XYZ.h>

#include <memory>
#include <valarray>
#include <vector>

namespace visr
{

namespace panning
{
  class LoudspeakerArray;
}

namespace pml
{
// class ListenerPosition;
class ObjectVector;
template< typename ElementType > class MatrixParameter;
class SharedDataProtocol;
class DoubleBufferingProtocol;
}

namespace rcl
{

/**
 * Audio component for calculating the gains for a variety of panning algorithms from a set of audio object descriptions.
 */
class VISR_RCL_LIBRARY_SYMBOL PanningCalculator: public AtomicComponent
{
public:
  enum class PanningMode
  {
    Nothing = 0, ///< Not used as a real choice, just for bit-testing.
    LF = 1,
    HF = 2,
    Diffuse = 4,
    Dualband = LF | HF,
    All = Dualband | Diffuse
  };

  enum class Normalisation
  {
    Default = 0,    ///< Use the default value for the frequency regime in question.
    Amplitude = 1,  ///< l_1 (amplitude/low frequency) normalisation
    Energy = 2      ///< l_2 (energy/high-frequency) normalisation
  };

  /**
   * Type of the gain coefficients. We use the same type as the samples in the signal flow graph
   * @todo maybe this should become a template parameter.
   */
  using CoefficientType = SampleType;

  /**
  * Constructor.
  * @param context Configuration object containing basic execution parameters.
  * @param name The name of the component. Must be unique within the containing composite component (if there is one).
  * @param parent Pointer to a containing component if there is one. Specify \p nullptr in case of a top-level component.
  * @param numberOfObjects The number of VBAP objects to be processed.
  * @param arrayConfig The array configuration object.
  * @param adaptiveListenerPosition Whether the rendering supports adaptation to a tracked listener.
  * @param separateLowpassPanning Whether to generate two separate gain matrixes for low and high frequencies.
  */
  explicit PanningCalculator( SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    std::size_t numberOfObjects,
    panning::LoudspeakerArray const & arrayConfig,
    bool adaptiveListenerPosition = false,
    bool separateLowpassPanning = false );


  /**
   * Constructor.
   * This is a legacy constructor that doesn't use the HF/LF port specification or noralisation options.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p nullptr in case of a top-level component.
   * @param numberOfObjects The number of VBAP objects to be processed.
   * @param arrayConfig The array configuration object.
   * @param adaptiveListenerPosition Whether the rendering supports adaptation to a tracked listener.
   * @param panningMode Select the gain outputs provided (LF, HF, Diffuse or any combination thereof)
   * @param lfNormalisation The normalisation norm used for the LF gains. Default is 'Energy' as in plain VBAP if HF panning is deactivated,
   * or 'Amplitude' if HF panning is present.
   * @param hfNormalisation The normalisation norm used for the HF (VBIP) gains. Default is 'Energy'.
   * @param diffuseNormalisation The normalisation norm used for the diffuse gains. Default is 'Energy'.
   * @param listenerPosition The initial listener position (central position, default is (0,0,0)
   */
  explicit PanningCalculator( SignalFlowContext const & context,
                              char const * name,
                              CompositeComponent * parent,
                              std::size_t numberOfObjects,
                              panning::LoudspeakerArray const & arrayConfig,
                              bool adaptiveListenerPosition,
                              PanningMode panningMode = PanningMode::LF,
                              Normalisation lfNormalisation = Normalisation::Default,
                              Normalisation hfNormalisation = Normalisation::Default,
                              Normalisation diffuseNormalisation = Normalisation::Default,
                              pml::ListenerPosition const & listenerPosition = pml::ListenerPosition(0.0f, 0.0f, 0.0f )
  );


  /**
   * Disabled (deleted) copy constructor
   */
  PanningCalculator( PanningCalculator const & ) = delete;


  /**`
   * Destructor.
   */
  ~PanningCalculator();

  /**
   * The process function.
   * It takes a vector of objects as input and calculates a vector of output gains.
   */
  void process();

private:
  bool separateLowpassPanning() const { return bool(mLowFrequencyGainOutput); }


  using LoudspeakerIndexType = std::size_t;


  /**
   *
   */
  bool isTwoD() const { return mVectorDimension == 2; }


  /**
  * Set the reference listener position. This overload accepts three cartesian coordinates.
  * The listener positions are used beginning with the next process() call.
  * This method triggers the recalculation of the internal data (e.g., the inverse panning matrices).
  * @param x The x coordinate [in meters]
  * @param y The y coordinate [in meters]
  * @param z The z coordinate [in meters]
  * @throw std::runtime_error If the recalculation of the internal panning data fails.
  * @todo Consider extending the interface to multiple listener positions.
  */
  void setListenerPosition( CoefficientType x, CoefficientType y, CoefficientType z );

  /**
  * Set the reference listener position given as a ListenerPosition object.
  * The listener positions are used beginning with the next process() call.
  * This method triggers the recalculation of the internal data (e.g., the inverse panning matrices).
  * @param pos The listener position.
  * @throw std::runtime_error If the recalculation of the internal panning data fails.
  * @todo Consider extending the interface to multiple listener positions.
  */
  void setListenerPosition( pml::ListenerPosition const & pos );

  /**
   * The number of audio objects handled by this object.
   */
  std::size_t const mNumberOfObjects;

  /**
   * The number of panning loudspeakers.
   * @note This excludes any potential subwoofers (which are not handled by the panning algorithm)
   */
  std::size_t const mNumberOfRegularLoudspeakers;

  /**
   * Number of all loudspeakers, including virtual (phantom) loudspeakers.
   *
   */
  std::size_t const mNumberOfAllLoudspeakers;


  /**
   * The dimension of the loudspeaker and position vectors.
   * Either 3 for a standard 3D panning setup, or 2 for a horizontal-only setup.
   */
  std::size_t const mVectorDimension;

  efl::BasicMatrix<CoefficientType> mLoudspeakerPositions;

  /**
   * The calculator object to generate the panning matrix coefficients.
   */
  std::unique_ptr<panning::VBAP> mVbapCalculator;

  /**
   * Temporary storage for the computed panning gains of one source.
   * Dimension: mNumberOfRegularLoudspeakers
   */
  mutable efl::BasicVector<SampleType> mTmpGains;

  /**
  * Vector for intermediate results used for separate high/low frequency panning
  */
  mutable efl::BasicVector<CoefficientType> mTmpHfGains;

  mutable efl::BasicVector<SampleType> mTmpDiffuseGains;

  Normalisation const mLfNormalisation;

  Normalisation const mHfNormalisation;

  Normalisation const mDiffuseNormalisation;

  using ChannelLabelLookup = std::map< std::string, std::size_t >;

  ChannelLabelLookup const mLabelLookup;

  /**
   * Internal function to initialise the label lookup table.
   */
  static ChannelLabelLookup fillLabelLookup( panning::LoudspeakerArray const & config );

  /**
   * Data type of the parameter ports for outgoing matrix data.
   */
  using ListenerPositionPort = ParameterInput<pml::DoubleBufferingProtocol, pml::ListenerPosition >;
  using ObjectPort = ParameterInput<pml::DoubleBufferingProtocol, pml::ObjectVector >;
  using MatrixPort = ParameterOutput<pml::SharedDataProtocol, pml::MatrixParameter<CoefficientType> >;

  std::unique_ptr<ObjectPort> mObjectVectorInput;

  std::unique_ptr<ListenerPositionPort> mListenerPositionInput;

  /**
   * Gain output for high-frequency panning (VBAP).
   */
  std::unique_ptr<MatrixPort> mLowFrequencyGainOutput;

  /**
   * Gain output for high-frequency panning (VBIP).
   */
  std::unique_ptr<MatrixPort> mHighFrequencyGainOutput;

  /**
  * Optional gain output for signals to be decorrelated.
  */
  std::unique_ptr<MatrixPort> mDiffuseGainOutput;


};




/**
* Bitwise operator to combine output port flags.
*/
VISR_RCL_LIBRARY_SYMBOL PanningCalculator::PanningMode operator|( PanningCalculator::PanningMode lhs,
                                                                  PanningCalculator::PanningMode rhs );

/**
* Bitwise operator to extract output port flags.
*/
VISR_RCL_LIBRARY_SYMBOL PanningCalculator::PanningMode operator&( PanningCalculator::PanningMode lhs,
                                                                  PanningCalculator::PanningMode rhs );



} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_PANNING_CALCULATOR_HPP_INCLUDED
