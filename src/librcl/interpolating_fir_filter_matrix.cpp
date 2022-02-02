/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "interpolating_fir_filter_matrix.hpp"

#include <librbbl/interpolating_convolver_uniform.hpp>

#include <ciso646>
#include <type_traits>

namespace visr
{
namespace rcl
{
InterpolatingFirFilterMatrix::ControlPortConfig operator&(
    InterpolatingFirFilterMatrix::ControlPortConfig lhs,
    InterpolatingFirFilterMatrix::ControlPortConfig rhs )
{
  using T = std::underlying_type<
      InterpolatingFirFilterMatrix::ControlPortConfig >::type;
  return static_cast< InterpolatingFirFilterMatrix::ControlPortConfig >(
      static_cast< T >( lhs ) & static_cast< T >( rhs ) );
}

InterpolatingFirFilterMatrix::ControlPortConfig operator|(
    InterpolatingFirFilterMatrix::ControlPortConfig lhs,
    InterpolatingFirFilterMatrix::ControlPortConfig rhs )
{
  using T = std::underlying_type<
      InterpolatingFirFilterMatrix::ControlPortConfig >::type;
  return static_cast< InterpolatingFirFilterMatrix::ControlPortConfig >(
      static_cast< T >( lhs ) | static_cast< T >( rhs ) );
}

InterpolatingFirFilterMatrix::InterpolatingFirFilterMatrix(
    SignalFlowContext const & context,
    char const * name,
    CompositeComponent * parent /*= nullptr*/,
    std::size_t numberOfInputs,
    std::size_t numberOfOutputs,
    std::size_t filterLength,
    std::size_t maxFilters,
    std::size_t maxRoutings,
    std::size_t numberOfInterpolants,
    std::size_t transitionSamples,
    efl::BasicMatrix< SampleType > const &
        filters /*= efl::BasicMatrix<SampleType>()*/,
    rbbl::InterpolationParameterSet const &
        initialInterpolants /*= rbbl::InterpolationParameterSet()*/,
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
 , mInterpolantInput(
       ( ( controlInputs & ControlPortConfig::Interpolants ) !=
         ControlPortConfig::None )
           ? new ParameterInput< pml::MessageQueueProtocol,
                                 pml::InterpolationParameter >(
                 "interpolantInput",
                 *this,
                 pml::InterpolationParameterConfig( numberOfInterpolants ) )
           : nullptr )
 , mConvolver( new rbbl::InterpolatingConvolverUniform< SampleType >(
       numberOfInputs,
       numberOfOutputs,
       period(),
       filterLength,
       maxRoutings,
       maxFilters,
       numberOfInterpolants,
       transitionSamples,
       routings,
       initialInterpolants,
       filters,
       cVectorAlignmentSamples,
       fftImplementation ) )
{
}

InterpolatingFirFilterMatrix::~InterpolatingFirFilterMatrix() = default;

void InterpolatingFirFilterMatrix::process()
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
        status(
            StatusMessage::Error,
            "InterpolatingFirFilterMatrix: Error while setting new filter: ",
            ex.what() );
      }
    }
  }

  // TODO: The routing control input is neither instantiated nor evaluated at
  // the moment.

  if( mInterpolantInput )
  {
    while( not mInterpolantInput->empty() )
    {
      try
      {
        rbbl::InterpolationParameter const & param = mInterpolantInput->front();

        setInterpolant( param, true/*startTransition*/ );
        // Argument checking is done inside the method.
        mInterpolantInput->pop();
      }
      catch( std::exception const & ex )
      {
        status( StatusMessage::Error,
                "InterpolatingFirFilterMatrix: Error while setting new "
                "interpolant: ",
                ex.what() );
      }
    }
  }

  mConvolver->process( mInput.data(), mInput.channelStrideSamples(),
                       mOutput.data(), mOutput.channelStrideSamples(),
                       cVectorAlignmentSamples );
}

void InterpolatingFirFilterMatrix::clearRoutings()
{
  mConvolver->clearRoutingTable();
}

void InterpolatingFirFilterMatrix::addRouting( std::size_t inputIdx,
                                               std::size_t outputIdx,
                                               std::size_t filterIdx,
                                               SampleType const gain )
{
  mConvolver->setRoutingEntry( inputIdx, outputIdx, filterIdx, gain );
}

void InterpolatingFirFilterMatrix::addRouting(
    rbbl::FilterRouting const & routing )
{
  mConvolver->setRoutingEntry(
      routing.inputIndex, routing.outputIndex, routing.filterIndex,
      static_cast< SampleType >( routing.gainLinear ) );
}

void InterpolatingFirFilterMatrix::addRoutings(
    rbbl::FilterRoutingList const & routings )
{
  for( rbbl::FilterRouting const & entry: routings )
  {
    addRouting( entry );
  }
}

void InterpolatingFirFilterMatrix::setRoutings(
    rbbl::FilterRoutingList const & routings )
{
  clearRoutings();
  addRoutings( routings );
}

bool InterpolatingFirFilterMatrix::removeRouting( std::size_t inputIdx,
                                                  std::size_t outputIdx )
{
  return mConvolver->removeRoutingEntry( inputIdx, outputIdx );
}

void InterpolatingFirFilterMatrix::clearFilters()
{
  mConvolver->clearFilters();
}

void InterpolatingFirFilterMatrix::setFilter(
    std::size_t filterIdx,
    SampleType const * const impulseResponse,
    std::size_t filterLength,
    std::size_t alignment /*=0*/ )
{
  mConvolver->setImpulseResponse( impulseResponse, filterLength, filterIdx,
                                  alignment );
}

void InterpolatingFirFilterMatrix::setFilters(
    efl::BasicMatrix< SampleType > const & filterSet )
{
  mConvolver->initFilters( filterSet );
}

void InterpolatingFirFilterMatrix::setInterpolant(
    rbbl::InterpolationParameter const & interpolant,
    bool startTransition )
{
  mConvolver->setInterpolant( interpolant, startTransition );
}

void InterpolatingFirFilterMatrix::setInterpolant(
    std::size_t id,
    std::vector< std::size_t > const & indices,
    std::vector< float > const & weights,
    bool startTransition )
{
  mConvolver->setInterpolant(
      rbbl::InterpolationParameter( id, indices, weights ),
      startTransition );
}

void InterpolatingFirFilterMatrix::setInterpolant(
    std::size_t id,
    std::initializer_list< std::size_t > const & indices,
    std::initializer_list< float > const & weights,
    bool startTransition )
{
  mConvolver->setInterpolant(
      rbbl::InterpolationParameter( id, indices, weights ),
      startTransition );
}

} // namespace rcl
} // namespace visr
