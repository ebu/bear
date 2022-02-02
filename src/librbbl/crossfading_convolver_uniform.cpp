/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "crossfading_convolver_uniform.hpp"

#include <libefl/vector_functions.hpp>

#include <complex>
#include <stdexcept>

namespace visr
{
namespace rbbl
{
template< typename SampleType >
CrossfadingConvolverUniform< SampleType >::CrossfadingConvolverUniform(
    std::size_t numberOfInputs,
    std::size_t numberOfOutputs,
    std::size_t blockLength,
    std::size_t maxFilterLength,
    std::size_t maxRoutingPoints,
    std::size_t maxFilterEntries,
    std::size_t transitionSamples,
    FilterRoutingList const & initialRoutings,
    efl::BasicMatrix< SampleType > const & initialFilters,
    std::size_t alignment /*= 0*/,
    char const * fftImplementation /*= "default"*/ )
 : mCoreConvolver( numberOfInputs,
                   numberOfOutputs,
                   blockLength,
                   maxFilterLength,
                   2 * maxFilterEntries,
                   initialFilters,
                   alignment,
                   fftImplementation )
 , mMaxNumberOfRoutingPoints( maxRoutingPoints )
 , mFrequencyDomainOutput( 2,
                           mCoreConvolver.dftBlockRepresentationSize(),
                           mCoreConvolver.complexAlignment() )
 , mTimeDomainTempOutput( 2, blockLength, alignment )
 , mMaxNumFilters( maxFilterEntries )
 , mNumRampBlocks( ( transitionSamples + blockLength - 1 ) / blockLength +
                   1 ) // integer version of ceil(transition/blockLength)
 , mCurrentFilterOutput( maxFilterEntries, 0 )
 , mCurrentRampBlock( maxFilterEntries, mNumRampBlocks - 1 )
 , mCrossoverRamps( 2, blockLength * mNumRampBlocks, alignment )
{
  initRoutingTable( initialRoutings );
  efl::ErrorCode ret;
  if( ( ret = efl::vectorRamp( mCrossoverRamps.row( 0 ), transitionSamples,
                               static_cast< SampleType >( 0.0f ),
                               static_cast< SampleType >( 1.0f ), false, true,
                               alignment ) ) != efl::noError )
  {
    throw std::runtime_error(
        "CrossfadingConvolverUniform: Error initialising crossfading ramps." );
  }
  if( ( ret = efl::vectorFill( static_cast< SampleType >( 1.0f ),
                               mCrossoverRamps.row( 0 ) + transitionSamples,
                               blockLength * mNumRampBlocks - transitionSamples,
                               0 /* cannot assume any alignment here*/ ) ) !=
      efl::noError )
  {
    throw std::runtime_error(
        "CrossfadingConvolverUniform: Error initialising crossfading ramps." );
  }
  if( ( ret = efl::vectorRamp( mCrossoverRamps.row( 1 ), transitionSamples,
                               static_cast< SampleType >( 1.0f ),
                               static_cast< SampleType >( 0.0f ), false, true,
                               alignment ) ) != efl::noError )
  {
    throw std::runtime_error(
        "CrossfadingConvolverUniform: Error initialising crossfading ramps." );
  }
  if( ( ret = efl::vectorFill( static_cast< SampleType >( 0.0f ),
                               mCrossoverRamps.row( 1 ) + transitionSamples,
                               blockLength * mNumRampBlocks - transitionSamples,
                               0 /* cannot assume any alignment here*/ ) ) !=
      efl::noError )
  {
    throw std::runtime_error(
        "CrossfadingConvolverUniform: Error initialising crossfading ramps." );
  }
}

template< typename SampleType >
CrossfadingConvolverUniform< SampleType >::~CrossfadingConvolverUniform() =
    default;

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::process(
    SampleType const * const input,
    std::size_t inputChannelStride,
    SampleType * const output,
    std::size_t outputChannelStride,
    std::size_t alignment /*= 0*/ )
{
  mCoreConvolver.processInputs( input, inputChannelStride, alignment );
  processOutputs( output, outputChannelStride, alignment );
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::processOutputs(
    SampleType * const output,
    std::size_t outputChannelStride,
    std::size_t alignment /*= 0*/ )
{
  std::size_t const blockSize{ blockLength() };
  // Zero all time-domain outputs beforehand.
  // Another solution would be to keep track results written to the outputs, and
  // to zero, copy or add the buffers as appropriate.
  for( std::size_t outputIdx( 0 ); outputIdx < numberOfOutputs(); ++outputIdx )
  {
    efl::vectorZero( output + outputIdx * outputChannelStride, blockSize,
                     alignment );
  }

  for( RoutingEntry const & routing: mRoutingTable )
  {
    // Find the current fade-in and fade-out filters.
    std::size_t const fadeInFilterIdx =
        ( mCurrentFilterOutput[ routing.filterIdx ] ) == 0
            ? routing.filterIdx
            : routing.filterIdx + mMaxNumFilters;
    std::size_t const fadeOutFilterIdx =
        ( mCurrentFilterOutput[ routing.filterIdx ] ) == 0
            ? routing.filterIdx + mMaxNumFilters
            : routing.filterIdx;

    mCoreConvolver.processFilter(
        routing.inputIdx, fadeInFilterIdx, routing.gainLinear,
        mFrequencyDomainOutput.row( 0 ), false /* add flag */ );
    mCoreConvolver.processFilter(
        routing.inputIdx, fadeOutFilterIdx, routing.gainLinear,
        mFrequencyDomainOutput.row( 1 ), false /* add flag */ );
    mCoreConvolver.transformOutput( mFrequencyDomainOutput.row( 0 ),
                                    mTimeDomainTempOutput.row( 0 ) );
    mCoreConvolver.transformOutput( mFrequencyDomainOutput.row( 1 ),
                                    mTimeDomainTempOutput.row( 1 ) );

    std::size_t const outputAlignment =
        std::min( alignment, mCoreConvolver.alignment() );
    std::size_t const coreAlignment = mCoreConvolver.alignment();

    efl::ErrorCode res;
    std::size_t const rampBlock = mCurrentRampBlock[ routing.filterIdx ];
    if( ( res = efl::vectorMultiplyInplace(
              mCrossoverRamps.row( 0 ) + rampBlock * blockSize,
              mTimeDomainTempOutput.row( 0 ), blockSize, coreAlignment ) ) !=
        efl::noError )
    {
      throw std::runtime_error(
          "CrossfadingConvolver: Multiplication with fade-in ramp failed." );
    }
    if( ( res = efl::vectorMultiplyAddInplace(
              mCrossoverRamps.row( 1 ) + rampBlock * blockSize,
              mTimeDomainTempOutput.row( 1 ), mTimeDomainTempOutput.row( 0 ),
              blockSize, coreAlignment ) ) != efl::noError )
    {
      throw std::runtime_error(
          "CrossfadingConvolver: Multiplication with fade-out ramp failed." );
    }
    if( ( res = efl::vectorAddInplace(
              mTimeDomainTempOutput.row( 0 ),
              output + routing.outputIdx * outputChannelStride, blockSize,
              outputAlignment ) ) != efl::noError )
    {
      throw std::runtime_error(
          "CrossfadingConvolver: Adding to output signal failed.." );
    }
  }

  // Advance the ramp counters
  for( auto & v: mCurrentRampBlock )
  {
    v = std::min( v + 1, mNumRampBlocks - 1 );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Manipulation of the routing table

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::clearRoutingTable()
{
  mRoutingTable.clear();
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::initRoutingTable(
    FilterRoutingList const & routings )
{
  clearRoutingTable();
  if( routings.size() > maxNumberOfRoutingPoints() )
  {
    throw std::invalid_argument(
        "CrossfadingConvolverUniform:initRoutingTable() exceeds the maximum "
        "admissible number of elements " );
  }
  for( FilterRouting const & v: routings )
  {
    setRoutingEntry( v );
  }
  assert( mRoutingTable.size() <= maxNumberOfRoutingPoints() );
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::setRoutingEntry(
    FilterRouting const & routing )
{
  setRoutingEntry( routing.inputIndex, routing.outputIndex, routing.filterIndex,
                   static_cast< SampleType >( routing.gainLinear ) );
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::setRoutingEntry(
    std::size_t inputIdx,
    std::size_t outputIdx,
    std::size_t filterIdx,
    SampleType gain )
{
  assert( mRoutingTable.size() <= maxNumberOfRoutingPoints() );
  if( inputIdx >= numberOfInputs() )
  {
    throw std::invalid_argument(
        "CoreConvolverUniform::setRoutingEntry(): Input index exceeds the "
        "admissible range." );
  }
  if( outputIdx >= numberOfOutputs() )
  {
    throw std::invalid_argument(
        "CoreConvolverUniform::setRoutingEntry(): Output index exceeds the "
        "admissible range." );
  }
  if( filterIdx >= maxNumberOfFilterEntries() )
  {
    throw std::invalid_argument(
        "CoreConvolverUniform::setRoutingEntry(): Filter index exceeds the "
        "admissible range." );
  }
  RoutingEntry newEntry( inputIdx, outputIdx, filterIdx, gain );
  typename RoutingTable::iterator findIt = mRoutingTable.find( newEntry );
  if( findIt != mRoutingTable.end() )
  {
    mRoutingTable.erase( findIt );
  }
  if( mRoutingTable.size() >= maxNumberOfRoutingPoints() )
  {
    throw std::invalid_argument(
        "CoreConvolverUniform::setRoutingEntry(): Maximum number of routing "
        "points already reached." );
  }
  mRoutingTable.insert( std::move( newEntry ) );
  assert( mRoutingTable.size() <= maxNumberOfRoutingPoints() );
}

template< typename SampleType >
bool CrossfadingConvolverUniform< SampleType >::removeRoutingEntry(
    std::size_t inputIdx, std::size_t outputIdx )
{
  RoutingEntry const testEntry( inputIdx, outputIdx, 0, 0.0f );
  auto const eraseRange = mRoutingTable.equal_range( testEntry );
  mRoutingTable.erase( eraseRange.first, eraseRange.second );
  return eraseRange.first != mRoutingTable.end();
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::clearFilters()
{
  mCoreConvolver.clearFilters();
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::initFilters(
    efl::BasicMatrix< SampleType > const & newFilters )
{
  mCoreConvolver.initFilters( newFilters );
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::transformImpulseResponse(
    SampleType const * ir,
    std::size_t irLength,
    FrequencyDomainType * result,
    std::size_t alignment /*= 0*/ ) const
{
  mCoreConvolver.transformImpulseResponse( ir, irLength, result, alignment );
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::setImpulseResponse(
    SampleType const * ir,
    std::size_t filterLength,
    std::size_t filterIdx,
    bool startTransition,
    std::size_t alignment )
{
  // TODO: Avoid code duplication in setTransformedFilter().

  std::size_t const oldFilterSet = mCurrentFilterOutput[ filterIdx ];
  std::size_t const newFilterSet = ( oldFilterSet == 0 ) ? 1 : 0;
  std::size_t const newFilterIndex =
      ( newFilterSet == 0 ) ? filterIdx : filterIdx + mMaxNumFilters;

  // Note: If the previous transition is not finished by now, the filter
  // switches immediately to the old target filter to use it as the starting
  // point of the new transition. Having smooth transition in this case would
  // require an interpolation of the FIR coefficients, and there is no space to
  // hold additional filtersets in the current implementation.

  // Transform the new impulse response into the alternate filter set.
  mCoreConvolver.setImpulseResponse( ir, filterLength, newFilterIndex,
                                     alignment );
  mCurrentFilterOutput[ filterIdx ] = newFilterSet; // switch the filter set
  if( startTransition )
  {
    mCurrentRampBlock[ filterIdx ] =
        0; // Set the interpolation sequence to the start position.
  }
}

template< typename SampleType >
void CrossfadingConvolverUniform< SampleType >::setTransformedFilter(
    FrequencyDomainType const * fdFilter,
    std::size_t filterIdx,
    bool startTransition,
    std::size_t alignment )
{
  std::size_t const oldFilterSet = mCurrentFilterOutput[ filterIdx ];
  std::size_t const newFilterSet = ( oldFilterSet == 0 ) ? 1 : 0;
  std::size_t const newFilterIndex =
      ( newFilterSet == 0 ) ? filterIdx : filterIdx + mMaxNumFilters;

  // Note: If the previous transition is not finished by now, the filter
  // switches immediately to the old target filter to use it as the starting
  // point of the new transition. Having smooth transition in this case would
  // require an interpolation of the FIR coefficients, and there is no space to
  // hold additional filtersets in the current implementation.

  mCoreConvolver.setFilter( fdFilter, newFilterIndex, alignment );
  mCurrentFilterOutput[ filterIdx ] = newFilterSet; // switch the filter set
  if( startTransition )
  {
    mCurrentRampBlock[ filterIdx ] = 0; 
  }
}

// explicit instantiations
template class CrossfadingConvolverUniform< float >;
template class CrossfadingConvolverUniform< double >;

} // namespace rbbl
} // namespace visr
