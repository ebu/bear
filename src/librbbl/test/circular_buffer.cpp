/* Copyright Institue of Sound and Vibration Research - All rights reserved. */

#include <librbbl/circular_buffer.hpp>

#include <libefl/basic_matrix.hpp>
#include <libvisr/constants.hpp>

#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_CASE( CircularBufferAllocation )
{
  std::size_t const bufferSize = 253; // deliberately odd
  std::size_t const numChannels = 2;

  CircularBuffer<float> buffer( numChannels, bufferSize, 16 );

  BOOST_CHECK( buffer.length() == bufferSize );
  BOOST_CHECK( buffer.numberOfChannels() == numChannels );

  efl::BasicMatrix<float> fillBuffer( 2, 5, { { 0, 1, 2, 3, 4 }, { 5, 6, 7, 8, 9 } } );

  BOOST_CHECK_NO_THROW( buffer.write( fillBuffer ) );

  std::vector<float const *> fillBufferPtr( numChannels );
  for( std::size_t chIdx( 0 ); chIdx < numChannels; ++chIdx )
  {
    fillBufferPtr[chIdx] = fillBuffer.row( chIdx );
  }
  BOOST_CHECK_NO_THROW( buffer.write( &fillBufferPtr[0], numChannels, fillBuffer.numberOfColumns() ) );

  std::vector<float const *> readBufferPtr( numChannels );

  std::size_t const singleDelayVal = 8;

  BOOST_CHECK_NO_THROW( buffer.getReadPointers( singleDelayVal, &readBufferPtr[0]) );

  std::copy( readBufferPtr[0], readBufferPtr[0] + 9, std::ostream_iterator<float>( std::cout, ", " ) );
  std::cout << std::endl;

  float currentVal;
  BOOST_CHECK_NO_THROW( currentVal = *(buffer.getReadPointer( 1, 0 )) );
  std::cout << " delay = 0, value: " << currentVal << std::endl;

  // check for delay exceeding the maximum admissible delay.
  BOOST_CHECK_THROW( buffer.getReadPointers( bufferSize, &readBufferPtr[0] ), std::invalid_argument );
}

} // namespace test
} // namespace rbbl
} // namespace visr
