/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "runtime_profiler.hpp"

#include "audio_signal_flow.hpp"

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>
#include <libefl/error_codes.hpp>
#include <libefl/vector_functions.hpp>

#include <libvisr/atomic_component.hpp>

//#include <iosfwd>
#include <limits>
#include <map>
#include <memory>
//#include <set>
#include <string>
#include <vector>

namespace visr
{
namespace rrl
{

RuntimeProfiler::RuntimeProfiler( AudioSignalFlow const & flow,
                                  std::size_t measureBufferSize )
: mFlow{ flow }
, cNumberOfAtomicComponents{ flow.mProcessingSchedule.size() }
, mMeasurementSampleCounter{ 0 }
, mStatisticsSampleCounter{ 0 }
, mCurrentData( cNumberOfAtomicComponents, visr::cVectorAlignmentSamples )
, mCurrentMeasurementBuffer( measureBufferSize,
   cNumberOfAtomicComponents, visr::cVectorAlignmentSamples )
, mBackMeasurementBuffer( measureBufferSize,
   cNumberOfAtomicComponents, visr::cVectorAlignmentSamples )
, mRunningMean( cNumberOfAtomicComponents, visr::cVectorAlignmentSamples )
, mRunningM2( cNumberOfAtomicComponents, visr::cVectorAlignmentSamples )
, mTmpBuffer( cNumberOfAtomicComponents, visr::cVectorAlignmentSamples )
, mTmpBuffer2( cNumberOfAtomicComponents, visr::cVectorAlignmentSamples )
{
}

RuntimeProfiler::~RuntimeProfiler() = default;

std::size_t RuntimeProfiler::numberOfComponents() const
{
  return cNumberOfAtomicComponents;
}

void RuntimeProfiler::setTime( std::size_t componentIdx, TimeType timing )
{
  if( componentIdx >= numberOfComponents() )
  {
    throw std::out_of_range( "Index exceeds number of components." );
  }
  mCurrentData[ componentIdx ] = timing;
}
  
void RuntimeProfiler::finishIteration()
{
  // Statistics that do not involve saving raw timings.
  if( mStatisticsSampleCounter == 0 )
  {
    efl::ErrorCode res;
    // Only mean can be computed in the first iteration.
    if( (res = efl::vectorCopy( mCurrentData.data(),
      mRunningMean.data(), numberOfComponents(),
      visr::cVectorAlignmentSamples )) != efl::noError )
    {
      throw std::runtime_error( "Error updating profiling data: raw timings." );
    }
  }
  else
  {
    // Welberg's algorithm assumes that the count is already
    // incremented.
    TimeType const invCount = static_cast<TimeType>(1.0)
      / static_cast<TimeType>( mStatisticsSampleCounter + 1 );
    //  delta = newValue - mean
    efl::ErrorCode res;
    if( (res = efl::vectorSubtract( mCurrentData.data(), mRunningMean.data(),
      mTmpBuffer.data(), numberOfComponents(),
      visr::cVectorAlignmentSamples )) != efl::noError )
    {
      throw std::runtime_error( "Error updating profiling data: mean." );
    }
    //  mean += delta / count
    if( (res = efl::vectorMultiplyConstant( invCount, mTmpBuffer.data(),
      mTmpBuffer2.data(), numberOfComponents(),
      visr::cVectorAlignmentSamples )) != efl::noError )
    {
      throw std::runtime_error( "Error updating profiling data: mean." );
    }
    // Compute the updated mean.
    if( (res = efl::vectorAddInplace( mTmpBuffer2.data(), mRunningMean.data(),
      numberOfComponents(), visr::cVectorAlignmentSamples )) != efl::noError )
    {
      throw std::runtime_error( "Error updating profiling data: mean." );
    }
    //  delta2 = newValue - mean
    if( (res = efl::vectorSubtract( mCurrentData.data(), mRunningMean.data(),
      mTmpBuffer2.data(), numberOfComponents(),
      visr::cVectorAlignmentSamples )) != efl::noError )
    {
      throw std::runtime_error( "Error updating profiling data: variance." );
    }
    //  M2 += delta * delta2
    if( (res = efl::vectorMultiplyAddInplace( mTmpBuffer.data(),
      mTmpBuffer2.data(), mRunningM2.data(), numberOfComponents(),
      visr::cVectorAlignmentSamples )) != efl::noError )
    {
      throw std::runtime_error( "Error updating profiling data: variance." );
    }
  }
  
  if( measurementBufferSize() )
  {
    std::size_t const currentRowIdx = mMeasurementSampleCounter % measurementBufferSize();
    efl::ErrorCode res;
    if( (res = efl::vectorCopy( mCurrentData.data(),
      mCurrentMeasurementBuffer.row(currentRowIdx), numberOfComponents(),
      visr::cVectorAlignmentSamples )) != efl::noError )
    {
      throw std::runtime_error( "Error updating profiling data: raw timings." );
    }
  }
  ++mStatisticsSampleCounter;
  ++mMeasurementSampleCounter;
}
  
void RuntimeProfiler::resetMeasurements()
{
  std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
    mFlow.parameterExchangeMutex() };
  mMeasurementSampleCounter = 0;
}

std::size_t RuntimeProfiler::
getStatistics( MeasurementVector & mean,
  MeasurementVector & variance ) const
{
  if( mean.size() != numberOfComponents() )
  {
    throw std::invalid_argument( "Size of \"mean\" buffer does not match number of profiled components." );
  }
  if( variance.size() != numberOfComponents() )
  {
    throw std::invalid_argument( "Size of \"variance\" buffer does not match number of profiled components." );
  }
  {
    std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
      mFlow.parameterExchangeMutex() };
    mean.copy( mRunningMean );
    calculateVariance( variance );
    return mStatisticsSampleCounter;
  }
}

std::size_t RuntimeProfiler::
getAndResetStatistics( MeasurementVector & mean,
  MeasurementVector & variance )
{
  if( mean.size() != numberOfComponents() )
  {
    throw std::invalid_argument( "Size of \"mean\" buffer does not match number of profiled components." );
  }
  if( variance.size() != numberOfComponents() )
  {
    throw std::invalid_argument( "Size of \"variance\" buffer does not match number of profiled components." );
  }
  {
    std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
      mFlow.parameterExchangeMutex() };
    std::size_t const numCycles = mStatisticsSampleCounter;
    mean.copy( mRunningMean );
    calculateVariance( variance );
    mStatisticsSampleCounter = 0;
    mRunningMean.zeroFill();
    mRunningM2.zeroFill();
    return numCycles;
  }
}

void RuntimeProfiler::resetStatistics()
{
  std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
    mFlow.parameterExchangeMutex() };
  mStatisticsSampleCounter = 0;
  mRunningMean.zeroFill();
  mRunningM2.zeroFill();  
}

std::size_t RuntimeProfiler::measurementBufferSize() const
{
  return mCurrentMeasurementBuffer.numberOfRows();
}

std::size_t RuntimeProfiler::measurementSamples() const
{
  std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
    mFlow.parameterExchangeMutex() };
  return measurementSamplesInternal();
}


std::size_t RuntimeProfiler::statisticsSamples() const
{
  std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
    mFlow.parameterExchangeMutex() };
  return mStatisticsSampleCounter;
}
  
std::vector< std::string >
RuntimeProfiler::getComponentNames() const
{
  std::vector< std::string > res;
  res.reserve( numberOfComponents() );
  for( auto const & component : mFlow.mProcessingSchedule )
  {
    res.push_back( component->fullName() );
  }
  return res;
}
  
void RuntimeProfiler::
writeComponentNames( std::ostream & os, std::string const & separator ) const
{
  for( auto compIt{ mFlow.mProcessingSchedule.begin()};
    compIt != mFlow.mProcessingSchedule.end(); )
  {
    os << (*compIt)->fullName();
    ++compIt;
    if( compIt == mFlow.mProcessingSchedule.end() )
    {
      break;
    }
    os << separator;
  }
}

RuntimeProfiler::MeasurementBuffer const &
RuntimeProfiler::getMeasurements( std::size_t & numIterations )
{
  std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
    mFlow.parameterExchangeMutex() };
  numIterations = measurementSamplesInternal();
  mBackMeasurementBuffer.swap( mCurrentMeasurementBuffer );
  mMeasurementSampleCounter = 0;
  return mBackMeasurementBuffer;
}

void RuntimeProfiler::getMean( efl::BasicVector<TimeType> & val ) const
{
  if( val.size() != numberOfComponents() )
  {
    throw std::invalid_argument( "Return value buffer has wrong size." );
  }
  if( mStatisticsSampleCounter < 1 )
  {
    val.fillValue( std::numeric_limits<TimeType>::quiet_NaN() );
  }
  {
    std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
    mFlow.parameterExchangeMutex() };
    val.copy( mRunningMean );
  }
}

void RuntimeProfiler::getVariance( efl::BasicVector<TimeType> & val ) const
{
  if( val.size() != numberOfComponents() )
  {
    throw std::invalid_argument( "Return value buffer has wrong size." );
  }
  else
  {
    std::lock_guard< AudioSignalFlow::ParameterExchangeMutexType > guard{
      mFlow.parameterExchangeMutex() };
    calculateVariance( val );
  }
}

void RuntimeProfiler::calculateVariance( MeasurementVector & ret ) const
{
  if( mStatisticsSampleCounter < 2 )
  {
    ret.fillValue( std::numeric_limits<TimeType>::quiet_NaN() );
  }
  else
  {
    // TODO: decide whether we need a lock guard here.  
    // M2 / (count - 1)
    TimeType const scaleFactor = 1.0f / static_cast<TimeType>(
      mStatisticsSampleCounter - 1 );
    efl::ErrorCode const res = efl::vectorMultiplyConstant( scaleFactor,
      mRunningM2.data(), ret.data(), numberOfComponents(),
      0 /*we don't know the alignment of val */ );
    if( res != efl::noError )
    {
      throw std::runtime_error( "Error computing variance." );
    }
  }
}

RuntimeProfiler::MeasurementVector const & 
RuntimeProfiler::currentData() const
{
  return mCurrentData;
}

RuntimeProfiler::MeasurementVector & 
RuntimeProfiler::currentData()
{
  return mCurrentData;
}

} // namespace rrl
} // namespace visr
