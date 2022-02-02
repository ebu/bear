/* Copyright Institute of Sound and Vibration Research - All rights reserved */


#include "component.hpp"
#include "composite_component.hpp"
#include "signal_flow_context.hpp"

#include "impl/component_implementation.hpp"

#include <ciso646>
#include <exception>
#include <iostream>
#include <utility>

namespace visr
{

Component::Component( impl::ComponentImplementation * impl )
  : mImpl( impl )
{
}

Component::Component( SignalFlowContext const & context,
                      char const * componentName,
                      CompositeComponent * parent)
 : Component( new impl::ComponentImplementation( *this, context, componentName,
              parent == nullptr ? nullptr : &(parent->implementation()) ) )
{
}

Component::Component( SignalFlowContext const & context,
                      std::string const & componentName,
                      CompositeComponent * parent)
 : Component( context, componentName.c_str(), parent )
{
}

Component::~Component()
{
  if( mImpl )
  {
    delete mImpl;
    mImpl = nullptr;
  }
}

/*static*/ std::string const & Component::nameSeparator()
{
  return impl::ComponentImplementation::cNameSeparator;
}

std::string const & Component::name() const
{
  std::string const & nm = mImpl->name();
  return nm;
}

std::string Component::fullName() const
{
  return mImpl->fullName();
}

void Component::status( StatusMessage::Kind statusId, char const * message )
{
  mImpl->status( statusId, message );
}

bool Component::isTopLevel() const
{
  return mImpl->isTopLevel();
}

bool Component::isComposite() const
{
  return mImpl->isComposite();
}

AudioPortBase& Component::audioPort( char const * portName )
{
  AudioPortBase * port = mImpl->findAudioPort( portName );
  if( port )
  {
    return *port;
  }
  throw std::invalid_argument( detail::composeMessageString("Audio port \"", portName, "\" not found in component \"", name(), "\".") );
}

AudioPortBase const& Component::audioPort( char const * portName ) const
{
  AudioPortBase const * port = mImpl->findAudioPort( portName );
  if( port )
  {
    return *port;
  }
  throw std::invalid_argument( detail::composeMessageString("Audio port \"", portName, "\" not found in component \"", name(), "\".") );
}

AudioPortBase& Component::audioPort( std::string const & portName )
{
  return audioPort( portName.c_str() );

}

AudioPortBase const& Component::audioPort( std::string const & portName ) const
{
  return audioPort( portName.c_str() );
}

ParameterPortBase& Component::parameterPort( char const * portName )
{
  ParameterPortBase * port = mImpl->findParameterPort( portName );
  if( port )
  {
    return *port;
  }
  throw std::invalid_argument( detail::composeMessageString("Parameter port \"", portName, "\" not found in component \"", name(), "\".") );
}

ParameterPortBase const& Component::parameterPort( char const * portName ) const
{
  ParameterPortBase const * port = mImpl->findParameterPort( portName );
  if( port )
  {
    return *port;
  }
  throw std::invalid_argument( detail::composeMessageString("Parameter port \"", portName, "\" not found in component \"", name(), "\".") );
}

ParameterPortBase& Component::parameterPort( std::string const & portName )
{
  return parameterPort( portName.c_str() );
}

ParameterPortBase const& Component::parameterPort( std::string const & portName ) const
{
  return parameterPort( portName.c_str() );
}

impl::ComponentImplementation & Component::implementation()
{
  return *mImpl;
}

impl::ComponentImplementation const & Component::implementation() const
{
  return *mImpl;
}

std::size_t Component::period() const { return mImpl->period(); }

SamplingFrequencyType Component::samplingFrequency() const { return mImpl->samplingFrequency(); }

Time const & Component::time() const
{
  return mImpl->time();
}

} // namespace visr
