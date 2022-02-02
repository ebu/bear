/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRRL_FLEXIBLE_BUFFER_WRAPPER_HPP_INCLUDED
#define VISR_LIBRRL_FLEXIBLE_BUFFER_WRAPPER_HPP_INCLUDED

#include "export_symbols.hpp"

#include <librbbl/circular_buffer.hpp>

#include <libvisr/constants.hpp>

namespace visr
{
namespace rrl
{
class AudioSignalFlow;

/**
 * Class to enable processing with arbitrary, varying numbers of frames per
 * process() calls. This involves buffering the input and output data,
 * accumulating them until the signal flow can be invoked with the fixed
 * number of samples.
 * This makes VISR signal flows usable with APIs that do not guarantee a fixed
 * number of frames per invocation. This causes an additional latency  of one
 * period size. A flexibleBufferWrapper is constructed around a an existing
 * AudioSignalFlow object, and is used by calling one of its process() methods
 * instead od AudioSignallFlow::process().
 */
class VISR_RRL_LIBRARY_SYMBOL FlexibleBufferWrapper
{
public:
  /**
   * Constructor. Creates a buffering object around an existing audio signal
   * flow.
   * @param flow The audio signal flow to be wrapped in a flexible buffer.
   */
  explicit FlexibleBufferWrapper( AudioSignalFlow & flow );

  ~FlexibleBufferWrapper();

  /**
   * Call the audio signal flow to consume and produce @p numFrames frames.
   * This involves buffering the inputs and outputs.
   * This overload accepts arrays of contiguous sample buffers for the capture
   * and  playback signals.
   * @param captureSamples The input samples, as an array of pointers.
   * @param playbackSamples The output samples, as an array of pointers.
   * @param numFrames The number of frames produced and consumed. Value can be
   * arbitrary.
   */
  void process( SampleType const * const * captureSamples,
                SampleType * const * playbackSamples,
                std::size_t numFrames );

  /**
   * Alternative process() function assuming matrices with fixed channel and
   * element strides for the inputs and outputs.
   * @param captureSamples Pointer to the matrix of input samples.
   * @param[out] playbackSamples Pointer to the first element of the matrix of
   * output samples filled by the process() call.
   * @param captureChannelStride Number of samples between consecutive audio
   * channels of the input matrix.
   * @param captureSampleStride Number of samples between consecutive audio
   * channels of the input matrix. A value of 1 corresponds to consecutively
   * stored samples.
   * @param playbackChannelStride Number of samples between consecutive audio
   * channels of the output matrix.
   * @param playbackSampleStride Number of samples between consecutive audio
   * channels of the output matrix. A value of 1 corresponds to consecutively
   * stored samples.
   * @param numFrames Number of samples (per channels) to be processed.
   */
  void process( SampleType const * captureSamples,
                std::size_t captureChannelStride,
                std::size_t captureSampleStride,
                SampleType * playbackSamples,
                std::size_t playbackChannelStride,
                std::size_t playbackSampleStride,
                std::size_t numFrames );

private:
  AudioSignalFlow & mFlow;

  /**
   * The fixed internal processing block size.
   */
  std::size_t const cInternalPeriod;

  /**
   * Internal sample buffer containing unprocessed input
   * input samples.
   */
  rbbl::CircularBuffer< SampleType > mInputBuffer;
  
  /**
   * Internal sample buffer containing processed output samples.
   * Initialised to one period size of zero samples..
   */
  rbbl::CircularBuffer< SampleType > mOutputBuffer;

  std::size_t mInputBufferLevel;
  std::size_t mOutputBufferLevel;
};

} // namespace rrl
} // namespace visr

#endif // #ifndef VISR_LIBRRL_FLEXIBLE_BUFFER_WRAPPER_HPP_INCLUDED
