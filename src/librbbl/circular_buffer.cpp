/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "circular_buffer.hpp"

#include <libefl/alignment.hpp>
#include <libefl/vector_functions.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

namespace visr
{
namespace rbbl
{

/**
 * Common part of the private implementation object.
 *
 */
template< typename DataType >
class CircularBuffer<DataType>::Impl
{
  // friend class CircularBuffer<DataType>;
public:
  Impl( std::size_t numberOfChannels, std::size_t length, std::size_t alignment )
    : mAllocatedLength( efl::nextAlignedSize( length, alignment ) )
    , mBuffer( numberOfChannels, 2 * mAllocatedLength, alignment )
  {
  }

  std::size_t allocatedSize() const
  {
    return mBuffer.numberOfColumns() / 2;
  }

  std::size_t stride() const
  {
    return mBuffer.stride();
  }

  DataType* basePointer() { return mBuffer.data(); }

  std::size_t advanceWriteIndex( std::size_t currentWriteIndex, std::size_t advanceSamples );
private:
  std::size_t const mAllocatedLength;
  efl::BasicMatrix<DataType> mBuffer;
};

template <typename DataType >
std::size_t CircularBuffer<DataType>::Impl::advanceWriteIndex( std::size_t currentWriteIndex, std::size_t advanceSamples )
{
  std::size_t const numChannels = mBuffer.numberOfRows();

  assert( currentWriteIndex < mAllocatedLength );
  // Write into the shadow copy.
  // Note that this operation can hit the wraparound boundary, so the write might be split into two parts

  // Write the first block.
  // TODO: be aware of one-off errors.
  std::size_t const firstBlockEnd = std::min( currentWriteIndex + advanceSamples, mAllocatedLength );
  std::size_t const samplesInFirstBlock = firstBlockEnd - currentWriteIndex;

  std::size_t const alignmentFirstBlock = 1; // ATM, we don't make assumptions about the alignment of source and target.

  for( std::size_t chIdx( 0 ); chIdx < numChannels; ++chIdx )
  {
    efl::ErrorCode res = efl::vectorCopy( mBuffer.row( chIdx ) + currentWriteIndex, mBuffer.row( chIdx ) + currentWriteIndex + mAllocatedLength,
                                          samplesInFirstBlock, alignmentFirstBlock );
    if( res != efl::noError )
    {
      throw std::invalid_argument( "CircularBuffer::write( ): Copying of data vector failed." );
    }
  }
  if( samplesInFirstBlock < advanceSamples ) // Do we need a second write due to wraparound?
  {
    std::size_t const remainingSamples = advanceSamples - samplesInFirstBlock;
    std::size_t alignmentSecondBlock = 1; // ATM, we don't make assumptions about the alignment of source and target.
    for( std::size_t chIdx( 0 ); chIdx < numChannels; ++chIdx )
    {
      efl::ErrorCode res = efl::vectorCopy( mBuffer.row( chIdx ) + mAllocatedLength,
                                            mBuffer.row( chIdx ),
                                            remainingSamples, alignmentSecondBlock );
      if( res != efl::noError )
      {
        throw std::invalid_argument( "CircularBuffer::write( ): Copying of data vector failed." );
      }
    }
  }
  return (currentWriteIndex + advanceSamples ) % mAllocatedLength;
}

template< typename DataType >
CircularBuffer<DataType>::CircularBuffer( std::size_t numberOfChannels, std::size_t length, std::size_t alignment /*= 0*/ )
 : mImpl( new Impl( numberOfChannels, length, alignment ) )
 , mLength( length )
 , mAllocatedLength( mImpl->allocatedSize() )
 , mStride( mImpl->stride() )
 , mNumberOfChannels( numberOfChannels )
 , mBasePointer( mImpl->basePointer() )
 , mWriteHeadIndex( 0 )
{
}

template< typename DataType >
CircularBuffer<DataType>::~CircularBuffer()
{
}

template< typename DataType >
void CircularBuffer<DataType>::write( DataType const * const * writeData,
                                      std::size_t numberOfChannels,
                                      std::size_t numberOfSamples,
                                      std::size_t alignmentElements /*= 0*/ )
{
  // TODO: Check minimum alignment, also taking into account the the write head position and the stride.
  std::size_t minAlignment = std::min<std::size_t>( alignmentElements, 1 ); // for now, use the worst-case alignment.

  if( numberOfChannels != mNumberOfChannels )
  {
    throw std::invalid_argument( "CircularBuffer::write(): The number of data channels differs from the number of channels in the circular buffer" );
  }
  if( numberOfSamples > mLength )
  {
    throw std::invalid_argument( "CircularBuffer::write(): The number samples to be written exceeds the length of the circular buffer" );
  }

  for( std::size_t chIdx( 0 ); chIdx < mNumberOfChannels; ++chIdx )
  {
    DataType * writePtr = mBasePointer + chIdx * mStride + mWriteHeadIndex;
    efl::ErrorCode res = efl::vectorCopy( writeData[ chIdx ], writePtr, numberOfSamples, minAlignment );
    if( res != efl::noError )
    {
      throw std::invalid_argument( "CircularBuffer::write( ): Copying of data vector failed." );
    }
  }
  mWriteHeadIndex = mImpl->advanceWriteIndex( mWriteHeadIndex, numberOfSamples );
}

template< typename DataType >
void CircularBuffer<DataType>::write( DataType const * writeData,
                                      std::size_t channelStrideElements,
                                      std::size_t numberOfChannels,
                                      std::size_t numberOfSamples,
                                      std::size_t alignmentElements /*= 0*/ )
{
  // TODO: Check minimum alignment, also taking into account the the write head position and the stride.
  std::size_t minAlignment = std::min<std::size_t>( alignmentElements, 1 ); // for now, use the worst-case alignment.

  if( numberOfChannels != mNumberOfChannels )
  {
    throw std::invalid_argument( "CircularBuffer::write(): The number of data channels differs from the number of channels in the circular buffer" );
  }
  if( numberOfSamples > mLength )
  {
    throw std::invalid_argument( "CircularBuffer::write(): The number samples to be written exceeds the length of the circular buffer" );
  }
  // TODO: Consider using a 'strided' copy function.
  for( std::size_t chIdx( 0 ); chIdx < mNumberOfChannels; ++chIdx )
  {
    DataType const * srcPtr = writeData + chIdx * channelStrideElements;
    DataType * destPtr = mBasePointer + chIdx * mStride + mWriteHeadIndex;
    efl::ErrorCode res = efl::vectorCopy( srcPtr, destPtr, numberOfSamples, minAlignment );
    if( res != efl::noError )
    {
      throw std::invalid_argument( "CircularBuffer::write( ): Copying of data vector failed." );
    }
  }
  mWriteHeadIndex = mImpl->advanceWriteIndex( mWriteHeadIndex, numberOfSamples );
}

template< typename DataType >
void CircularBuffer<DataType>::write( efl::BasicMatrix<DataType> const & writeData )
{
  // TODO: Check minimum alignment, also taking into account the the write head position and the stride.
  std::size_t minAlignment = std::min<std::size_t>( writeData.alignmentElements(), 1 ); // for now, use the worst-case alignment.

  if( writeData.numberOfRows() != numberOfChannels() )
  {
    throw std::invalid_argument("CircularBuffer::write(): The number of matrix rows differs from the number of channels in the circular buffer");
  }
  std::size_t const numberOfElements = writeData.numberOfColumns( );
  if( numberOfElements > mLength )
  {
    throw std::invalid_argument("CircularBuffer::write(): The number samples exceeds the length of the circular buffer");
  }

  for( std::size_t chIdx( 0 ); chIdx < mNumberOfChannels; ++chIdx )
  {
    DataType * writePtr = mBasePointer + chIdx * mStride + mWriteHeadIndex;
    efl::ErrorCode res = efl::vectorCopy( writeData.row( chIdx ), writePtr, numberOfElements, minAlignment );
    if( res != efl::noError )
    {
      throw std::invalid_argument( "CircularBuffer::write( ): Copying of data vector failed." );
    }
  }
  mWriteHeadIndex = mImpl->advanceWriteIndex( mWriteHeadIndex, numberOfElements );
}

template< typename DataType >
DataType * CircularBuffer<DataType>::getFirstWritePointer()
{
  return mBasePointer + mWriteHeadIndex;
}

template< typename DataType >
void CircularBuffer<DataType>::getWritePointers( DataType * * writePointers )
{
  for( std::size_t chIdx(0); chIdx < mNumberOfChannels; ++chIdx )
  {
    writePointers[chIdx] = mBasePointer + chIdx * mStride + mWriteHeadIndex;
  }
}

template< typename DataType >
void CircularBuffer<DataType>::commitWrite( std::size_t numberOfElementsWritten )
{
  if( numberOfElementsWritten > length() )
  {
    throw std::invalid_argument( "The number of committed writen samples must not exceed the length of the circular buffer." );
  }
  mWriteHeadIndex = mImpl->advanceWriteIndex( mWriteHeadIndex, numberOfElementsWritten );
}

template< typename DataType >
void CircularBuffer<DataType>::getReadPointers( DelayIndexType delay, DataType const * * readPointers ) const
{
  if( delay >= length() )
  {
    throw std::invalid_argument( "CircularBuffer::getReadPointers(): Delay value exceeds length of the delay line." );
  }
  for( std::size_t chIdx( 0 ); chIdx < mNumberOfChannels; ++chIdx )
  {
    readPointers[chIdx] = (mBasePointer + chIdx * mStride + mAllocatedLength + mWriteHeadIndex) - delay;
  }
}

template< typename DataType >
void CircularBuffer<DataType>::getReadPointers( DelayIndexType const * delay, DataType const * * readPointers ) const
{
  for( std::size_t chIdx( 0 ); chIdx < mNumberOfChannels; ++chIdx )
  {
    if( delay[chIdx] >= length( ) )
    {
      throw std::invalid_argument( "CircularBuffer::getReadPointers(): Delay value exceeds length of the delay line." );
    }
    readPointers[chIdx] = (mBasePointer + chIdx * mStride + mAllocatedLength + mWriteHeadIndex) - delay[chIdx];
  }
}

// Explicit template instantiations for standard sample types
template class CircularBuffer<float>;
template class CircularBuffer<double>;

} // namespace rbbl
} // namespace visr
