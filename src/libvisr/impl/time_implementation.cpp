/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "time_implementation.hpp"

#include <libvisr/signal_flow_context.hpp>

namespace visr
{
namespace impl
{
TimeImplementation::
TimeImplementation( visr::SignalFlowContext const & context )
 : mPeriod( context.period() )
 , mSamplingFrequency( context.samplingFrequency() )
 , mSamplingFrequencyInverse( static_cast<ContinuousTimeType>(1.0)
    / static_cast<ContinuousTimeType>(context.samplingFrequency()))
{
}

TimeImplementation::~TimeImplementation() = default;
    
Time::IntegerTimeType TimeImplementation::blockCount() const
{
  return mBlockCounter;
}
    
Time::IntegerTimeType TimeImplementation::sampleCount() const
{
  return mBlockCounter * mPeriod;
}

void TimeImplementation::resetCounter()
{
  mBlockCounter = 0;
}

void TimeImplementation::advanceBlockCounter()
{
  ++mBlockCounter;
}

TimeImplementation::ContinuousTimeType
TimeImplementation::renderingTime() const
{
  return static_cast<ContinuousTimeType>(sampleCount()) * mSamplingFrequencyInverse;
}


} // namespace impl
} // namespace visr
