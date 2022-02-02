/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "multichannel_delay_line.hpp"

#include <librbbl/circular_buffer.hpp>
#include <librbbl/fractional_delay_factory.hpp>

#include <cassert>
#include <cmath>
#include <complex>
#include <initializer_list>
#include <limits>
#include <map>
#include <memory>
#include <vector>

namespace visr
{
namespace rbbl
{

template< typename SampleType >
MultichannelDelayLine<SampleType>::MultichannelDelayLine( std::size_t numberOfChannels,
                                                          SamplingFrequencyType samplingFrequency,
                                                          std::size_t blockLength,
                                                          SampleType maxDelaySeconds,
                                                          char const * interpolationMethod,
                                                          MethodDelayPolicy methodDelayPolicy,
                                                          std::size_t alignment /*= 0*/ )
  : cBlockLength( blockLength )
  , cSamplingFrequency( static_cast<SampleType>(samplingFrequency) )
  , cMaxDelaySamples( std::ceil( maxDelaySeconds * samplingFrequency ) )
  , cMethodDelayPolicy( methodDelayPolicy )
  , mRingbuffer( numberOfChannels, static_cast<std::size_t>(cMaxDelaySamples)+blockLength, alignment ) // Ideally this would include the implementation delay.
  , mInterpolator( FractionalDelayFactory<SampleType>::create( interpolationMethod, blockLength ) )
  , cMethodDelay( mInterpolator ? mInterpolator->methodDelay() : static_cast<SampleType>(0.0) )
{
  if( not mInterpolator )
  {
    throw std::invalid_argument( "MultichannelDelayLine: interpolator type not implemented.");
  }
}

template< typename SampleType >
MultichannelDelayLine<SampleType>::~MultichannelDelayLine() = default;

template< typename SampleType >
std::size_t MultichannelDelayLine<SampleType>::numberOfChannels() const
{
  return mRingbuffer.numberOfChannels();
}

template< typename SampleType >
std::size_t MultichannelDelayLine<SampleType>::blockLength() const
{
  return cBlockLength;
}

template< typename SampleType >
SampleType MultichannelDelayLine<SampleType>::methodDelaySeconds() const
{
  return cMaxDelaySamples / cSamplingFrequency;
}

template< typename SampleType >
SampleType MultichannelDelayLine<SampleType>::methodDelaySamples() const
{
  return cMaxDelaySamples;
}

template< typename SampleType >
void MultichannelDelayLine<SampleType>::write( SampleType const * input,
                                               std::size_t channelStride,
                                               std::size_t pNumberOfChannels,
                                               std::size_t alignment )
{
  if( pNumberOfChannels != numberOfChannels() )
  {
    throw std::invalid_argument( "MultichannelDelayLine::write(): the number of channels passed differs from the channel number of the delay line." );
  }
  mRingbuffer.write( input, channelStride, pNumberOfChannels, cBlockLength, alignment );
}

template< typename SampleType >
void MultichannelDelayLine<SampleType>::interpolate( SampleType * output,
                                                     std::size_t channelIdx,
                                                     std::size_t numberOfSamples,
                                                     SampleType startDelay, SampleType endDelay,
                                                     SampleType startGain, SampleType endGain )
{
  if( channelIdx >= numberOfChannels() )
  {
    throw std::invalid_argument( "MultichannelDelayLine<SampleType>::interpolate()" );
  }

  mInterpolator->interpolate( mRingbuffer.getReadPointer(channelIdx, 1 ), // The '0' position is one past the the last written sample, so we have to get the '1' position.
    output,
    numberOfSamples,
    adjustDelay(startDelay), adjustDelay(endDelay),
    startGain, endGain );
}

template <typename SampleType >
SampleType MultichannelDelayLine<SampleType>::
adjustDelay( SampleType rawDelay ) const
{
  rawDelay *= cSamplingFrequency; // Scale from seconds to samples.
  if( rawDelay > cMaxDelaySamples )
  {
    throw std::out_of_range( "LagrangeInterpolator::interpolate(): Delay exceeds the maximum admissible value." );
  }
  switch( cMethodDelayPolicy )
  {
    case MethodDelayPolicy::Add:
      if( rawDelay < - std::numeric_limits<SampleType>::epsilon() )
      {
        throw std::out_of_range( "LagrangeInterpolator::interpolate(): Delay is negative while the \"Add\" strategy is selected." );
      }
      return rawDelay; // method delay is added automatically
    case MethodDelayPolicy::Limit:
      return std::max( rawDelay - cMethodDelay, static_cast<SampleType>(0.0) );
    case MethodDelayPolicy::Reject:
      if( rawDelay < cMethodDelay )
      {
        throw std::out_of_range( "LagrangeInterpolator::interpolate(): Delay is smaller than the method delay and the \"Reject\" strategy is selected." );
      }
      return rawDelay - cMethodDelay;
  }
  throw std::logic_error( "LagrangeInterpolator::interpolate(): Unhandled method delay adjustment policy type" );
}


// Explicit instantiations
template class MultichannelDelayLine<float>;
template class MultichannelDelayLine<double>;

} // namespace rbbl
} // namespace visr
