/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "biquad_iir_filter.hpp"

#include <libefl/error_codes.hpp>
#include <libefl/filter_functions.hpp>

#include <libpml/matrix_parameter_config.hpp>

#include <librbbl/biquad_coefficient.hpp>

#include <libvisr/constants.hpp>

#include <ciso646>
#include <stdexcept>

namespace visr
{
namespace rcl
{
BiquadIirFilter::BiquadIirFilter( SignalFlowContext const & context,
                                  char const * name,
                                  CompositeComponent * parent,
                                  std::size_t numberOfChannels,
                                  std::size_t numberOfBiquads,
                                  bool controlInput /*= false*/ )
 : AtomicComponent( context, name, parent )
 , mInput( "in", *this, numberOfChannels )
 , mOutput( "out", *this, numberOfChannels )
 , mEqInput(
       controlInput
           ? new ParameterInput< pml::DoubleBufferingProtocol,
                                 pml::BiquadParameterMatrix< SampleType > >(
                 "eqInput",
                 *this,
                 pml::MatrixParameterConfig( numberOfChannels,
                                             numberOfBiquads ) )
           : nullptr )
 , cNumberOfChannels( numberOfChannels )
 , cNumberOfBiquadSections( numberOfBiquads )
 , mCoefficients( numberOfChannels,
                  numberOfBiquads * cBiquadCoefficientsStride,
                  cVectorAlignmentSamples )
 , mState( numberOfChannels,
           numberOfBiquads * cBiquadStateStride,
           cVectorAlignmentSamples )
{
  mState.zeroFill();
}

BiquadIirFilter::BiquadIirFilter(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    std::size_t numberOfChannels,
    std::size_t numberOfBiquads,
    rbbl::BiquadCoefficient< SampleType > const & initialBiquad,
    bool controlInput /*= false*/ )
 : BiquadIirFilter(
       context, name, parent, numberOfChannels, numberOfBiquads, controlInput )
{
  for( std::size_t channelIdx( 0 ); channelIdx < cNumberOfChannels;
       ++channelIdx )
  {
    for( std::size_t biquadIdx( 0 ); biquadIdx < cNumberOfBiquadSections;
         ++biquadIdx )
    {
      setCoefficientsInternal( channelIdx, biquadIdx, initialBiquad );
    }
  }
}

BiquadIirFilter::BiquadIirFilter(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    std::size_t numberOfChannels,
    std::size_t numberOfBiquads,
    rbbl::BiquadCoefficientList< SampleType > const & coeffs,
    bool controlInput /*= false*/ )
 : BiquadIirFilter(
       context, name, parent, numberOfChannels, numberOfBiquads, controlInput )
{
  if( coeffs.size() != numberOfBiquads )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The length of the coefficient initialiser list does "
        "not match the number of biquads per channel." );
  }
  for( std::size_t channelIdx( 0 ); channelIdx < cNumberOfChannels;
       ++channelIdx )
  {
    setChannelCoefficientsInternal( channelIdx, coeffs );
  }
}

BiquadIirFilter::BiquadIirFilter(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent,
    std::size_t numberOfChannels,
    std::size_t numberOfBiquads,
    rbbl::BiquadCoefficientMatrix< SampleType > const & coeffs,
    bool controlInput /*= false*/ )
 : BiquadIirFilter(
       context, name, parent, numberOfChannels, numberOfBiquads, controlInput )
{
  if( ( coeffs.numberOfFilters() != numberOfChannels ) or
      ( coeffs.numberOfSections() != numberOfBiquads ) )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The size of the coefficient matrix does not match "
        "the dimension numberOfChannels x numberOfBiquads." );
  }
  setCoefficientMatrixInternal( coeffs );
}

void BiquadIirFilter::setCoefficients(
    std::size_t channelIndex,
    std::size_t biquadIndex,
    rbbl::BiquadCoefficient< SampleType > const & coeffs )
{
  if( channelIndex >= cNumberOfChannels )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The channel index exceeds the number of channels." );
  }
  if( biquadIndex >= cNumberOfBiquadSections )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The biquad index exceeds the number of biquad "
        "sections per channel." );
  }
  setCoefficientsInternal( channelIndex, biquadIndex, coeffs );
}

void BiquadIirFilter::setCoefficientsInternal(
    std::size_t channelIndex,
    std::size_t biquadIndex,
    rbbl::BiquadCoefficient< SampleType > const & coeffs )
{
  SampleType * startVal = mCoefficients.row( channelIndex ) +
                          biquadIndex * cBiquadCoefficientsStride;
  std::copy( coeffs.data(),
             coeffs.data() +
                 rbbl::BiquadCoefficient< SampleType >::cNumberOfCoeffs,
             startVal );
}

void BiquadIirFilter::setChannelCoefficients(
    std::size_t channelIndex,
    rbbl::BiquadCoefficientList< SampleType > const & coeffs )
{
  if( channelIndex >= cNumberOfChannels )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The channel index exceeds the number of channels." );
  }
  if( coeffs.size() != cNumberOfBiquadSections )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The filter coefficient list does not match the "
        "number of biquad sections per channel." );
  }
  setChannelCoefficientsInternal( channelIndex, coeffs );
}

void BiquadIirFilter::setChannelCoefficientsInternal(
    std::size_t channelIndex,
    rbbl::BiquadCoefficientList< SampleType > const & coeffs )
{
  for( std::size_t biquadIdx( 0 ); biquadIdx < cNumberOfBiquadSections;
       ++biquadIdx )
  {
    setCoefficientsInternal( channelIndex, biquadIdx, coeffs[ biquadIdx ] );
  }
}

void BiquadIirFilter::setCoefficientMatrix(
    rbbl::BiquadCoefficientMatrix< SampleType > const & coeffs )
{
  if( coeffs.numberOfFilters() != cNumberOfChannels )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The number of filters in the coefficient matrix does "
        "not match the number of channels of this component." );
  }
  // Special case if the parameter has zero channels (i.e., inactive)
  if( ( cNumberOfChannels > 0 ) and
      coeffs.numberOfSections() != cNumberOfBiquadSections )
  {
    throw std::invalid_argument(
        "BiquadIirFilter: The number of biquad section in the coefficient "
        "matrix does not match the number of sections of this component." );
  }
  setCoefficientMatrixInternal( coeffs );
}

void BiquadIirFilter::setCoefficientMatrixInternal(
    rbbl::BiquadCoefficientMatrix< SampleType > const & coeffs )
{
  for( std::size_t channelIdx( 0 ); channelIdx < cNumberOfChannels;
       ++channelIdx )
  {
    setChannelCoefficientsInternal( channelIdx, coeffs[ channelIdx ] );
  }
}

void BiquadIirFilter::process()
{
  if( mEqInput and mEqInput->changed() )
  {
    setCoefficientMatrix( mEqInput->data() );
    mEqInput->resetChanged();
  }
  std::size_t const numSamples{ period() };
  std::size_t const alignment{ mInput.alignmentSamples() };
  for( std::size_t chIdx{ 0 }; chIdx < cNumberOfChannels; ++chIdx )
  {
    efl::ErrorCode const res = efl::iirFilterBiquadsSingleChannel(
        mInput[ chIdx ], mOutput[ chIdx ], mState.row( chIdx ),
        mCoefficients.row( chIdx ), numSamples, cNumberOfBiquadSections,
        cBiquadStateStride, cBiquadCoefficientsStride, alignment );
    if( res != efl::noError )
    {
      status( StatusMessage::Error,
              "Error during IIR filtering: ", efl::errorMessage( res ) );
    }
  }
}

} // namespace rcl
} // namespace visr
