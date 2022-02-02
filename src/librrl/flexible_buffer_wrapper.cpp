/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "flexible_buffer_wrapper.hpp"

#include "audio_signal_flow.hpp"

#include <libefl/basic_matrix.hpp>
#include <libefl/vector_functions.hpp>

#include <libvisr/constants.hpp>

#include <algorithm>
#include <cassert>

namespace visr
{
namespace rrl
{
FlexibleBufferWrapper::FlexibleBufferWrapper( AudioSignalFlow & flow )
 : mFlow{ flow }
 , cInternalPeriod{ flow.period() }
 , mInputBuffer{ flow.numberOfCaptureChannels(), 2 * cInternalPeriod }
 , mOutputBuffer{ flow.numberOfPlaybackChannels(), 2 * cInternalPeriod }
 , mInputBufferLevel{ 0 }
 , mOutputBufferLevel{ cInternalPeriod }
{
  efl::BasicMatrix< SampleType > initMtx{ flow.numberOfPlaybackChannels(),
                                          cInternalPeriod };
  mOutputBuffer.write( initMtx );
}

FlexibleBufferWrapper::~FlexibleBufferWrapper() = default;

namespace // unnamed
{
void copyBuffer( SampleType const * src,
                 SampleType * dest,
                 std::size_t numElements,
                 std::size_t srcStride,
                 std::size_t destStride )
{
  efl::ErrorCode const res =
      ( ( srcStride == 1 ) and ( destStride == 1 ) )
          ? efl::vectorCopy( src, dest, numElements, 0 )
          : efl::vectorCopyStrided( src, dest, srcStride, destStride,
                                    numElements, 0 );
  if( res != efl::ErrorCode::noError )
  {
    throw std::runtime_error( "Error while performing strided copy:" );
  }
}

} // unnamed namespace

void FlexibleBufferWrapper::process( SampleType const * const * captureSamples,
                                     SampleType * const * playbackSamples,
                                     std::size_t numFrames )
{
  std::size_t const numCaptureChannels{ mFlow.numberOfCaptureChannels() };
  std::size_t const numPlaybackChannels{ mFlow.numberOfPlaybackChannels() };

  std::size_t processedFrames{ 0 };
  std::size_t remainingFrames{ numFrames };
  while( remainingFrames > 0 )
  {
    assert( mInputBufferLevel < cInternalPeriod );
    assert( processedFrames + remainingFrames == numFrames );

    std::size_t const chunkFrames{ std::min(
        remainingFrames, cInternalPeriod - mInputBufferLevel ) };
    std::size_t const newBufferLevel{ mInputBufferLevel + chunkFrames };
    assert( newBufferLevel <= cInternalPeriod );

    SampleType * writePtr = mInputBuffer.getFirstWritePointer();
    for( std::size_t chIdx{ 0 }; chIdx < numCaptureChannels; ++chIdx )
    {
      copyBuffer( captureSamples[ chIdx ] + processedFrames,
                  writePtr + chIdx * mInputBuffer.stride(), chunkFrames,
                  1 /*source stride*/, 1 /*destination stride*/ );
    }
    mInputBuffer.commitWrite( chunkFrames );
    mInputBufferLevel += chunkFrames;

    if( newBufferLevel == cInternalPeriod )
    {
      assert( mInputBufferLevel == cInternalPeriod );
      SampleType const * procInputPtr{ mInputBuffer.getReadPointer(
          0, cInternalPeriod ) };
      SampleType * procOutputPtr{ mOutputBuffer.getFirstWritePointer() };
      mFlow.process( procInputPtr, mInputBuffer.stride(), 1, procOutputPtr,
                     mOutputBuffer.stride(), 1 );
      mOutputBuffer.commitWrite( cInternalPeriod );
      mInputBufferLevel -= cInternalPeriod;
      mOutputBufferLevel += cInternalPeriod;
    }

    SampleType const * readPtr =
        mOutputBuffer.getReadPointer( 0, mOutputBufferLevel );
    for( std::size_t chIdx{ 0 }; chIdx < numPlaybackChannels; ++chIdx )
    {
      copyBuffer( readPtr + chIdx * mOutputBuffer.stride(),
                  playbackSamples[ chIdx ] + processedFrames, chunkFrames,
                  1 /*source stride*/, 1 /*destination stride*/ );
    }
    mOutputBufferLevel -= chunkFrames;
    processedFrames += chunkFrames;
    remainingFrames -= chunkFrames;
  }
}

void FlexibleBufferWrapper::process( SampleType const * captureSamples,
                                     std::size_t captureChannelStride,
                                     std::size_t captureSampleStride,
                                     SampleType * playbackSamples,
                                     std::size_t playbackChannelStride,
                                     std::size_t playbackSampleStride,
                                     std::size_t numFrames )
{
  std::size_t const numCaptureChannels{ mFlow.numberOfCaptureChannels() };
  std::size_t const numPlaybackChannels{ mFlow.numberOfPlaybackChannels() };

  std::size_t processedFrames{ 0 };
  std::size_t remainingFrames{ numFrames };
  while( remainingFrames > 0 )
  {
    assert( mInputBufferLevel < cInternalPeriod );
    assert( processedFrames + remainingFrames == numFrames );

    std::size_t const chunkFrames{ std::min(
        remainingFrames, cInternalPeriod - mInputBufferLevel ) };
    std::size_t const newBufferLevel{ mInputBufferLevel + chunkFrames };
    assert( newBufferLevel <= cInternalPeriod );
    SampleType const * captureStartPtr =
        captureSamples + processedFrames * captureSampleStride;

    SampleType * writePtr = mInputBuffer.getFirstWritePointer();
    for( std::size_t chIdx{ 0 }; chIdx < numCaptureChannels; ++chIdx )
    {
      copyBuffer( captureStartPtr + chIdx * captureChannelStride,
                  writePtr + chIdx * mInputBuffer.stride(), chunkFrames,
                  captureSampleStride, 1 /*destination stride*/ );
    }
    mInputBuffer.commitWrite( chunkFrames );
    mInputBufferLevel += chunkFrames;

    if( newBufferLevel == cInternalPeriod )
    {
      assert( mInputBufferLevel == cInternalPeriod );
      SampleType const * procInputPtr{ mInputBuffer.getReadPointer(
          0, cInternalPeriod ) };
      SampleType * procOutputPtr{ mOutputBuffer.getFirstWritePointer() };
      mFlow.process( procInputPtr, mInputBuffer.stride(), 1, procOutputPtr,
                     mOutputBuffer.stride(), 1 );
      mOutputBuffer.commitWrite( cInternalPeriod );
      mInputBufferLevel -= cInternalPeriod;
      mOutputBufferLevel += cInternalPeriod;
    }

    SampleType const * readPtr =
        mOutputBuffer.getReadPointer( 0, mOutputBufferLevel );
    SampleType * outputWritePtr = playbackSamples + processedFrames;
    for( std::size_t chIdx{ 0 }; chIdx < numPlaybackChannels; ++chIdx )
    {
      copyBuffer( readPtr + chIdx * mOutputBuffer.stride(),
                  outputWritePtr + chIdx * playbackChannelStride, chunkFrames,
                  1 /*source stride*/, playbackSampleStride );
    }
    mOutputBufferLevel -= chunkFrames;
    processedFrames += chunkFrames;
    remainingFrames -= chunkFrames;
  }
}

} // namespace rrl
} // namespace visr
