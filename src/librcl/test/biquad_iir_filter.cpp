/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <librcl/biquad_iir_filter.hpp>

#include <libvisr/signal_flow_context.hpp>

#include <libpml/biquad_parameter.hpp>

#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

namespace visr
{
namespace rcl
{
namespace test
{

BOOST_AUTO_TEST_CASE( InstantiateBiquad )
{
  std::size_t const blockSize{64};
  SamplingFrequencyType const fs{48000};
  SignalFlowContext const ctxt( blockSize, fs );

  std::size_t const numberOfChannels{ 3 };
  std::size_t const numberOfSections{ 5 };
  bool const controlInput{ true };

  rcl::BiquadIirFilter biquad(ctxt, "biquad", nullptr,
    numberOfChannels, numberOfSections, controlInput );
}

} // namespace test
} // namespace rcl
} // namespce visr
