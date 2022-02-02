/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "panning_matrix_parameter.hpp"

#include <libvisr/constants.hpp>
#include <libvisr/parameter_factory.hpp>
#include <libvisr/parameter_type.hpp>

#include <libpml/matrix_parameter_config.hpp>

#include <algorithm>
#include <cassert>
#include <ciso646>
#include <limits>
#include <stdexcept>

namespace visr
{
namespace panningdsp
{

namespace // unnamed
{
/**
 * Utility functions to work around the limited interface of the low-level
 * class efl::AlignedArray.
 */
//@{

template< typename T >
void copy(visr::efl::AlignedArray< T> const & src,
  visr::efl::AlignedArray< T> & dest)
{
  assert( src.size() == dest.size() );
  std::copy( src.data(), src.data() + src.size(),
    dest.data());
}

/**
 * Create a copy that can be used in a move constructor.
 * This is to work around the deliberate omission of a copy constructor
 * in efl::AlignedArray.
 */
template< typename T >
efl::AlignedArray< T > createCopy( efl::AlignedArray< T > const & src, std::size_t alignment )
{
  efl::AlignedArray< T > res( src.size(), alignment );
  copy(src, res);
  return res;
}

/**
 * Fill all elements of \p dest with the constant value \p val.
 */
template< typename T >
void fill( efl::AlignedArray< T > & dest, T val)
{
  std::fill(dest.data(), dest.data() + dest.size(), val);
}

/**
 * Create a AlignedArray filled with a constant value.
 * Use, for example, to pass into move constructors.
 */
template< typename T >
efl::AlignedArray< T > createConstantArray( T val, std::size_t size, std::size_t alignment )
{
  efl::AlignedArray< T > res( size, alignment );
  fill( res, val );
  return res;
}

template< typename T >
efl::AlignedArray< T > arrayFromInitializerList(std::initializer_list< T > list, std::size_t size, std::size_t alignment)
{
  if (list.size() != size)
  {
    throw std::invalid_argument( "Initializer list size does not match expected size." );
  }
  efl::AlignedArray< T > res( size , alignment);
  std::copy(list.begin(), list.end(), res.data());
  return res;
}


//@}
} // unnamed namespace

PanningMatrixParameter::PanningMatrixParameter( std::size_t numberOfObjects,
  std::size_t numberOfLoudspeakers, std::size_t alignment /*= 0*/ )
 : mGains( numberOfObjects, numberOfLoudspeakers, alignment )
 , mTimeStamps( createConstantArray( cTimeStampInfinity, numberOfObjects, alignment ) )
 , mTransitionTimes( createConstantArray( cTimeStampInfinity, numberOfObjects, alignment ) )
{
  mGains.zeroFill();
}

PanningMatrixParameter::PanningMatrixParameter(visr::efl::BasicMatrix<SampleType> const & gains,
  visr::efl::AlignedArray< TimeType > const & timeStamps,
  visr::efl::AlignedArray< TimeType > const & transitionTimes)
 : mGains(gains.numberOfRows(), gains.numberOfColumns(), gains.alignmentElements())
 , mTimeStamps( createCopy( timeStamps, gains.alignmentElements() ) )
 , mTransitionTimes( createCopy( transitionTimes, gains.alignmentElements() ) )
{
  mGains.copy( gains );
}

PanningMatrixParameter::PanningMatrixParameter(visr::efl::BasicMatrix<SampleType> const & gains,
  std::initializer_list< TimeType > const & timeStamps,
  std::initializer_list< TimeType > const & transitionTimes)
 : PanningMatrixParameter( gains,
     arrayFromInitializerList( timeStamps, gains.numberOfRows(), gains.alignmentElements() ),
     arrayFromInitializerList( transitionTimes,
       gains.numberOfRows(), gains.alignmentElements()) )
{
}

PanningMatrixParameter::PanningMatrixParameter(
  std::initializer_list< std::initializer_list< SampleType > > const & gains,
  std::initializer_list< TimeType > const & timeStamps,
  std::initializer_list< TimeType > const & transitionTimes,
  std::size_t alignment /*= 0*/ )
 : mGains( gains.size(),
     gains.begin() == gains.end() ? 0 : gains.begin()->size(),
     gains, alignment )
 , mTimeStamps(arrayFromInitializerList(timeStamps,
     numberOfObjects(), alignment ) )
 , mTransitionTimes(arrayFromInitializerList( transitionTimes,
     numberOfObjects(), alignment ))
{
}

PanningMatrixParameter::PanningMatrixParameter( pml::MatrixParameterConfig const & config )
: PanningMatrixParameter( config.numberOfRows( ), config.numberOfColumns( ), cVectorAlignmentSamples )
{
}

PanningMatrixParameter::PanningMatrixParameter( ParameterConfigBase const & config )
: PanningMatrixParameter( dynamic_cast<pml::MatrixParameterConfig const &>(config) )
{
  // Todo: handle exceptions
}

PanningMatrixParameter::PanningMatrixParameter( PanningMatrixParameter const & rhs )
 : mGains(rhs.gains().numberOfRows(), rhs.gains().numberOfColumns(), rhs.alignmentElements())
 , mTimeStamps( createCopy( rhs.timeStamps(),  rhs.alignmentElements() ) )
 , mTransitionTimes( createCopy( rhs.transitionTimes(), rhs.alignmentElements() ) )
{
  mGains.copy( rhs.gains() );
}

PanningMatrixParameter::~PanningMatrixParameter() = default;

PanningMatrixParameter& PanningMatrixParameter::operator=(PanningMatrixParameter const & rhs)
{
  if (&rhs != this)
  {
    mTimeStamps.resize(rhs.numberOfObjects());
    mTransitionTimes.resize(rhs.numberOfObjects());
    mGains.resize(rhs.numberOfLoudspeakers(),
      rhs.numberOfObjects());
    copy(rhs.timeStamps(), timeStamps());
    copy(rhs.transitionTimes(), transitionTimes());
    mGains.copy(rhs.gains());
  }
  return *this;
}

std::size_t PanningMatrixParameter::numberOfObjects() const
{
  return mGains.numberOfRows();
}

std::size_t PanningMatrixParameter::numberOfLoudspeakers() const
{
  return mGains.numberOfColumns();
}

std::size_t PanningMatrixParameter::alignmentElements() const
{
  return mGains.alignmentElements();
}

GainMatrixType const &
PanningMatrixParameter::gains() const
{
  return mGains;
}

GainMatrixType &
PanningMatrixParameter::gains()
{
  return mGains;
}


TimeVector const &
PanningMatrixParameter::timeStamps() const
{
  return mTimeStamps;
}

TimeVector & 
PanningMatrixParameter::timeStamps()
{
  return mTimeStamps;
}

TimeVector const & 
PanningMatrixParameter::transitionTimes() const
{
  return mTransitionTimes;
}

TimeVector &
PanningMatrixParameter::transitionTimes()
{
  return mTransitionTimes;
}

} // namespace panningdsp
} // namespace visr
