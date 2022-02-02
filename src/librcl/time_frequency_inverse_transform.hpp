/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_TIME_FREQUENCY_INVERSE_TRANSFORM_HPP_INCLUDED
#define VISR_LIBRCL_TIME_FREQUENCY_INVERSE_TRANSFORM_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/parameter_input.hpp>

#include <libefl/aligned_array.hpp>
#include <libefl/basic_matrix.hpp>

#include <libpml/time_frequency_parameter.hpp>
#include <libpml/shared_data_protocol.hpp>

#include <cstddef> // for std::size_t

namespace visr
{
// Forward declarations
namespace rbbl
{
template< typename SampleType >
class FftWrapperBase;

template< typename SampleType >
class CircularBuffer;

} // namespace rbbl

namespace rcl
{
class VISR_RCL_LIBRARY_SYMBOL TimeFrequencyInverseTransform
 : public AtomicComponent
{
  using SampleType = visr::SampleType;

public:
  /**
   * Enumeration to select the FFT normalisation applied to the inverse Fourier
   * transform.
   */
  enum class Normalisation
  {
    One,     /**< No normalisation applied. */
    Unitary, /**< Normalisation (scaling) factor 1/sqrt(N), which distributes
                the scaling for an overall gain of 1 uniformly between the
                forward and the inverse transform. */
    OneOverN /**< Scaling factor 1/N, which means that the corresponding forward
                FFT should have a normalisation of "One" to preserve an overall
                scaling of 1. */
  };

  /**
   * Constructor.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component
   * @param numberOfChannels The number of simultaneous input waveforms.
   * @param dftLength The size of the DFT transform used. Must be a power of two
   * for most FFT implementations.
   * @param hopSize Advance (in samples) between successive frames. The
   * component's period size must be an ineger multiple of the hop size.
   * @param fftImplementation String desribing the FFT implementation to be
   * used. Optional parameter, defaults to the platform's default FFT
   * implementation.
   * @param normalisation FFT scaling applied to the inverse FFT, optional
   * argument. THe defulet value is 'OneOverN'.
   */
  explicit TimeFrequencyInverseTransform(
      SignalFlowContext const & context,
      char const * name,
      CompositeComponent * parent,
      std::size_t numberOfChannels,
      std::size_t dftLength,
      std::size_t hopSize,
      char const * fftImplementation = "default",
      Normalisation normalisation = Normalisation::OneOverN );

  ~TimeFrequencyInverseTransform() override;

  void process() override;

private:
  std::size_t const cAlignment;

  /**
   * Number of separate audio signals in the output signal 
   * (and in the received time-frequency frames).
   */
  std::size_t const cNumberOfChannels;

  /**
   * The size of the Fourier transform;
   */
  std::size_t const cDftSize;

  std::size_t const cNumberOfDftBins;

  std::size_t const cFramesPerPeriod;

  std::size_t const cHopSize;

  efl::BasicMatrix< SampleType > mAccumulationBuffer;

  /**
   * Wrapper for the different FFT libraries
   */
  std::unique_ptr< rbbl::FftWrapperBase< SampleType > > mFftWrapper;

  efl::AlignedArray< SampleType > mCalcBuffer;

  /**
   * Scaling factor to achieve the desired normalisation.
   */
  SampleType const cScalingFactor;

  /**
   * Parameter input port for time-frequency frames, name "in"
   */
  ParameterInput< pml::SharedDataProtocol,
                  pml::TimeFrequencyParameter< SampleType > >
      mInput;

  /**
   * Audio output port for the transformed audio signals, name "out", width \p
   * numberOfChannels.
   */
  AudioOutput mOutput;
};

} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_TIME_FREQUENCY_INVERSE_TRANSFORM_HPP_INCLUDED
