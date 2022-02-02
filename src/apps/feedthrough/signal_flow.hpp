/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_APPS_FEEDTHROUGH_SIGNAL_FLOW_HPP_INCLUDED
#define VISR_APPS_FEEDTHROUGH_SIGNAL_FLOW_HPP_INCLUDED

#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/atomic_component.hpp>
#include <libvisr/composite_component.hpp>

#include <memory>

namespace visr
{
namespace apps
{
namespace feedthrough
{

namespace detail
{

/**
 * Component to consume unused audio inputs.
 * @todo Consider moving to rcl library (make it more flexible, e.g.,
 * by accepting selectable sample formats).
 */
class AudioTerminator: public AtomicComponent
{
public:
  AudioTerminator( SignalFlowContext & context, const char* name,
		   CompositeComponent * parent,
		   std::size_t numberOfChannels )
    : AtomicComponent( context, name, parent )
    , mInput( "input", *this, numberOfChannels )
  {}

  virtual ~AudioTerminator() override = default;

  void process() override
  {
    // Do nothing
  }
private:
  AudioInput mInput;
};

} // unnamed namespace

class Feedthrough: public CompositeComponent
{
public:
  explicit Feedthrough( SignalFlowContext & context, const char* name,
			CompositeComponent * parent,
			std::size_t inputChannels,
			std::size_t outputChannels );

  ~Feedthrough();

private:
  AudioInput mInput;

  AudioOutput mOutput;

  detail::AudioTerminator mTerminator;
};

} // namespace feedthrough
} // namespace apps
} // namespace visr

#endif // #ifndef VISR_APPS_FEEDTHROUGH_SIGNAL_FLOW_HPP_INCLUDED
