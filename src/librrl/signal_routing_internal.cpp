/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "signal_routing_internal.hpp"

#include <libefl/vector_functions.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace visr
{
namespace rrl
{

template< typename SampleType >
SignalRoutingInternal<SampleType>::SignalRoutingInternal( SignalFlowContext const & context,
                                  char const * name,
                                  CompositeComponent * parent,
                                  std::size_t inputWidth,
                                  std::vector<std::size_t> signalIndices )
 : AtomicComponent( context, name, parent )
 , mInputIndices( signalIndices )
 , mInput( "in", *this, inputWidth )
 , mOutput( "out", *this, mInputIndices.size() )
{
  if( mInputIndices.empty() )
  {
    return; // Must omit the following check, because it would dereference the value past the empty array.
  }
  if( *std::max_element( mInputIndices.cbegin(), mInputIndices.cend()) >= inputWidth )
  {
    throw std::invalid_argument( "rrl::SignalRoutingInternal: An input channel index exceeds the width of the input port." );
  }
}

template< typename SampleType >
SignalRoutingInternal<SampleType>::~SignalRoutingInternal() = default;

template< typename SampleType >
AudioPortBase * SignalRoutingInternal<SampleType>::input()
{
  return &mInput;
}

template< typename SampleType >
AudioPortBase * SignalRoutingInternal<SampleType>::output()
{
  return &mOutput;
}

template< typename SampleType >
void SignalRoutingInternal<SampleType>::process()
{
  std::size_t const outputWidth = mInputIndices.size();
  std::size_t const blockSize = period();
  for( std::size_t chIdx(0); chIdx < outputWidth; ++chIdx )
  {
    std::size_t const inIdx = mInputIndices[chIdx];
    efl::ErrorCode const res = efl::vectorCopy( mInput[inIdx], mOutput[chIdx], blockSize, cVectorAlignmentSamples );
    if( res != efl::noError )
    {
      throw std::runtime_error( "rrl::SignalRoutingInternal: Copying of sample vector failed." );
    }
  }
}

template class SignalRoutingInternal<float>;
template class SignalRoutingInternal<double>;


std::unique_ptr<AtomicComponent> createSignalRoutingComponent( AudioSampleType::Id sampleType,
                                                               SignalFlowContext const & context,
                                                               char const * name,
                                                               CompositeComponent * parent,
                                                               std::size_t inputWidth,
                                                               std::vector<std::size_t> signalIndices )
{
  switch( sampleType )
  {
  case AudioSampleType::floatId: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<float>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::doubleId: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<double>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::longDoubleId: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<long double>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::uint8Id: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<uint8_t>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::int8Id: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<int8_t>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::uint16Id: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<uint16_t>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::int16Id: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<int16_t>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::uint32Id: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<uint32_t>( context, name, parent, inputWidth, signalIndices ) );
  case AudioSampleType::int32Id: return std::unique_ptr<AtomicComponent>( new SignalRoutingInternal<int32_t>( context, name, parent, inputWidth, signalIndices ) );
  default: throw std::invalid_argument( "createSignalRoutingComponent(): Sample type not supported.");
  }
}

} // namespace rrl
} // namespace visr
