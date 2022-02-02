/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_TIME_FREQUENCY_TRANSFORM_HPP_INCLUDED
#define VISR_LIBRCL_TIME_FREQUENCY_TRANSFORM_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/parameter_output.hpp>

#include <libefl/basic_vector.hpp>

#include <libpml/time_frequency_parameter.hpp>
#include <libpml/shared_data_protocol.hpp>

#include <librbbl/circular_buffer.hpp>

#include <cstddef> // for std::size_t
#include <memory>

namespace visr
{
// Forward declarations
namespace rbbl
{
template< typename SampleType >
class FftWrapperBase;
}

namespace rcl
{
/**
 * Component to transform a multichannel input audio signal into a sequence of
 * time-frequency parameter data.
 */
class VISR_RCL_LIBRARY_SYMBOL TimeFrequencyTransform: public AtomicComponent
{
public:
  using SampleType = visr::SampleType;

  /**
   * Enumeration to select the FFT normalisation applied to the forward Fourier
   * transform.
   */
  enum class Normalisation
  {
    One,     /**< No normalisation applied. */
    Unitary, /**< Normalisation (scaling) factor 1/sqrt(N) */
    OneOverN /**< Scaling factor 1/N, which means that the corresponding inverse
                FFT should have a normalisation of "One" to preserve an overall scaling of 1. */
  };

  /**
   * Constructor. Creates a time-frequency transformation component with a
   * default window shape.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component
   * @param numberOfChannels The number of distinct audio waveforms received
   * through the input audio port.
   * @param dftSize The size of the DFT transform used. Must be a power of two
   * for most FFT implementations.
   * @param windowLength The length of the window. A Hann window modified to
   * satisfy the Constant Overlap-Add (COLA) property is used.
   * @param hopSize Advance (in samples) between successive frames. The
   * component's period size must be an ineger multiple of the hop size.
   * @param fftImplementation String desribing the FFT implementation to be
   * used. Optional parameter, defaults to the platform's default FFT
   * implementation.
   * @param normalisation The DFT normalisation policy, defaults to 'Unitary'
   */
  explicit TimeFrequencyTransform(
      SignalFlowContext const & context,
      char const * name,
      CompositeComponent * parent,
      std::size_t numberOfChannels,
      std::size_t dftSize,
      std::size_t windowLength,
      std::size_t hopSize,
      char const * fftImplementation = "default",
      Normalisation normalisation = Normalisation::Unitary );

  /**
   * Constructor. Creates a time-frequency transformation component with a
   * user-defined window shape.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component
   * @param numberOfChannels The number of distinct audio waveforms received
   * through the input audio port.
   * @param dftSize The size of the DFT transform used. Must be a power of two
   * for most FFT implementations.
   * @param window The coefficients of the window applied to the time-domain
   * input frames.
   * @param hopSize Advance (in samples) between successive frames. The
   * component's period size must be an ineger multiple of the hop size.
   * @param fftImplementation String naming the FFT implementation to be used.
   * Optional parameter, defaults to the platform's default FFT implementation.
   * @param normalisation The DFT normalisation policy, defaults to 'Unitary'
   */
  explicit TimeFrequencyTransform(
      SignalFlowContext const & context,
      char const * name,
      CompositeComponent * parent,
      std::size_t numberOfChannels,
      std::size_t dftSize,
      efl::BasicVector< SampleType > const & window,
      std::size_t hopSize,
      char const * fftImplementation = "default",
      Normalisation normalisation = Normalisation::Unitary );

  /**
   * Destructor (virtual)
   */
  ~TimeFrequencyTransform() override;

  void process() override;

private:
  std::size_t const cAlignment;

  /**
   * The number of simulataneously processed audio channels.
   */
  std::size_t const cNumberOfChannels;

  /**
   * The size of the discreteFourier transform;
   */
  std::size_t const cDftSize;

  /**
   * The number of unique DFT frequency bins of the real-to-complex conversion.
   */
  std::size_t const cNumberOfDftBins;

  /**
   * Size of the window that is multiplied with the input samples.
   * If the window length is smaller than the DFT size, then the remaining input
   * samples before the start of the window are zeroed out.
   */
  std::size_t const cWindowLength;

  std::size_t cFramesPerPeriod;

  std::size_t const cHopSize;

  rbbl::CircularBuffer< SampleType > mInputBuffer;

  /**
   * Wrapper for the different FFT libraries
   */
  std::unique_ptr< rbbl::FftWrapperBase< SampleType > > mFftWrapper;

  /**
   * The window sequence.
   */
  efl::BasicVector< SampleType > mWindow;

  /**
   * Vector to hold temporary results.
   */
  efl::BasicVector< SampleType > mCalcBuffer;

  /**
   * Input audio port, name "in", width numberOfChannels.
   */
  AudioInput mInput;

  /**
   * Output parameter port.
   */
  ParameterOutput< pml::SharedDataProtocol,
                   pml::TimeFrequencyParameter< SampleType > >
      mOutput;
};

} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_TIME_FREQUENCY_TRANSFORM_HPP_INCLUDED
