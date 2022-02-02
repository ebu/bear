/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "crossfading_fir_filter_matrix.hpp"

#include <librbbl/crossfading_convolver_uniform.hpp>

#include <ciso646>
#include <type_traits>

namespace visr
{
namespace rcl
{
CrossfadingFirFilterMatrix::ControlPortConfig operator&(
    CrossfadingFirFilterMatrix::ControlPortConfig lhs,
    CrossfadingFirFilterMatrix::ControlPortConfig rhs )
{
  using T = std::underlying_type<
      CrossfadingFirFilterMatrix::ControlPortConfig >::type;
  return static_cast< CrossfadingFirFilterMatrix::ControlPortConfig >(
      static_cast< T >( lhs ) & static_cast< T >( rhs ) );
}

CrossfadingFirFilterMatrix::ControlPortConfig operator|(
    CrossfadingFirFilterMatrix::ControlPortConfig lhs,
    CrossfadingFirFilterMatrix::ControlPortConfig rhs )
{
  using T = std::underlying_type<
      CrossfadingFirFilterMatrix::ControlPortConfig >::type;
  return static_cast< CrossfadingFirFilterMatrix::ControlPortConfig >(
      static_cast< T >( lhs ) | static_cast< T >( rhs ) );
}

CrossfadingFirFilterMatrix::CrossfadingFirFilterMatrix(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent /*= nullptr*/,
    std::size_t numberOfInputs,
    std::size_t numberOfOutputs,
    std::size_t filterLength,
    std::size_t maxFilters,
    std::size_t maxRoutings,
    std::size_t transitionSamples,
    efl::BasicMatrix< SampleType > const &
        filters /*= efl::BasicMatrix<SampleType>()*/,
    rbbl::FilterRoutingList const & routings /*= rbbl::FilterRoutingList()*/,
    ControlPortConfig controlInputs /*= ControlPortConfig::None*/,
    char const * fftImplementation /*= "default" */ )
 : AtomicComponent( context, name, parent )
 , mInput( "in", *this, numberOfInputs )
 , mOutput( "out", *this, numberOfOutputs )
 , mSetFilterInput(
       ( ( controlInputs & ControlPortConfig::Filters ) !=
         ControlPortConfig::None )
           ? new ParameterInput<
                 pml::MessageQueueProtocol,
                 pml::IndexedValueParameter< std::size_t,
                                             std::vector< SampleType > > >(
                 "filterInput", *this, pml::EmptyParameterConfig() )
           : nullptr )
 , mConvolver( new rbbl::CrossfadingConvolverUniform< SampleType >(
       numberOfInputs,
       numberOfOutputs,
       period(),
       filterLength,
       maxRoutings,
       maxFilters,
       transitionSamples,
       routings,
       filters,
       cVectorAlignmentSamples,
       fftImplementation ) )
{
}

CrossfadingFirFilterMatrix::~CrossfadingFirFilterMatrix() = default;

void CrossfadingFirFilterMatrix::process()
{
  if( mSetFilterInput )
  {
    while( not mSetFilterInput->empty() )
    {
      pml::IndexedValueParameter< std::size_t, std::vector< SampleType > > const
          newFilter = mSetFilterInput->front();

      try
      {
        // Argument checking is done inside the method.
        setFilter( newFilter.index(), &newFilter.value()[ 0 ],
                   newFilter.value().size() );
        mSetFilterInput->pop();
      }
      catch( std::exception const & ex )
      {
        status( StatusMessage::Error,
                "CrossfadingFirFilterMatrix: Error while setting new filter: ",
                ex.what() );
      }
    }
  }

  mConvolver->process( mInput.data(), mInput.channelStrideSamples(),
                       mOutput.data(), mOutput.channelStrideSamples(),
                       cVectorAlignmentSamples );
}

void CrossfadingFirFilterMatrix::clearRoutings()
{
  mConvolver->clearRoutingTable();
}

void CrossfadingFirFilterMatrix::addRouting( std::size_t inputIdx,
                                             std::size_t outputIdx,
                                             std::size_t filterIdx,
                                             SampleType const gain )
{
  mConvolver->setRoutingEntry( inputIdx, outputIdx, filterIdx, gain );
}

void CrossfadingFirFilterMatrix::addRouting(
    rbbl::FilterRouting const & routing )
{
  mConvolver->setRoutingEntry(
      routing.inputIndex, routing.outputIndex, routing.filterIndex,
      static_cast< SampleType >( routing.gainLinear ) );
}

void CrossfadingFirFilterMatrix::addRoutings(
    rbbl::FilterRoutingList const & routings )
{
  for( rbbl::FilterRouting const & entry: routings )
  {
    addRouting( entry );
  }
}

void CrossfadingFirFilterMatrix::setRoutings(
    rbbl::FilterRoutingList const & routings )
{
  clearRoutings();
  addRoutings( routings );
}

bool CrossfadingFirFilterMatrix::removeRouting( std::size_t inputIdx,
                                                std::size_t outputIdx )
{
  return mConvolver->removeRoutingEntry( inputIdx, outputIdx );
}

void CrossfadingFirFilterMatrix::clearFilters() { mConvolver->clearFilters(); }

void CrossfadingFirFilterMatrix::setFilter(
    std::size_t filterIdx,
    SampleType const * const impulseResponse,
    std::size_t filterLength,
    std::size_t alignment /*=0*/ )
{
  mConvolver->setImpulseResponse( impulseResponse, filterLength, filterIdx,
                                  true /*startTransition*/, alignment );
}

void CrossfadingFirFilterMatrix::setFilters(
    efl::BasicMatrix< SampleType > const & filterSet )
{
  mConvolver->initFilters( filterSet );
}

} // namespace rcl
} // namespace visr
