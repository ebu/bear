/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "interpolating_convolver_uniform.hpp"

#include <libvisr/detail/compose_message_string.hpp>

#include <libefl/vector_functions.hpp>

#include <complex>

namespace visr
{
namespace rbbl
{
template< typename SampleType >
InterpolatingConvolverUniform< SampleType >::InterpolatingConvolverUniform(
    std::size_t numberOfInputs,
    std::size_t numberOfOutputs,
    std::size_t blockLength,
    std::size_t maxFilterLength,
    std::size_t maxRoutingPoints,
    std::size_t maxFilterEntries,
    std::size_t numberOfInterpolants,
    std::size_t transitionSamples,
    FilterRoutingList const & initialRoutings /*= FilterRoutingList()*/,
    InterpolationParameterSet const &
        initialInterpolants /*= InterpolationParameterSet()*/,
    efl::BasicMatrix< SampleType > const & initialFilters,
    std::size_t alignment /*= 0*/,
    char const * fftImplementation /*= "default"*/ )
 : mConvolver(
       numberOfInputs,
       numberOfOutputs,
       blockLength,
       maxFilterLength,
       maxFilterEntries,
       maxRoutingPoints,
       transitionSamples,
       initialRoutings,
       efl::BasicMatrix< SampleType >(
           alignment ), // No initial filters (they are set by the interpolants)
       alignment,
       fftImplementation )
 , mFilters( maxFilterEntries,
             mConvolver.dftFilterRepresentationSize(),
             mConvolver.complexAlignment() )
 , mTempFilter( mConvolver.dftFilterRepresentationSize(),
                mConvolver.complexAlignment() )
 , mNumberOfInterpolants( numberOfInterpolants )
{
  initFilters( initialFilters );

  setInterpolants( initialInterpolants, false /*startTransition*/ );
}

template< typename SampleType >
InterpolatingConvolverUniform< SampleType >::~InterpolatingConvolverUniform() =
    default;

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::process(
    SampleType const * const input,
    std::size_t inputChannelStride,
    SampleType * const output,
    std::size_t outputChannelStride,
    std::size_t alignment /*= 0*/ )
{
  mConvolver.process( input, inputChannelStride, output, outputChannelStride,
                      alignment );
}

#if 0
template< typename SampleType >
void InterpolatingConvolverUniform<SampleType>::
processOutputs( SampleType * const output, std::size_t outputChannelStride,
                std::size_t alignment /*= 0*/ )
{
  std::size_t const blockSize{ blockLength() };
  // Zero all time-domain outputs beforehand.
  // Another solution would be to keep track results written to the outputs, and to zero,
  // copy or add the buffers as appropriate.
  for( std::size_t outputIdx(0); outputIdx < numberOfOutputs(); ++outputIdx )
  {
    efl::vectorZero( output + outputIdx * outputChannelStride, blockSize, alignment );
  }

  for( RoutingEntry const & routing : mRoutingTable )
  {
    // Find the current fade-in and fade-out filters.
    std::size_t const fadeInFilterIdx = (mCurrentFilterOutput[routing.filterIdx]) == 0
        ? routing.filterIdx : routing.filterIdx + mMaxNumFilters;
    std::size_t const fadeOutFilterIdx = (mCurrentFilterOutput[routing.filterIdx]) == 0
        ? routing.filterIdx + mMaxNumFilters : routing.filterIdx;


    mCoreConvolver.processFilter( routing.inputIdx, fadeInFilterIdx, routing.gainLinear,
                                  mFrequencyDomainOutput.row(0), false /* add flag */ );
    mCoreConvolver.processFilter( routing.inputIdx, fadeOutFilterIdx, routing.gainLinear,
                                  mFrequencyDomainOutput.row(1), false /* add flag */ );
    mCoreConvolver.transformOutput( mFrequencyDomainOutput.row(0), mTimeDomainTempOutput.row(0) );
    mCoreConvolver.transformOutput( mFrequencyDomainOutput.row(1), mTimeDomainTempOutput.row(1) );

    std::size_t const outputAlignment = std::min( alignment, mCoreConvolver.alignment() );
    std::size_t const coreAlignment = mCoreConvolver.alignment();

    efl::ErrorCode res;
    std::size_t const rampBlock = mCurrentRampBlock[ routing.filterIdx ];
    if( (res = efl::vectorMultiplyInplace( mCrossoverRamps.row(0)+rampBlock*blockSize,
                                           mTimeDomainTempOutput.row(0), blockSize, coreAlignment)) != efl::noError )
    {
      throw std::runtime_error( "InterpolatingConvolver: Multiplication with fade-in ramp failed.");
    }
    if( (res = efl::vectorMultiplyAddInplace( mCrossoverRamps.row(1)+rampBlock*blockSize,
                                              mTimeDomainTempOutput.row(1), mTimeDomainTempOutput.row(0),
                                              blockSize, coreAlignment)) != efl::noError )
    {
      throw std::runtime_error( "InterpolatingConvolver: Multiplication with fade-out ramp failed.");
    }
    if( (res = efl::vectorAddInplace( mTimeDomainTempOutput.row(0), output+routing.outputIdx*outputChannelStride, blockSize, outputAlignment)) != efl::noError )
    {
      throw std::runtime_error( "InterpolatingConvolver: Adding to output signal failed..");
    }
  }

  // Advance the ramp counters
  for( auto & v: mCurrentRampBlock )
  {
    v = std::min( v+1, mNumRampBlocks-1 );
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Manipulation of the routing table

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::clearRoutingTable()
{
  mConvolver.clearRoutingTable();
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::initRoutingTable(
    FilterRoutingList const & routings )
{
  mConvolver.initRoutingTable( routings );
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::setRoutingEntry(
    FilterRouting const & routing )
{
  mConvolver.setRoutingEntry( routing );
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::setRoutingEntry(
    std::size_t inputIdx,
    std::size_t outputIdx,
    std::size_t filterIdx,
    SampleType gain )
{
  mConvolver.setRoutingEntry( inputIdx, outputIdx, filterIdx, gain );
}

template< typename SampleType >
bool InterpolatingConvolverUniform< SampleType >::removeRoutingEntry(
    std::size_t inputIdx, std::size_t outputIdx )
{
  return mConvolver.removeRoutingEntry( inputIdx, outputIdx );
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::clearFilters()
{
  mFilters.zeroFill();
  // Decision: also clear the filters of the running convolution process.
  mConvolver.clearFilters();
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::initFilters(
    efl::BasicMatrix< SampleType > const & newImpulseResponses )
{
  std::size_t const numFilters{ newImpulseResponses.numberOfRows() };
  std::size_t const irLength{ newImpulseResponses.numberOfColumns() };
  if( numFilters > maxNumberOfFilterEntries() )
  {
    throw std::invalid_argument(
        "InterpolatingConvolverUniform::initFilters(): Size of new filter "
        "matrix exceeds number of filter slots." );
  }
  if( irLength > maxFilterLength() )
  {
    throw std::invalid_argument(
        "InterpolatingConvolverUniform::initFilters(): Size of new filter "
        "matrix exceeds maximum filter length." );
  }
  // Compute the minimum alignment: *2 because mFilters is complex.
  // TODO: revise semantics of transformImpulseResponse
  std::size_t const align( std::min( newImpulseResponses.alignmentElements(),
                                     mFilters.alignmentElements() * 2 ) );
  for( std::size_t filterIdx( 0 ); filterIdx < numFilters; ++filterIdx )
  {
    mConvolver.transformImpulseResponse( newImpulseResponses.row( filterIdx ),
                                         irLength, mFilters.row( filterIdx ),
                                         align );
  }
  // Zero the remain filter entries.
  for( std::size_t filterIdx( numFilters );
       filterIdx < maxNumberOfFilterEntries(); ++filterIdx )
  {
    efl::vectorZero( mFilters.row( filterIdx ), mFilters.numberOfColumns(),
                     mFilters.alignmentElements() );
  }
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::setImpulseResponse(
    SampleType const * ir,
    std::size_t filterLength,
    std::size_t filterIdx,
    std::size_t alignment /*= 0*/ )
{
  if( filterIdx >= maxNumberOfFilterEntries() )
  {
    throw std::invalid_argument(
        "InterpolatingConvolverUniform::setImpulseResponse(): Filter index "
        "exceeds number of filter slots." );
  }
  if( filterLength > maxFilterLength() )
  {
    throw std::invalid_argument(
        "InterpolatingConvolverUniform::setImpulseResponse(): Length of new "
        "impulse response exceeds maximum filter length." );
  }
  // Compute the minimum alignment: *2 because mFilters is complex.
  // TODO: revise semantics of transformImpulseResponse
  std::size_t const align(
      std::min( alignment, mFilters.alignmentElements() * 2 ) );
  mConvolver.transformImpulseResponse( ir, filterLength,
                                       mFilters.row( filterIdx ), align );
}

template< typename SampleType >
std::size_t InterpolatingConvolverUniform< SampleType >::numberOfInterpolants()
    const
{
  return mNumberOfInterpolants;
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::setInterpolant(
    rbbl::InterpolationParameter const & param, bool startTransition )
{
  rbbl::InterpolationParameter::IdType const routingId = param.id();
  if( routingId >= maxNumberOfRoutingPoints() )
  {
    throw std::out_of_range(
        "InterpolatingConvolverUniform::setInterpolant(): interpolant id "
        "exceeds number of filter routings. " );
  }
  assert( mNumberOfInterpolants > 0 ); // Normally checked in the constructor
  assert( param.indices().size() == param.numberOfInterpolants() );
  // Typically this would be prohibited externally
  if( param.numberOfInterpolants() != mNumberOfInterpolants )
  {
    throw std::invalid_argument(
        "InterpolatingConvolverUniform::setInterpolant(): Number of "
        "interpolants in parameter does not match the value set in the "
        "convolver." );
  }
  InterpolationParameter::IndexType const maxIndex =
      *( std::max_element( param.indices().begin(), param.indices().end() ) );
  if( maxIndex >= mFilters.numberOfRows() )
  {
    throw std::runtime_error( detail::composeMessageString(
        "InterpolatingConvolverUniform::setInterpolant(): At least one filter "
        "index \'",
        maxIndex, "\' exceeds maximum admissible index (",
        mFilters.numberOfRows() - 1, ")." ) );
  }

  // We use real-valued arithmetics to scale the complex-valued FD
  // representations
  std::size_t const numRealElements =
      2 * mConvolver.dftFilterRepresentationSize();

  std::size_t align{ mConvolver.alignment() };

  // TODO: Consider moving that code into a separate function.
  efl::ErrorCode res = efl::vectorMultiplyConstant< SampleType >(
      param.weight( 0 ),
      reinterpret_cast< SampleType const * const >(
          mFilters.row( param.index( 0 ) ) ),
      reinterpret_cast< SampleType * const >( mTempFilter.data() ),
      numRealElements, align );
  if( res != efl::noError )
  {
    throw std::runtime_error( detail::composeMessageString(
        "InterpolatingConvolverUniform::setInterpolant(): filter interpolation "
        "failed: ",
        efl::errorMessage( res ) ) );
  }
  for( std::size_t interpolantIdx( 1 ); interpolantIdx < mNumberOfInterpolants;
       ++interpolantIdx )
  {
    efl::ErrorCode res = efl::vectorMultiplyConstantAddInplace< SampleType >(
        param.weight( interpolantIdx ),
        reinterpret_cast< SampleType const * const >(
            mFilters.row( param.index( interpolantIdx ) ) ),
        reinterpret_cast< SampleType * const >( mTempFilter.data() ),
        numRealElements, align );
    if( res != efl::noError )
    {
      throw std::runtime_error( detail::composeMessageString(
          "InterpolatingConvolverUniform::setInterpolant(): filter "
          "interpolation failed: ",
          efl::errorMessage( res ) ) );
    }
  }
  mConvolver.setTransformedFilter( mTempFilter.data(), routingId,
                                   startTransition,
                                   mTempFilter.alignmentElements() );
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::setInterpolants(
    InterpolationParameterSet const & params, bool startTransition )
{
  for( rbbl::InterpolationParameter const & v: params )
  {
    setInterpolant( v, startTransition );
  }
}

template< typename SampleType >
void InterpolatingConvolverUniform< SampleType >::clearInterpolants()
{
  mConvolver.clearFilters();
}

// explicit instantiations
template class InterpolatingConvolverUniform< float >;
template class InterpolatingConvolverUniform< double >;

} // namespace rbbl
} // namespace visr
