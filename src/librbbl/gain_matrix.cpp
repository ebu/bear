/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "gain_matrix.hpp"

#include <libefl/vector_functions.hpp>

#include <algorithm>
#include <limits>

namespace visr
{
namespace rbbl
{

template< typename ElementType >
GainMatrix<ElementType>::GainMatrix( std::size_t numberOfInputs,
                        std::size_t numberOfOutputs,
                        std::size_t blockLength,
                        std::size_t interpolationSteps,
                        ElementType initialValue /*= static_cast<ElementType>(0.0)*/,
                        std::size_t alignment /*= 0 */ )
 : mPreviousGains( numberOfOutputs, numberOfInputs, alignment )
 , mNextGains( numberOfOutputs, numberOfInputs, alignment )
 , mBlockSize( blockLength )
 , mAlignment( alignment )
 , mInterpolationCounter( 0 )
 , mFader( blockLength, interpolationSteps, alignment )
 {
  mPreviousGains.fillValue( initialValue );
  mNextGains.copy( mPreviousGains );
 }

template< typename ElementType >
GainMatrix<ElementType>::GainMatrix( std::size_t numberOfInputs,
                        std::size_t numberOfOutputs,
                        std::size_t blockLength,
                        std::size_t interpolationSteps,
                        efl::BasicMatrix<ElementType> const& initialMatrix,
                        std::size_t alignment /*= 0 */ )
 : GainMatrix( numberOfInputs, numberOfOutputs, blockLength, interpolationSteps, 0.0f, alignment )
{
  if( (initialMatrix.numberOfRows() != numberOfOutputs) or( initialMatrix.numberOfColumns() != numberOfInputs ) )
  {
    throw std::invalid_argument( "GainMatrix: The size of the passed initial matrix is incompatible with the dimension of the gain matrix." );
  }
  mPreviousGains.copy( initialMatrix );
  mNextGains.copy( initialMatrix );
}

template< typename ElementType >
GainMatrix<ElementType>::~GainMatrix( ) = default;

template< typename ElementType >
void GainMatrix<ElementType>::process( ElementType const * const * input, ElementType * const * output )
{
  processAudio( input, output );
  // advance the interpolation counter
  if( mInterpolationCounter < mFader.interpolationPeriods() )
  {
    ++mInterpolationCounter;
  }
}

template< typename ElementType >
void GainMatrix<ElementType>::process( ElementType const * const * input, ElementType * const * output,
                                       efl::BasicMatrix<ElementType> const& newGains )
{
  if( (newGains.numberOfRows() != mPreviousGains.numberOfRows())
       or (newGains.numberOfColumns() != mPreviousGains.numberOfColumns()) )
  {
    throw std::invalid_argument( "GainMatrix::process(): Dimension of new gain matrix does not match." );
  }
  setGainsInternal( newGains );
  process( input, output );
}

template< typename ElementType >
void GainMatrix<ElementType>::setNewGains( efl::BasicMatrix<ElementType> const& newGains )
{
  if( (newGains.numberOfRows() != mPreviousGains.numberOfRows())
       or (newGains.numberOfColumns() != mPreviousGains.numberOfColumns()) )
  {
    throw std::invalid_argument( "GainMatrix::setNewGains(): Dimension of new gain matrix does not match." );
  }
  setGainsInternal( newGains );
}

template< typename ElementType >
void GainMatrix<ElementType>::processAudio( ElementType const * const * input, ElementType * const * output )
{
  std::size_t const numInputs( mPreviousGains.numberOfColumns() );
  std::size_t const numOutputs( mPreviousGains.numberOfRows() );

  if( numInputs == 0 ) // Short cut, also avoids output zeroing for the other cases
  {
    for( std::size_t outputIdx( 0 ); outputIdx < numOutputs; ++outputIdx )
    {
      ElementType * const outVector = output[outputIdx];
      efl::ErrorCode res = efl::vectorZero( outVector, mBlockSize, mAlignment );
      if( res != efl::noError )
      {
        throw std::runtime_error( "GainMatrix::process(): Clearing of output vector failed." );
      }
    }
    return;
  }
  for( std::size_t outputIdx( 0 ); outputIdx < numOutputs; ++outputIdx )
  {
    ElementType * const outVector = output[outputIdx];
    mFader.scale( input[0], outVector, mPreviousGains( outputIdx, 0), mNextGains( outputIdx, 0 ), mInterpolationCounter );
    for( std::size_t inputIdx( 1 ); inputIdx < numInputs; ++inputIdx )
    {
      mFader.scaleAndAccumulate( input[inputIdx], outVector, mPreviousGains( outputIdx, inputIdx ), mNextGains( outputIdx, inputIdx ), mInterpolationCounter );
    }
  }
}

template< typename ElementType >
void GainMatrix<ElementType>::setGainsInternal( efl::BasicMatrix<ElementType> const & newGains )
{
  std::size_t const numInputs( mPreviousGains.numberOfColumns( ) );
  std::size_t const numOutputs( mPreviousGains.numberOfRows( ) );

  // two distinct cases:
  if( mInterpolationCounter >= mFader.interpolationPeriods() ) // previous transition is completed
  {
    mPreviousGains.swap( mNextGains );
  }
  else // previous transition is not yet complete.
  {
    // Set the previous gains according to the currently reached interpolation ratio.
    ElementType const ratio = std::min( static_cast<ElementType>(1.0),
      static_cast<ElementType>(mInterpolationCounter * mBlockSize ) / static_cast<ElementType>(mFader.interpolationSamples() ) );
    for( std::size_t outputIdx( 0 ); outputIdx < numOutputs; ++outputIdx )
    {
      for( std::size_t inputIdx( 0 ); inputIdx < numInputs; ++inputIdx )
      {
        // If the difference becomes to small, set the previous value to the
        // target value immediately.
        // This avoids a exponential creep towards the target value, where
        // repeated rounding might imply that the target value is not reached at
        // all. This also avoids problems (e.g., performance reduction) due to denormalised
        // floating-point values. 
        ElementType const newScale{ ratio * (mNextGains( outputIdx, inputIdx ) - mPreviousGains( outputIdx, inputIdx )) };
        if( std::abs( newScale ) >= std::numeric_limits<ElementType>::epsilon() )
        {
          mPreviousGains( outputIdx, inputIdx ) += newScale;
        }
        else
        {
          mPreviousGains( outputIdx, inputIdx ) = mNextGains( outputIdx, inputIdx );
        }
      }
    }
  }
  mNextGains.copy( newGains );
  mInterpolationCounter = 0;
}

// explicit instantiations
template class GainMatrix<float>;
template class GainMatrix<double>;

} // namespace rbbl
} // namespace visr
