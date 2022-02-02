/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "time_frequency_parameter.hpp"

#include <libvisr/constants.hpp>
#include <libvisr/parameter_factory.hpp>

#include <ciso646>
#include <complex>
#include <fstream>
#include <limits>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <vector>

namespace visr
{
namespace pml
{

template< typename ElementType >
TimeFrequencyParameter<ElementType>::TimeFrequencyParameter( std::size_t alignment /*= 0*/ )
  : mData( alignment )
  , mNumberOfChannels( 0 )
{
  mData.zeroFill();
}

template< typename ElementType >
TimeFrequencyParameter<ElementType>::TimeFrequencyParameter( ParameterConfigBase const & config )
  : TimeFrequencyParameter<ElementType>( dynamic_cast<TimeFrequencyParameterConfig const &>(config))
{

}

template< typename ElementType >
TimeFrequencyParameter<ElementType>::TimeFrequencyParameter( TimeFrequencyParameterConfig const & config )
  : mData( config.numberOfFrames() * config.numberOfChannels(), config.numberOfDftBins(), cVectorAlignmentSamples )
  , mNumberOfChannels( config.numberOfChannels() )
{

}

template< typename ElementType >
TimeFrequencyParameter<ElementType>::TimeFrequencyParameter( std::size_t numberOfDftBins,
                                                             std::size_t numberOfChannels,
                                                             std::size_t numberOfFrames,
                                                             size_t alignment /*= 0*/ )
 : mData( numberOfFrames * numberOfChannels, numberOfDftBins, alignment )
 , mNumberOfChannels( numberOfChannels )
{
  // Zero-initialise
  mData.zeroFill();
}

template< typename ElementType >
TimeFrequencyParameter<ElementType>::TimeFrequencyParameter( TimeFrequencyParameter<ElementType> const & rhs )
  : mData( rhs.mData.numberOfRows(), rhs.mData.numberOfColumns(), rhs.alignment() )
  , mNumberOfChannels( rhs.numberOfChannels() )
{
  mData.copy( rhs.mData );
}

template< typename ElementType >
TimeFrequencyParameter<ElementType>::~TimeFrequencyParameter() = default;

template< typename ElementType >
TimeFrequencyParameter<ElementType>&
TimeFrequencyParameter<ElementType>::operator=( TimeFrequencyParameter<ElementType> const & rhs )
{
  if( &rhs == this )
  {
    return *this;
  }
  mData.resize( rhs.mData.numberOfRows(), rhs.mData.numberOfColumns() );
  mData.copy( rhs.mData );
  mNumberOfChannels = rhs.numberOfChannels();
  return *this;
}

template< typename ElementType >
void TimeFrequencyParameter<ElementType>::
resize( std::size_t numberOfDftBins, std::size_t numberOfChannels, std::size_t numberOfFrames )
{
  mData.resize( numberOfFrames * numberOfChannels, numberOfDftBins );
  mData.zeroFill();
  mNumberOfChannels = numberOfChannels;
}

// Explicit instantiations for sample types float and double
template class TimeFrequencyParameter<float>;
template class TimeFrequencyParameter<double>;

} // namespace pml
} // namespace visr
