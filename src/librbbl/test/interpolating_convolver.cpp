/* Copyright Institue of Sound and Vibration Research - All rights reserved. */

#include <librbbl/interpolating_convolver_uniform.hpp>
#include <librbbl/interpolation_parameter.hpp>

#include <libefl/basic_matrix.hpp>
#include <libvisr/constants.hpp>
#include <libpml/matrix_parameter.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ciso646>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace visr
{
namespace rbbl
{
namespace test
{

#ifdef VISR_PML_USE_SNDFILE_LIBRARY
BOOST_AUTO_TEST_CASE( InterpolatingConvolverStatic )
{
  static const std::size_t alignment = 8; // element
  using SampleType = float;
  using Conv = InterpolatingConvolverUniform<SampleType>;
  boost::filesystem::path const baseDir = CMAKE_CURRENT_SOURCE_DIR;
  pml::MatrixParameter<SampleType> const filters = pml::MatrixParameter<SampleType>::fromAudioFile( (baseDir/boost::filesystem::path("signals/filters_4ch.wav")).string(), alignment );
  pml::MatrixParameter<SampleType> const inputSignal = pml::MatrixParameter<SampleType>::fromAudioFile( (baseDir / boost::filesystem::path( "signals/input_4ch.wav" )).string( ), alignment );
  pml::MatrixParameter<SampleType> const outputSignalReference = pml::MatrixParameter<SampleType>::fromAudioFile( (baseDir / boost::filesystem::path( "signals/output_4ch.wav" )).string( ), alignment );

  std::size_t const cNumberOfInputs = inputSignal.numberOfRows();
  std::size_t const cNumberOfOutputs = outputSignalReference.numberOfRows( );
  std::size_t const cNumFilters = filters.numberOfRows();
  std::size_t const cSignalLength = inputSignal.numberOfColumns();

  std::size_t const cFilterLength = filters.numberOfColumns();

  std::size_t const cNumberOfInterpolants = 2;

  InterpolationParameter const ip0{ 0, {0,1,}, {1.0f, 0.0f} };
  InterpolationParameterSet const ips{ ip0 };
  
  std::size_t const cBlockLength = 32;
  BOOST_CHECK( cBlockLength % alignment == 0 );

  std::size_t const cMaxRoutings = 12;
  rbbl::FilterRoutingList routings = {{ 0, 0, 0, 1.0f }, { 1, 1, 1, 1.0f } };

  efl::BasicMatrix<SampleType> outputSignal( cNumberOfOutputs, cSignalLength, alignment );

  
  Conv convolver( cNumberOfInputs,
		  cNumberOfOutputs,
		  cBlockLength,
		  cFilterLength,
		  cMaxRoutings,
		  cNumFilters,
		  cNumberOfInterpolants,
		  0, // transition samples
		  routings,
		  ips,
		  filters,
		  alignment,
		  "ffts" );
#if 0
  BOOST_CHECK_MESSAGE( cSignalLength % cBlockLength == 0 , "The signal length must be an integral multiple of the block size." );
  std::size_t const cNumBlocks = cSignalLength / cBlockLength;
  for( std::size_t blockIdx( 0 ); blockIdx < cNumBlocks; ++blockIdx )
  {
    const std::size_t signalIdx = blockIdx * cBlockLength;
    convolver.process( inputSignal.data() + signalIdx, inputSignal.stride(),
                       outputSignal.data() + signalIdx, outputSignal.stride() );
  }

  for( std::size_t chIdx( 0 ); chIdx < cNumberOfOutputs; ++chIdx )
  {
    SampleType const * outSig = outputSignal.row( chIdx );
    SampleType const * outSigRef = outputSignalReference.row( chIdx );

    efl::BasicVector<SampleType> diff( cSignalLength, alignment );
    std::transform( outSig, outSig + cSignalLength, outSigRef, diff.data(), []( SampleType lhs, SampleType rhs ) { return std::abs( lhs - rhs ); } );

    SampleType * res = diff.data();

    SampleType const * maxErr = std::max_element( res, res + diff.size() );
    std::cout << "Max. error for channel " << chIdx << ": " << *res << std::endl;
    BOOST_CHECK_MESSAGE( *maxErr <= static_cast<SampleType>(4.0) * std::numeric_limits<SampleType>::epsilon(), "Convolution error exceeds 4*epsilon() for this datatype." );
  }
#endif
}
#endif


} // namespace test
} // namespace rbbl
} // namespace visr
