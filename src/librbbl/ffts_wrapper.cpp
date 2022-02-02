/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "ffts_wrapper.hpp"

#include <libefl/aligned_array.hpp>

#include <ffts.h>

#include <ciso646>
#include <stdexcept>

namespace visr
{
namespace rbbl
{

template<>
class FftsWrapper<float>::Impl
{
public:
  Impl( std::size_t fftSize, std::size_t alignComplexElements )
  {
#if defined(VISR_SYSTEM_PROCESSOR_x86_64) || defined(VISR_SYSTEM_PROCESSOR_armv7l)
    // Internal, non-documented requirement in FFTS due to the use of aligned SSE or NEON loads.
    if( alignComplexElements < 128 / (8 * sizeof(std::complex< float >)) )
    {
      throw std::invalid_argument( "FftsWrapper requires a minimum alignment of 2 complex elements" );
    }
#endif
    mFwdPlan = ffts_init_1d_real( fftSize, -1 );
    if( not mFwdPlan )
    {
      throw std::invalid_argument( "FfftWrapper: Initialisation of forward FFT plan failed." );
    }
    mInvPlan = ffts_init_1d_real( fftSize, 1 );
    if( not mInvPlan )
    {
      throw std::invalid_argument( "FfftWrapper: Initialisation of inverse FFT plan failed." );
    }
  }

  ~Impl()
  {
    ffts_free( mFwdPlan );
    ffts_free( mInvPlan );
  }


  ffts_plan_t* mFwdPlan;
  ffts_plan_t* mInvPlan;
};

template<typename DataType>
FftsWrapper<DataType>::FftsWrapper( std::size_t fftSize, std::size_t alignmentElements )
 : mImpl( new FftsWrapper<DataType>::Impl( fftSize, alignmentElements ) )
{
}
template FftsWrapper<float>::FftsWrapper( std::size_t, std::size_t );

template<typename DataType>
FftsWrapper<DataType>::~FftsWrapper()
{
}
template FftsWrapper<float>::~FftsWrapper( );

template<>
efl::ErrorCode FftsWrapper<float>::forwardTransform( float const * const in, std::complex<float> * out ) const
{
  ffts_execute( mImpl->mFwdPlan, in, out );
  return efl::noError; // apparently, there is no error reporting
}

template<>
efl::ErrorCode FftsWrapper<float>::inverseTransform( std::complex<float> const * const in, float * out ) const
{
  ffts_execute( mImpl->mInvPlan, in, out );
  return efl::noError; // apparently, there is no error reporting apart from a SIGSEGV in case of misalignment
}

} // namespace rbbl
} // namespace visr
