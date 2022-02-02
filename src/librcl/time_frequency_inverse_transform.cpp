/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "time_frequency_inverse_transform.hpp"

#include <libefl/vector_functions.hpp>

#include <libpml/time_frequency_parameter.hpp>
#include <libpml/time_frequency_parameter_config.hpp>

#include <librbbl/circular_buffer.hpp>
#include <librbbl/fft_wrapper_base.hpp>
#include <librbbl/fft_wrapper_factory.hpp>

#include <boost/math/constants/constants.hpp>

#include <algorithm>
#include <ciso646>
#include <cmath>

namespace visr
{
namespace rcl
{
namespace // unnamed
{
template< typename T >
T calculateScalingFactor(
    TimeFrequencyInverseTransform::Normalisation desiredNormalisation,
    T actualDftScaling,
    std::size_t dftSize )
{
  using Normalisation = TimeFrequencyInverseTransform::Normalisation;
  T const desiredScaling =
      ( desiredNormalisation == Normalisation::One )
          ? 1.0f
          : ( ( desiredNormalisation == Normalisation::Unitary )
                  ? 1.0f / std::sqrt( static_cast< T >( dftSize ) )
                  : 1.0f / static_cast< SampleType >( dftSize ) );
  SampleType const scaleFactor = desiredScaling / actualDftScaling;
  return scaleFactor;
}

} // unnamed namespace

TimeFrequencyInverseTransform::TimeFrequencyInverseTransform(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    std::size_t numberOfChannels,
    std::size_t dftSize,
    std::size_t hopSize,
    char const * fftImplementation /* = "default"*/,
    Normalisation normalisation /* = Normalisation::OneOverN */ )
 : AtomicComponent( context, name, parent )
 , cAlignment( cVectorAlignmentSamples )
 , cNumberOfChannels( numberOfChannels )
 , cDftSize( dftSize )
 , cNumberOfDftBins(
       pml::TimeFrequencyParameter< SampleType >::numberOfBinsRealToComplex(
           dftSize ) )
 , cFramesPerPeriod( period() / hopSize )
 , cHopSize( hopSize )
 , mAccumulationBuffer( cNumberOfChannels, dftSize - cHopSize, cAlignment )
 , mFftWrapper( rbbl::FftWrapperFactory< SampleType >::create(
       fftImplementation, dftSize, cAlignment ) )
 , mCalcBuffer( dftSize, cAlignment )
 , cScalingFactor{ calculateScalingFactor(
       normalisation, mFftWrapper->inverseScalingFactor(), dftSize ) }
 , mInput( "in",
           *this,
           pml::TimeFrequencyParameterConfig(
               cNumberOfDftBins, numberOfChannels, cFramesPerPeriod ) )
 , mOutput( "out", *this, numberOfChannels )
{
  if( period() % hopSize != 0 )
  {
    throw std::invalid_argument(
        "TimeFrequencyInverseTransform: Invalid hop size (no integer number of "
        "hops per audio processing period)." );
  }
}

TimeFrequencyInverseTransform::~TimeFrequencyInverseTransform() = default;

void TimeFrequencyInverseTransform::process()
{
  pml::TimeFrequencyParameter< SampleType > const & inMtx = mInput.data();
  const std::size_t accuElementsToCopy = cDftSize - cHopSize;
  // operating channel by channel might save copying to and fro the accumulation
  // buffer in case of multiple hops per period.
  for( std::size_t channelIdx( 0 ); channelIdx < cNumberOfChannels;
       ++channelIdx )
  {
    efl::ErrorCode res;
    for( std::size_t frameIdx( 0 ); frameIdx < cFramesPerPeriod; ++frameIdx )
    {
      std::complex< SampleType > const * dftPtr =
          inMtx.channelSlice( frameIdx, channelIdx );
      res = mFftWrapper->inverseTransform( dftPtr, mCalcBuffer.data() );
      if( res != efl::noError )
      {
        throw std::runtime_error(
            "TimeFrequencyInverseTransform: Error during FFT operation." );
      }
      res = efl::vectorAddInplace( mAccumulationBuffer.row( channelIdx ),
                                   mCalcBuffer.data(), accuElementsToCopy );
      if( res != efl::noError )
      {
        throw std::runtime_error(
            "TimeFrequencyInverseTransform: Updating of output accumulator "
            "failed." );
      }
      // Copy the output buffer back into storage (skip first block to implement
      // the shift)
      if( ( res = efl::vectorCopy( mCalcBuffer.data() + cHopSize,
                                   mAccumulationBuffer.row( channelIdx ),
                                   accuElementsToCopy ) ) != efl::noError )
      {
        throw std::runtime_error(
            "TimeFrequencyInverseTransform: Storing partial results failed." );
      }
    }
    // Copy first portion of accumulated result to output port.
    // Also applies the DFT scaling.
    // if( ( res = efl::vectorCopy( mCalcBuffer.data(), mOutput[ channelIdx ],
    //                              period(), cAlignment ) ) != efl::noError )
    if( ( res = efl::vectorMultiplyConstant( cScalingFactor,
        mCalcBuffer.data(), mOutput[ channelIdx ],
        period(), cAlignment ) ) != efl::noError )
    {
      throw std::runtime_error(
          "TimeFrequencyInverseTransform: Error while copying output data." );
    }
  }
}

} // namespace rcl
} // namespace visr
