/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "panning_gain_matrix.hpp"

#include <libefl/vector_functions.hpp>

#include <libpml/matrix_parameter.hpp>

#include <algorithm>
#include <cassert>
#include <ciso646>
#include <stdexcept>

namespace visr
{
namespace panningdsp
{

namespace // unnamed
{

template< typename T >
efl::AlignedArray< T > constantArray( T val, std::size_t size, std::size_t alignment )
{
  efl::AlignedArray< T > ret{ size, alignment };
  std::fill( ret.data(), ret.data() + size, val );
  return ret;
}

template< typename T >
efl::BasicMatrix< T > zeroMatrix( std::size_t rows, std::size_t cols, std::size_t alignment )
{
  efl::BasicMatrix< T > ret{ rows, cols , alignment };
  ret.zeroFill(); // Don't trust the initialisation.
  return ret;
}

template< typename T >
efl::BasicVector< T > createRamp( T start, T inc, std::size_t size, std::size_t alignment )
{
  efl::BasicVector< T > ret( size, alignment );
  for( std::size_t idx{0}; idx < size; ++idx )
  {
    ret[ idx ] = start + static_cast< T >( idx ) * inc;
  }
  return ret;
}

} // unnamed

PanningGainMatrix::PanningGainMatrix(SignalFlowContext const & context,
  char const * name,
  CompositeComponent * parent,
  std::size_t numberOfObjects,
  std::size_t numberOfLoudspeakers )
 : PanningGainMatrix{ context, name, parent,
    numberOfObjects, numberOfLoudspeakers,
    zeroMatrix<SampleType>( numberOfObjects, numberOfLoudspeakers , cVectorAlignmentSamples ) }
{}

PanningGainMatrix::PanningGainMatrix( SignalFlowContext const & context,
  char const * name,
  CompositeComponent * parent,
  std::size_t numberOfObjects,
  std::size_t numberOfLoudspeakers,
  visr::efl::BasicMatrix<SampleType> const & initialGains )
 : AtomicComponent( context, name, parent )
 , mAudioInput( "in", *this, numberOfObjects )
 , mAudioOutput( "out", *this, numberOfLoudspeakers )
 , mGainInput( "gainInput", *this, pml::MatrixParameterConfig(  numberOfObjects, numberOfLoudspeakers ) )
 , mPreviousTime{ constantArray( size_t(0), numberOfObjects, visr::cVectorAlignmentSamples ) }
 , mTargetTime{ constantArray( cTimeStampInfinity, numberOfObjects, visr::cVectorAlignmentSamples ) }
 , mPreviousGains{ numberOfObjects, numberOfLoudspeakers, visr::cVectorAlignmentSamples }
 , mTargetGains{ numberOfObjects, numberOfLoudspeakers, visr::cVectorAlignmentSamples }
 , mPendingTransitions( cNumPendingTransitions,
       PanningMatrixParameter( numberOfObjects, numberOfLoudspeakers, 
       visr::cVectorAlignmentSamples ) )
 , mScalingRamp{ createRamp( static_cast<SampleType>(1.0), 
    static_cast<SampleType>(1.0),
    period(), visr::cVectorAlignmentSamples )}
{
  if( (initialGains.numberOfRows() != numberOfObjects) 
   or (initialGains.numberOfColumns() != numberOfLoudspeakers) )
  {
    throw std::invalid_argument( "The parameter \"initialGains\" does not match"
      " the dimension numberOfObjects x numberOfLoudspeakers." );
  }
  mPreviousGains.copy( initialGains );
  mTargetGains.copy( initialGains );
}

PanningGainMatrix::PanningGainMatrix(SignalFlowContext const & context,
  char const * name,
  CompositeComponent * parent,
  std::size_t numberOfObjects,
  std::size_t numberOfLoudspeakers,
  efl::BasicMatrix<SampleType> const & initialGains,
  PanningMatrixParameter const & initialTransitions )
 : PanningGainMatrix{ context, name, parent,
    numberOfObjects, numberOfLoudspeakers, initialGains }
{
  updateTransitions( 0, initialTransitions );
}

PanningGainMatrix::~PanningGainMatrix() = default;

void PanningGainMatrix::process()
{
  processParameters();
  processAudio();
}

namespace // unnamed
{

using GainType = PanningGainMatrix::SampleType;

/**
 * Compute the slope value of currentTime between previousTime
 * and targetTime  
 */
GainType interpolationRatio( TimeType currentTime, 
    TimeType previousTime, TimeType targetTime )
{
  return (targetTime == cTimeStampInfinity )
    ? static_cast< GainType >( 0.0 ) 
    : static_cast<GainType>( currentTime - previousTime) /
    static_cast<GainType>( targetTime - previousTime );
}

GainType gainIncRatio( TimeType previousTime, TimeType targetTime )
{
  return (targetTime == cTimeStampInfinity )
    ? static_cast< GainType >( 0.0 ) 
    : static_cast< GainType >( 1.0 ) /
    static_cast<GainType>( targetTime - previousTime );
}

template< typename T >
void copyRow( T const * src, T * dest,
std::size_t numElements, std::size_t alignment = 0 )
{
  efl::ErrorCode const res = efl::vectorCopy( src,
    dest, numElements, alignment );
  if( res != efl::noError )
  {
    throw std::invalid_argument( visr::detail::composeMessageString(
      "Error while copying vector: ", efl::errorMessage( res) ) );
  }
}

} // unnamed namespace

void PanningGainMatrix::processAudio()
{
  std::size_t const numObjs{ mAudioInput.width() };
  std::size_t const numLsp{ mAudioOutput.width() };
  std::size_t const bufSize{ period() };
  if( numObjs == 0 )
  {
    efl::ErrorCode const res
      = efl::vectorZero( mAudioOutput.data(),
        numLsp * mAudioOutput.channelStrideSamples() );
    if( res != efl::noError )
    {
      status( StatusMessage::Error, "Error during zeroing of output signal: ",
        efl::errorMessage( res ) );
    }
    return;
  }

  // Time::IntegerTimeType const periodEndTime{ currentTime + bufSize };
  for( std::size_t objIdx{ 0 }; objIdx < numObjs; ++objIdx )
  {
    // Add to the output signals for all but the first (zeroth) object
    bool const accumulateFlag = (objIdx != 0);

    TimeType currentTime{ time().sampleCount() };
    TimeType const blockEndTime{ currentTime + bufSize };
    std::size_t bufferOffset{ 0 };

    while( currentTime < blockEndTime )
    {
      assert( mPreviousTime[ objIdx ] < mTargetTime[ objIdx ] );
      assert( currentTime >= mPreviousTime[ objIdx ] );
      assert( currentTime < mTargetTime[ objIdx ] );
      assert( mPendingTransitions[0].timeStamps()[objIdx] > currentTime );
      Time::IntegerTimeType const duration{ std::min( 
        mTargetTime[ objIdx ], blockEndTime ) - currentTime };
      assert( duration > 0 ); // Otherwise this is a logical error,
      processAudioSingleSlope( objIdx, currentTime, bufferOffset, duration, accumulateFlag, 
        0 /*Alignment not guaranteed for arbitrary offsets.*/  );

      currentTime += duration;
      bufferOffset += duration;
      if( currentTime == mTargetTime[objIdx] )
      {
        mPreviousTime[ objIdx ] = currentTime;
        TimeType const nextStartTime{ mPendingTransitions[0].timeStamps()[ objIdx ] };
        if( nextStartTime == currentTime ) // next transition starts immediately
        {
          TimeType const transitionTime{ mPendingTransitions[0].transitionTimes()[ objIdx ] };
          if( transitionTime == 0 )
          {
            copyRow( mPendingTransitions[0].gains().row( objIdx ),
                mPreviousGains.row( objIdx ),
                numLsp, visr::cVectorAlignmentSamples );
            mTargetTime[ objIdx ] = cTimeStampInfinity;
          }
          else
          {
            copyRow( mTargetGains.row( objIdx ), mPreviousGains.row( objIdx ),
                numLsp, visr::cVectorAlignmentSamples );
            mTargetTime[ objIdx ] = currentTime + transitionTime;
          }
          copyRow( mPendingTransitions[0].gains().row( objIdx ),
              mTargetGains.row( objIdx ),
              numLsp, visr::cVectorAlignmentSamples );
          mPendingTransitions[0].timeStamps()[ objIdx ] = cTimeStampInfinity;
          mPendingTransitions[0].transitionTimes()[ objIdx ] = cTimeStampInfinity;
        }
        else // There's a gap to the next transition
        {
          mTargetTime[ objIdx ] = nextStartTime;
          copyRow( mTargetGains.row( objIdx ), mPreviousGains.row( objIdx ),
              numLsp, visr::cVectorAlignmentSamples );
          // mTargetGains remain unchanged.

        }
      }
    } // while(true)
  }
}

void PanningGainMatrix::processAudioSingleSlope( std::size_t objIdx, TimeType currentTime, 
  std::size_t startIdx,
  std::size_t duration, bool accumulate, std::size_t alignment )
{
  std::size_t const numLsp{ mAudioOutput.width() };
  SampleType const startRatio = interpolationRatio( currentTime, mPreviousTime[objIdx], 
    mTargetTime[objIdx] );
  SampleType const incRatio = gainIncRatio( mPreviousTime[objIdx], 
    mTargetTime[objIdx] );
  for( std::size_t lspIdx{ 0 }; lspIdx < numLsp; ++lspIdx )
  {
    // Could be vectorised (but the gain matrix entries are non)
    SampleType const startGain = mPreviousGains( objIdx, lspIdx )
      + startRatio * (mTargetGains( objIdx, lspIdx ) - mPreviousGains( objIdx, lspIdx ) );
    SampleType const gainInc = incRatio
      * (mTargetGains( objIdx, lspIdx ) - mPreviousGains( objIdx, lspIdx ) );

    // Alignment is ensured because we start from the beginning of the input and output buffers.
    scaleSignal( mAudioInput[objIdx], mAudioOutput[lspIdx],
      startIdx, duration, startGain, gainInc, accumulate, alignment );
  }
}

void PanningGainMatrix::processParameters()
{
  if( mGainInput.changed() )
  {
    TimeType const currentTime{ time().sampleCount() };
    updateTransitions( currentTime, mGainInput.data() );
    mGainInput.resetChanged();
  }
}

void PanningGainMatrix::
updateTransitions(  TimeType currentTime,
    PanningMatrixParameter const & newParams )
{
  std::size_t const numObjs{ mTargetGains.numberOfRows() };
  for( std::size_t objIdx{ 0 }; objIdx < numObjs; ++objIdx )
  {
    updateTransition( objIdx, currentTime,
      newParams.timeStamps()[objIdx],
      newParams.transitionTimes()[objIdx],
      newParams.gains().row( objIdx ) );
  }
}

void PanningGainMatrix::updateTransition( std::size_t objIdx,
  TimeType currentTime,
  TimeType startTime,
  TimeType duration,
  SampleType const * gains )
{
  if( startTime == cTimeStampInfinity )
  {
    return;
  }
  if( startTime < currentTime )
  {
    status( StatusMessage::Error, "Time stamp of gain change is in the past." );
    return;
  }
  std::size_t const numLsp{ mTargetGains.numberOfColumns() };
  // Special case: we have to update the data members immediately.
  if( startTime == currentTime )
  {
    mPreviousTime[ objIdx ] = currentTime;
    if( duration == 0 ) // Immediate change w/o interpolation (e.g., jump position)
    {
      copyRow( gains, mPreviousGains.row( objIdx ), numLsp, 
        0 /*Do not assume special alignment*/ );
      copyRow( gains, mTargetGains.row( objIdx ), numLsp, 
        0 /*Do not assume special alignment*/ );
      mTargetTime[ objIdx ] = cTimeStampInfinity;
    }
    else
    {
      // Adjust the current target gains by computing the value at 
      // startTime
      GainType const alpha = interpolationRatio( currentTime,
          mPreviousTime[ objIdx ], mTargetTime[ objIdx ] );
      // TODO: turn into a vector function.
      for( std::size_t lspIdx{ 0 }; lspIdx < numLsp; ++lspIdx)
      {
        mPreviousGains( objIdx, lspIdx ) = alpha * mTargetGains( objIdx, lspIdx )
          + (1.0f - alpha ) * mPreviousGains( objIdx, lspIdx );
      }
      copyRow( gains, mTargetGains.row( objIdx ), numLsp,
        0 /*Do not assume special alignment*/ );
      mTargetTime[ objIdx ] = currentTime + duration;
    }
    // Clear next pending transition
    mPendingTransitions[0].timeStamps()[ objIdx ] = cTimeStampInfinity;
    mPendingTransitions[0].transitionTimes()[ objIdx ] = cTimeStampInfinity;
    copyRow( mTargetGains.row( objIdx ), mPendingTransitions[0].gains().row( objIdx ), numLsp );
  }
  else
  {
    if( startTime < mTargetTime[ objIdx ] )
    {
      // Adjust the current target gains by computing the value at 
      // startTime
      GainType const alpha = interpolationRatio( currentTime,
          mPreviousTime[ objIdx ], mTargetTime[ objIdx ] );
      std::size_t const numLsp{ mTargetGains.numberOfColumns() };
      // TODO: turn into a vector function.
      for( std::size_t lspIdx{ 0 }; lspIdx < numLsp; ++lspIdx)
      {
        mTargetGains( objIdx, lspIdx ) = alpha * mTargetGains( objIdx, lspIdx )
          + (1.0f - alpha ) * mPreviousGains( objIdx, lspIdx );
      }
      mTargetTime[ objIdx ] = startTime;
    }
    mPendingTransitions[0].timeStamps()[ objIdx ] = startTime;
    mPendingTransitions[0].transitionTimes()[ objIdx ] = duration;
    copyRow( gains, mPendingTransitions[0].gains().row( objIdx ),
      numLsp );
  }
}

void PanningGainMatrix::scaleSignal( SampleType const * input, SampleType * output,
    std::size_t startIdx, std::size_t duration,
    SampleType gainStart, SampleType gainInc, bool accumulate, std::size_t alignment )
{
  // SampleType const rampGain{ gainInc / static_cast< SampleType >( duration ) };
  efl::ErrorCode const res = efl::vectorRampScaling< SampleType >( input + startIdx,
    mScalingRamp.data(), output + startIdx,
    gainStart, gainInc, duration, accumulate, alignment );
  if( res != efl::noError )
  {
    status( StatusMessage::Error, "Error during audio signal matrixing: ",
      efl::errorMessage( res ) );
  }
}

} // namespace rcl
} // namespace visr
