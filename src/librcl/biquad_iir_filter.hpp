/* Copyright 2019 AudioScenic - All rights reserved */

#ifndef VISR_LIBRCL_BIQUAD_IIR_FILTER_HPP_INCLUDED
#define VISR_LIBRCL_BIQUAD_IIR_FILTER_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/parameter_input.hpp>

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>

#include <libpml/biquad_parameter.hpp>
#include <libpml/double_buffering_protocol.hpp>

#include <librbbl/biquad_coefficient.hpp>

#include <cstddef>
#include <memory>

namespace visr
{
namespace rcl
{
/**
 * A multichannel IIR filter consisting of a fixed, arbitrary number of biquads
 * per channel. This class has one input port named "in" and one output port
 * named "out". The widths of the input and the output port are identical and is
 * set by the argument <b>numberOfChannels</b> in the setup() method.
 */
class VISR_RCL_LIBRARY_SYMBOL BiquadIirFilter: public visr::AtomicComponent
{
  using SampleType = visr::SampleType;

public:
  /**
   * Constructor that yields a fully initialised object.
   * It is not needed to call a setup() afterwards.
   * This overload sets all EQs to a flat response.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component.
   * @param numberOfChannels The number of single audio waveforms in
   * the multichannel input and output waveforms. .
   * @param numberOfBiquads The number of biquads per audio channel.
   * @param controlInput Flag whether to instantiate a parameter port for
   * receiving filter update commands.
   */
  explicit BiquadIirFilter( visr::SignalFlowContext const & context,
                            char const * name,
                            visr::CompositeComponent * parent,
                            std::size_t numberOfChannels,
                            std::size_t numberOfBiquads,
                            bool controlInput = false );

  /**
   * Constructor that yields a fully initialised object.
   * It is not needed to call a setup() afterwards.
   * This overload sets all biquad sections to the same initial value.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component.
   * @param numberOfChannels The number of single audio waveforms in
   * the multichannel input and output waveforms. .
   * @param numberOfBiquads The number of biquads per audio channel.
   * @param initialBiquad The initial setting for the filter characteristics.
   * All biquads in all channels are set to this coefficient set. The default is
   * a flat, direct-feedthrough filter
   * @param controlInput Flag whether to instantiate a parameter port for
   * receiving filter update commands.
   */
  explicit BiquadIirFilter(
      visr::SignalFlowContext const & context,
      char const * name,
      visr::CompositeComponent * parent,
      std::size_t numberOfChannels,
      std::size_t numberOfBiquads,
      visr::rbbl::BiquadCoefficient< SampleType > const & initialBiquad,
      bool controlInput = false );

  /**
   * Constructor that yields a fully initialised object.
   * It is not needed to call a setup() afterwards.
   * This overload sets each channel to the same sequence of biquad sections.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component.
   * @param numberOfChannels The number of signals in the input signal.
   * @param numberOfBiquads The number of biquad sections for each channel.
   * @param coeffs The initial biquad coefficients, which are set identically
   * for all channels. The number of biquad coefficient sets in this parameter
   * must equal the \p numberOfBiquads parameter.
   * @param controlInput Flag whether to instantiate a parameter port for
   * receiving filter update commands.
   */
  explicit BiquadIirFilter(
      visr::SignalFlowContext const & context,
      char const * name,
      visr::CompositeComponent * parent,
      std::size_t numberOfChannels,
      std::size_t numberOfBiquads,
      visr::rbbl::BiquadCoefficientList< SampleType > const & coeffs,
      bool controlInput = false );

  /**
   * Constructor that yields a fully initialised object.
   * It is not needed to call a setup() afterwards.
   * This overload accepts a matrix of biquad filters to specifically initialise
   * every biquad section for each channel.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component. Setup method to initialise the
   * object and set the biquad filters individually for each filter channel and
   * biquad section.
   * @param numberOfChannels The number of signals in the input signal.
   * @param numberOfBiquads The number of biquad sections for each channel.
   * @param coeffs The initial biquad coefficients as a matrix of size \p
   * numberOfChannels x \p numberOfBiquads
   * @param controlInput Flag whether to instantiate a parameter port for
   * receiving filter update commands.
   */
  explicit BiquadIirFilter(
      visr::SignalFlowContext const & context,
      char const * name,
      visr::CompositeComponent * parent,
      std::size_t numberOfChannels,
      std::size_t numberOfBiquads,
      visr::rbbl::BiquadCoefficientMatrix< SampleType > const & coeffs,
      bool controlInput = false );

  /**
   * The process method applies the IIR filters to the audio channels.
   * values to the stream of input samples.
   */
  void process() override;

  /**
   * Support for runtime changes of the filter coefficients.
   * @note At the moment, the coefficients are appplied instantaneously, with
   * not precautions against transients or potential instabilities.
   */
  //@{
  /**
   * Set the IIR biquad coefficients for a specific biquad section of a becific
   * filter channel.
   * @param channelIndex The index of the filter channel.
   * @param biquadIndex The index of the biquad section to be set.
   * @param coeffs The set of biquad IIR coefficients.
   * @throw std::out_of_range If \p channelIndex exceeds the number of filter
   * channels.
   * @throw std::out_of_range If \p biquadIndex exceeds the number of biquad
   * sections of this component.
   */
  void setCoefficients(
      std::size_t channelIndex,
      std::size_t biquadIndex,
      visr::rbbl::BiquadCoefficient< SampleType > const & coeffs );

  /**
   * Set the biquad coefficients for a filter channel.
   * @param channelIndex The index of the filter channel.
   * @param coeffs The biquad coefficients. The number of sections must match
   * the configured number of biquads of the component.
   * @throw std::out_of_range If \p channelIndex exceeds the number of filter
   * channels.
   * @throw std::invalid_argument If the number of biquads in \p coeffs does not
   * match the number of biquad sections in this component.
   */
  void setChannelCoefficients(
      std::size_t channelIndex,
      visr::rbbl::BiquadCoefficientList< SampleType > const & coeffs );

  /**
   * Set the biquad coefficients for a filter channel.
   * @param coeffs The matrix of biquad coefficients, dimension \p
   * numberOfChannels x \p numberOfBiquadSections
   * @throw std::invalid_argument If the matrix dimension differs from \p
   * numberOfChannels x \p numberOfBiquadSections
   */
  void setCoefficientMatrix(
      visr::rbbl::BiquadCoefficientMatrix< SampleType > const & coeffs );

  //@}
private:
  /**
   * Internal method to set a single biquad section.
   * No range checks.
   * @see setCoefficients
   */
  void setCoefficientsInternal(
      std::size_t channelIndex,
      std::size_t biquadIndex,
      visr::rbbl::BiquadCoefficient< SampleType > const & coeffs );

  /**
   * Internal method to set the biquad coefficients for a filter channel.
   * No range checks performed.
   * @param channelIndex The index of the filter channel.
   * @param coeffs The biquad coefficients. The number of sections must match
   * the configured number of biquads of the component.
   * @see setChannelCoefficients
   */
  void setChannelCoefficientsInternal(
      std::size_t channelIndex,
      visr::rbbl::BiquadCoefficientList< SampleType > const & coeffs );

  /**
   * Internal method to set the complete coefficient matrix.
   * No range checks performed.
   * @see setCoefficientMatrix
   */
  void setCoefficientMatrixInternal(
      visr::rbbl::BiquadCoefficientMatrix< SampleType > const & coeffs );

  /**
   * The audio input port for this component.
   */
  visr::AudioInput mInput;

  /**
   * The audio output port for this component.
   */
  visr::AudioOutput mOutput;

  std::unique_ptr<
      visr::ParameterInput< visr::pml::DoubleBufferingProtocol,
                            visr::pml::BiquadParameterMatrix< SampleType > > >
      mEqInput;

  /**
   * The number of simultaneous audio channels.
   */
  std::size_t const cNumberOfChannels;

  /**
   * The number of biquads per channel.
   */
  std::size_t const cNumberOfBiquadSections;

  /**
   * Matrix to store the IIR coefficients.
   * Each column contains the coefficients for one output channel.
   * The five coefficients for one biquad section are stored in five consecutive
   * columns. Thus, the coefficients \p b0, \p b1, \p b2, \p a1 and \p a2 of
   * biquad section \p N are stored in the rows \p
   * cBiquadCoefficientsStride*N... \p cBiquadCoefficientsStride*N+4. The
   * dimension of the matrix is \p numberOfChannels x \p numberOfBiquadSections*
   * cBiquadCoefficientStride .
   */
  visr::efl::BasicMatrix< SampleType > mCoefficients;

  /**
   * Matrix structure to hold the state (i.e., past outputs of the recursive
   * part of the filter \p w[n-1] and \p w[n-2]. Each row contains contains the
   * state for one channel, with 2 state variables per biquad section The
   * dimension of the matrix is \p numberOfChannels x \p cBiquadStateStride * \p
   * mNumberOfBiquadSections.
   */
  visr::efl::BasicMatrix< SampleType > mState;

  static constexpr std::size_t cBiquadCoefficientsStride =
      visr::rbbl::BiquadCoefficient< SampleType >::cNumberOfCoeffs;

  static constexpr std::size_t cBiquadStateStride = 2;
};

} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_BIQUAD_IIR_FILTER_HPP_INCLUDED
