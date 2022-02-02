/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_IMPL_TIME_IMPLEMENTATION_HPP_INCLUDED
#define VISR_IMPL_TIME_IMPLEMENTATION_HPP_INCLUDED

#include <libvisr/time.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/export_symbols.hpp>

#include <cstddef>

namespace visr
{
class SignalFlowContext;

namespace impl
{

/**
 *
 *
 */
class VISR_CORE_LIBRARY_SYMBOL TimeImplementation
{
public:
  using ContinuousTimeType = Time::ContinuousTimeType;

  TimeImplementation( visr::SignalFlowContext const & context );

  ~TimeImplementation();
    
  TimeImplementation() = delete;
  
  TimeImplementation( TimeImplementation const & ) = delete;
    
  TimeImplementation( TimeImplementation && ) = delete;

  Time::IntegerTimeType blockCount() const;
    
  Time::IntegerTimeType sampleCount() const;
    
  ContinuousTimeType renderingTime() const;
    
  void resetCounter();
    
  void advanceBlockCounter();

private:
  std::size_t const mPeriod;
    
  visr::SamplingFrequencyType const mSamplingFrequency;
    
  ContinuousTimeType const mSamplingFrequencyInverse;
    
  std::size_t mBlockCounter;
};

} // namespace impl
} // namespace visr

#endif // #ifndef VISR_IMPL_TIME_IMPLEMENTATION_HPP_INCLUDED
