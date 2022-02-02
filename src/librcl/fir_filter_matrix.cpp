/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "fir_filter_matrix.hpp"

#include <librbbl/multichannel_convolver_uniform.hpp>

#include <ciso646>
#include <type_traits>


namespace visr
{
namespace rcl
{

FirFilterMatrix::ControlPortConfig operator&(FirFilterMatrix::ControlPortConfig lhs,
                                        FirFilterMatrix::ControlPortConfig rhs )
{
  using T = std::underlying_type<FirFilterMatrix::ControlPortConfig>::type;
  return static_cast<FirFilterMatrix::ControlPortConfig>( static_cast<T>(lhs) & static_cast<T>(rhs) );
}

FirFilterMatrix::ControlPortConfig operator|(FirFilterMatrix::ControlPortConfig lhs,
                                        FirFilterMatrix::ControlPortConfig rhs )
{
  using T = std::underlying_type<FirFilterMatrix::ControlPortConfig>::type;
  return static_cast<FirFilterMatrix::ControlPortConfig>( static_cast<T>(lhs) | static_cast<T>(rhs) );
}

FirFilterMatrix::FirFilterMatrix( SignalFlowContext const & context,
                                  char const * name,
                                  CompositeComponent * parent /*= nullptr*/ )
 : AtomicComponent( context, name, parent )
 , mInput( "in", *this )
 , mOutput( "out", *this )
 , mConvolver()
{
}

FirFilterMatrix::FirFilterMatrix( SignalFlowContext const & context,
  char const * name,
  CompositeComponent * parent,
  std::size_t numberOfInputs,
  std::size_t numberOfOutputs,
  std::size_t filterLength,
  std::size_t maxFilters,
  std::size_t maxRoutings,
  efl::BasicMatrix<SampleType> const & filters /*= efl::BasicMatrix<SampleType>()*/,
  rbbl::FilterRoutingList const & routings /*= rbbl::FilterRoutingList()*/,
  ControlPortConfig controlInputs /*= ControlPortConfig::None*/,
  char const * fftImplementation /*= "default"*/ )
  : AtomicComponent( context, name, parent )
  , mInput( "in", *this, numberOfInputs )
  , mOutput( "out", *this, numberOfOutputs )
  , mConvolver(new rbbl::MultichannelConvolverUniform<SampleType>(
    numberOfInputs, numberOfOutputs, period(),
    filterLength, maxRoutings, maxFilters,
    routings, filters, cVectorAlignmentSamples, fftImplementation ) )
{
  if( (controlInputs & ControlPortConfig::Filters) != ControlPortConfig::None )
  {
    mSetFilterInput.reset( new FilterInput("filterInput", *this, pml::EmptyParameterConfig()) );
  }
  if( (controlInputs & ControlPortConfig::Routings) != ControlPortConfig::None )
  {
    mSingleRoutingInput.reset( new SingleRoutingInput( "singleRouting", *this, pml::EmptyParameterConfig() ) );
  }
  if( (controlInputs & ControlPortConfig::AllRoutings) != ControlPortConfig::None )
  {
    mAllRoutingsInput.reset( new AllRoutingsInput( "allRoutings", *this, pml::EmptyParameterConfig() ) );
  }
}

FirFilterMatrix::~FirFilterMatrix() = default;

void FirFilterMatrix::process()
{
  if( mSetFilterInput )
  {
    while( not mSetFilterInput->empty() )
    {
      pml::IndexedValueParameter<std::size_t, std::vector<SampleType> > const newFilter = mSetFilterInput->front();

      try
      {
        // Argument checking is done inside the method.
        setFilter( newFilter.index(), &newFilter.value()[0], newFilter.value().size() );
        mSetFilterInput->pop();
      }
      catch( std::exception const & ex )
      {
        status( StatusMessage::Error, "FirFilterMatrix: Error while setting new filter: ", ex.what() );
      }
    }
  }
  // If mSingleRoutingInput and mAllRoutingsInput are both defined, the 'all routings' message is handled first.
  // That means that single routings messages are applied on top of the new complete routing, i.e., they are not lost.
  if( mAllRoutingsInput and mAllRoutingsInput->changed() )
  {
    rbbl::FilterRoutingList const & routings{ mAllRoutingsInput->data() };
    try
    {
      setRoutings( routings );
    }
    catch( std::exception const & ex )
    {
      status( StatusMessage::Error, "FirFilterMatrix: Error while resetting filter routings: ", ex.what() );
    }
    mAllRoutingsInput->resetChanged();
  }
  if( mSingleRoutingInput )
  {
    while( not mSingleRoutingInput->empty() )
    {
      try
      {
        rbbl::FilterRouting const & entry{ mSingleRoutingInput->front() };
        addRouting( entry );
      }
      catch( std::exception const & ex )
      {
        status( StatusMessage::Error, "FirFilterMatrix: Error while setting filter routing: ", ex.what() );      
      }      
      mSingleRoutingInput->pop();
    }
  }
  mConvolver->process( mInput.data(), mInput.channelStrideSamples(), 
                       mOutput.data(), mOutput.channelStrideSamples(),
                       cVectorAlignmentSamples );
}

void FirFilterMatrix::clearRoutings()
{
  mConvolver->clearRoutingTable();
}

void FirFilterMatrix::addRouting( std::size_t inputIdx, std::size_t outputIdx, std::size_t filterIdx, SampleType const gain )
{
  mConvolver->setRoutingEntry( inputIdx, outputIdx, filterIdx, gain );
}

void FirFilterMatrix::addRouting( rbbl::FilterRouting const & routing )
{
  mConvolver->setRoutingEntry( routing.inputIndex, routing.outputIndex, routing.filterIndex, static_cast<SampleType>(routing.gainLinear) );
}

void FirFilterMatrix::addRoutings( rbbl::FilterRoutingList const & routings )
{
  for( rbbl::FilterRouting const & entry : routings )
  {
    addRouting( entry );
  }
}

void FirFilterMatrix::setRoutings( rbbl::FilterRoutingList const & routings )
{
  clearRoutings();
  addRoutings( routings );
}

bool FirFilterMatrix::removeRouting( std::size_t inputIdx, std::size_t outputIdx )
{
  return mConvolver->removeRoutingEntry( inputIdx, outputIdx );
}

void FirFilterMatrix::clearFilters()
{
  mConvolver->clearFilters();
}

void FirFilterMatrix::setFilter( std::size_t filterIdx, SampleType const * const impulseResponse, std::size_t filterLength, std::size_t alignment /*=0*/ )
{
  mConvolver->setImpulseResponse( impulseResponse, filterLength, filterIdx, alignment );
}

void FirFilterMatrix::setFilters( efl::BasicMatrix<SampleType> const & filterSet )
{
  mConvolver->initFilters( filterSet );
}

} // namespace rcl
} // namespace visr
