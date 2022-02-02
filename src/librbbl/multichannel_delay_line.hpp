/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRBBL_MULTICHANNEL_DELAY_LINE_HPP_INCLUDED
#define VISR_LIBRBBL_MULTICHANNEL_DELAY_LINE_HPP_INCLUDED

#include "circular_buffer.hpp"
#include "fractional_delay_base.hpp"
#include "export_symbols.hpp"

#include <libvisr/constants.hpp>

#include <cassert>
#include <complex>
#include <memory>
#include <vector>

namespace visr
{
namespace rbbl
{

/**
 * Generic multichannel delay line class that may use a variety of fractional delay interpolation techniques.
 * New input is written as one multichannel chunk with a fixed number of samples, but data is read individually 
 * based on the current write pointer position. Thus this class can be used as a multitap delay line.
 * @tparam SampleType The floating-point type of the signal samples
 */
template< typename SampleType >
class  VISR_RBBL_LIBRARY_SYMBOL MultichannelDelayLine
{
public:
  /**
   * Enumeration for how to incorporate the implementation delay of the method into the interpolation.
   * The implementation (or method) delay is the delay introduced by the fractional delay algorithm itself.
   */
  enum class MethodDelayPolicy
  {
    Add, /**< Always add the implementation delay to the nominal amount of delay. */
    Limit, /**< Compensate the implementation delay, but limit to the minimum admissible delay if the nominal delay
    is smaller than the implementation delay */
    Reject /**< Throw an exception if the nominal delay value is lower than the method delay. */
  };

  /**
  * Constructor.
  * @param numberOfChannels The number of input signals processed.
  * @param samplingFrequency The sampling frequency [in Hz]. This is needed to convert the delay input values to samples.
  * @param blockLength The numbers of samples written in each call to write().
  * @param maxDelaySeconds The maximum admissible delay (in seconds)
  * both reuse the same filter representation multiple times, or store multiple filter representations to enable switching of the filter characteristics at runtime.
  * @param interpolationMethod The fractional delay method used to retrieve the delayed samples. It must match the name of an algorithm registered in 
  * the FractionalDelayFactory factory class.
  * @param methodDelayPolicy Enumeration value  determining how the inherent method (or implementation) delay is incorporated into the desired delay value.
  * @param alignment The alignment (given as a multiple of the sample type size) of the data buffers passed to the write() and interpolate() calls.
  * The alignment setting is also used to allocate all data structures.
  * the . 
  */
  explicit MultichannelDelayLine( std::size_t numberOfChannels,
                                  SamplingFrequencyType samplingFrequency,
                                  std::size_t blockLength,
                                  SampleType maxDelaySeconds,
                                  char const * interpolationMethod,
                                  MethodDelayPolicy methodDelayPolicy = MethodDelayPolicy::Add,
                                  std::size_t alignment = 0 );

  /**
   * Destructor.
   */
  ~MultichannelDelayLine();

  /**
   * Return the number of channels in the delay line.
   */
  std::size_t numberOfChannels() const;

  /**
   * The number of elements to be written in one call to write().
   */
  std::size_t blockLength() const;

  /**
  * Return the inherent method delay (or implementation delay).
  * @return Implementation delay in seconds.
  */
  SampleType methodDelaySeconds() const;

  /**
  * Return the inherent method delay (or implementation delay).
  * @return Implementation delay in samples.
  */
  SampleType methodDelaySamples() const;

  /**
   * Write a chunk of multichannel sample data into the delay line.
   * @param input The data to be written. Samples within a channel must be contiguous (stride==1)
   * @param channelStride The distance (in number of elements) between the sample data of subsequent channels.
   * @param numberOfChannels The number of channels contained in the \p input buffer.
   * @param alignment The minimum alignment (in samples) of each channel of the \p input buffer.
   * @throw std::invalid_argument If \p numberOfChannels does not match the channel number of the delay line.
   */
  void write( SampleType const * input,
              std::size_t channelStride,
              std::size_t numberOfChannels,
              std::size_t alignment );

  /**
   * Interpolate and scale a given number of samples of one of the input channels.
   * The delay and gain (scaling) values are given for the first (actually one before the first) 
   * and the last sample, and delay and gain are interpolated linearly for the samples in between.
   * However, the selected interpolation algorithm might behave somewhat differently.
   * @param[out] output Buffer to return the interpolated signal. Must provide space for \p numberOfSamples values and be aligned 
   * with the alignment set in the constructor.
   * @param channelIdx The delay line channel to be interpolated. Must be in the range $0 <= channelIndex < numberOfChannels()$.
   * @param numberOfSamples The number of samples to be generated.
   * @param startDelay The start value for the interpolated delay values (in seconds). In terms of the linearly interpolated delay, this value refers to the delay
   * prior to the first output sample \p output[0].
   * @param endDelay The start value for the interpolated delay values (in seconds). 
   * This delay value is reached at the last output sample \p output[numberOfSamples-1]
   * @param startGain The start value for the interpolated gain values (linear scale). In terms of the linearly interpolated gains, this value refers to the gain
   * prior to the first output sample \p output[0].
   * @param endGain The start value for the interpolated gain values (linear scale).
   * This gain value is reached at the last output sample \p output[numberOfSamples-1]
   * @throw std::invalid_argument If \p channelIndex exceeds the number of channels in the delay line.
   * @throw std::out_of_range If either \p startDelay or \p endDelay exceeds the configured maximum delay.
   * @throw std::out_of_range If either \p startDelay or \p endDelay undershoots the minimum admissible delay (determined 
   * by the selected fractional delay algorithm and the chosen policy for the method delay.
   */
  void interpolate( SampleType * output,
                    std::size_t channelIdx,
                    std::size_t numberOfSamples,
                    SampleType startDelay, SampleType endDelay,
                    SampleType startGain, SampleType endGain );

private:
  /**
   * Adjust the delay for the method delay of the interpolator depending on the chosen policy and scale it from seconds to samples.
   * @param rawDelay Desired delay value (in seconds)
   * @return Delay value in samples, adjusted according to the configured method delay policy.
   * @throw std::out_of_range if the \p rawDelay value exceeds the configured maximum delay.
   * @throw std::out_of_range if the delay value is lower than the method delay and the "reject" policy is chosen.
   */
  SampleType adjustDelay( SampleType rawDelay ) const;

  /**
   * Number of elements written in each write() call.
   * @note Technically, we would not need a constant number of samples written as 
   * long as the write pointer remains identical for all channels
   */
  std::size_t const cBlockLength;

  /**
   * The sampling frequency in Hz.
   */
  SampleType const cSamplingFrequency;

  /**
   * The maximum admissible delay (in samples)
   */
  SampleType const cMaxDelaySamples;

  /**
   * Policy how to incorporate the inherent delay of the fractional delay algorithm used.
   */
  MethodDelayPolicy const cMethodDelayPolicy;

  /**
   * Circular buffer holding the delayed values.
   */
  rbbl::CircularBuffer<SampleType> mRingbuffer;

  /**
   * The fractional delay algorihm object.
   * @note this needs to be a shared pointer, because the object is dynamically 
   * allocated via a factory
   */
  std::unique_ptr<FractionalDelayBase<SampleType> > mInterpolator;

  /**
   * The method delay (implementation delay) of the selected fractional delay algorithm in samples.
   */
  SampleType cMethodDelay;
};

} // namespace rbbl
} // namespace visr
  
#endif // #ifndef VISR_LIBRBBL_MULTICHANNEL_DELAY_LINE_HPP_INCLUDED
