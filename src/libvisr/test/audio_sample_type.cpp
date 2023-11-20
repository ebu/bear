/* Copyright Institue of Sound and Vibration Research - All rights reserved. */


#include <libvisr/audio_sample_type.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <cstdint>

namespace visr
{
namespace test
{

BOOST_AUTO_TEST_CASE( CheckTypeToIdTranslation )
{
  constexpr AudioSampleType::Id idOrig = AudioSampleType::Id::doubleId;
  constexpr AudioSampleType::Id idResolved = AudioSampleType::TypeToId<double>::id;

  // We could check that at compile time as well:
  static_assert(idOrig == idResolved, "Extraction of type id failed" );
  BOOST_CHECK( idOrig == idResolved );

  using SampleType = AudioSampleType::IdToType<AudioSampleType::uint16Id>::Type;
  BOOST_CHECK( typeid(SampleType) == typeid(uint16_t) );

  std::cout << "Type info name : " << typeid(SampleType).name() << ", "
            << "hash: " << typeid(SampleType).hash_code() << std::endl;

}

} // namespace test
} // namespace visr
