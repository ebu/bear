/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "lagrange_interpolator.hpp"

#include <libvisr/detail/compose_message_string.hpp>

#include <array>
#include <cmath>
#include <numeric>

namespace visr
{
namespace rbbl
{
template <typename SampleType, std::size_t order >
LagrangeInterpolator<SampleType, order >::LagrangeInterpolator( std::size_t maxNumSamples,
                                                                std::size_t alignmentElements )
  : mDelays(  maxNumSamples, alignmentElements )
  , mGains( maxNumSamples, alignmentElements )
{
}

template <typename SampleType, std::size_t order >
LagrangeInterpolator<SampleType, order>::~LagrangeInterpolator() = default;

template <typename SampleType, std::size_t order >
SampleType LagrangeInterpolator<SampleType, order>::methodDelay() const
{
  return cMethodDelay;
}

template <typename SampleType, std::size_t order >
void LagrangeInterpolator<SampleType, order>::interpolate( SampleType const * basePointer,
                                                           SampleType * result,
                                                           std::size_t numSamples,
                                                           SampleType startDelay, SampleType endDelay,
                                                           SampleType startGain, SampleType endGain )
{
  if( numSamples > mDelays.size() )
  {
    throw std::invalid_argument( "LagrangeInterpolator::interpolate(): number of elements exceeds maximum admissible number.");
  }

  efl::ErrorCode res = efl::vectorRamp( mGains.data(), numSamples,
                                        startGain, endGain,
                                        false/*startInclusive*/, true/*endInclusive*/,
                                        mGains.alignmentElements() );
  if( res != efl::noError )
  {
    throw std::runtime_error( detail::composeMessageString( "LagrangeInterpolator::interpolate(): Error during gain ramp computation:",
                                                            efl::errorMessage(res)) );
  }

  SampleType const adjustedStartTimeOffset = -static_cast<SampleType>(numSamples + order) - startDelay;
  SampleType const adjustedEndTimeOffset = -static_cast<SampleType>(order) - endDelay;

  res = efl::vectorRamp( mDelays.data(), numSamples,
                         adjustedStartTimeOffset,
                         adjustedEndTimeOffset,
                         false/*startInclusive*/, true/*endInclusive*/,
                         mDelays.alignmentElements() );
  if( res != efl::noError )
  {
    throw std::runtime_error( detail::composeMessageString( "LagrangeInterpolator::interpolate(): Error during delay ramp computation:",
                                                            efl::errorMessage(res)) );
  }

  std::array<SampleType, order+1> lagrangeCoefficients;
  for( std::size_t sampleIdx( 0 ); sampleIdx < numSamples; ++sampleIdx )
  {
    SampleType delay = mDelays[sampleIdx];
    // does the same thing as std::round, minus some edge cases that aren't
    // important to us
    std::ptrdiff_t baseOffset = delay > 0
                                     ? delay + static_cast<SampleType>(0.5)
                                     : delay - static_cast<SampleType>(0.5);
    SampleType intersamplePosition = delay - baseOffset;

    mCoeffCalculator.calculateCoefficients( intersamplePosition,
                                            &lagrangeCoefficients[0] );
    result[sampleIdx] = mGains[sampleIdx] * std::inner_product( lagrangeCoefficients.rbegin(), lagrangeCoefficients.rend(),
                                                                basePointer + baseOffset,
                                                                static_cast<SampleType>(0.0) );
  }
}

// Explicit instantiations
template class LagrangeInterpolator<float, 0>;
template class LagrangeInterpolator<float, 1>;
template class LagrangeInterpolator<float, 2>;
template class LagrangeInterpolator<float, 3>;
template class LagrangeInterpolator<float, 4>;
template class LagrangeInterpolator<float, 5>;
template class LagrangeInterpolator<float, 6>;
template class LagrangeInterpolator<float, 7>;
template class LagrangeInterpolator<float, 8>;
template class LagrangeInterpolator<float, 9>;

template class LagrangeInterpolator<double, 0>;
template class LagrangeInterpolator<double, 1>;
template class LagrangeInterpolator<double, 2>;
template class LagrangeInterpolator<double, 3>;
template class LagrangeInterpolator<double, 4>;
template class LagrangeInterpolator<double, 5>;
template class LagrangeInterpolator<double, 6>;
template class LagrangeInterpolator<double, 7>;
template class LagrangeInterpolator<double, 8>;
template class LagrangeInterpolator<double, 9>;

} // namespace rbbl
} // namespace visr
