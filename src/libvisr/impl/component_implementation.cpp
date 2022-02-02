/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "component_implementation.hpp"

#include "audio_port_base_implementation.hpp"
#include "parameter_port_base_implementation.hpp"
#include "composite_component_implementation.hpp"

#include "../audio_port_base.hpp"
#include "../composite_component.hpp"
#include "../parameter_port_base.hpp"
#include "../signal_flow_context.hpp"

#include <algorithm>
#include <cassert>
#include <ciso646>
#include <cstring>
#include <exception>
#include <iostream>
#include <utility>

namespace visr
{
namespace impl
{

ComponentImplementation::ComponentImplementation( Component & component, 
                                                  SignalFlowContext const & context,
                                                  char const * componentName,
                                                  CompositeComponentImplementation * parent)
 : mComponent( component )
 , mContext( context )
 , mName( componentName )
 , mParent( parent )
 , mTime( parent == nullptr
         ? std::shared_ptr<TimeImplementation>(new TimeImplementation( context ))
         : parent->time().mImpl )
{
  assert( mTime.mImpl != nullptr );
  if( parent != nullptr )
  {
    parent->registerChildComponent( componentName, this );
  }
}

ComponentImplementation::~ComponentImplementation()
{
  if( not isTopLevel() )
  {
    mParent->unregisterChildComponent( this );
  }
}

/*static*/ const std::string ComponentImplementation::cNameSeparator = ":";

std::string const & ComponentImplementation::name() const
{ 
  return mName;
}

std::string ComponentImplementation::fullName() const
{
  if( isTopLevel() or mParent->isTopLevel() )
  {
    return name();
  }
  else
  {
    return mParent->fullName() + cNameSeparator + name();
  }
}

void ComponentImplementation::status( StatusMessage::Kind statusId, char const * message )
{
  std::cout << fullName() << ": Message id: " << statusId << ": " << message << std::endl;
  // Trivial default implementation, ought to be replaced be a more sophisticated solution
  // (should be passed to the runtime system and handled dependent on the software environment)
  if( (statusId == StatusMessage::Error) or (statusId == StatusMessage::Critical) )
  {
    throw std::runtime_error( detail::composeMessageString( fullName(), ": \"", message, "\"."));
  }
}

/*virtual*/ bool ComponentImplementation::isComposite() const
{
  return false;
}


ComponentImplementation::AudioPortContainer const&
ComponentImplementation::getAudioPortList()  const
{
  return mAudioPorts;
}


ComponentImplementation::AudioPortContainer&
ComponentImplementation::getAudioPortList( )
{
  return mAudioPorts;
}
 
std::size_t ComponentImplementation::period() const { return mContext.period(); }

// bool ComponentImplementation::initialised() const  { return mContext.initialised(); }

SamplingFrequencyType ComponentImplementation::samplingFrequency() const { return mContext.samplingFrequency(); }

Time & ComponentImplementation::time()
{
  return mTime;
}

Time const & ComponentImplementation::time() const
{
  return mTime;
}

TimeImplementation const & ComponentImplementation::
timeImplementation() const
{
  return mTime.implementation();
}

TimeImplementation & ComponentImplementation::
timeImplementation()
{
  return mTime.implementation();
}


void ComponentImplementation::setParent( CompositeComponentImplementation * parent )
{
  mParent = parent;
}

void ComponentImplementation::unregisterAudioPort( AudioPortBaseImplementation* port )
{
  // According to C++11, findIt may be a const iterator, but the standard library of gcc 4.8 does not permit that.
  AudioPortContainer::iterator findIt = std::find( mAudioPorts.begin(), mAudioPorts.end(), port );
  if( findIt != mAudioPorts.end() )
  {
    (*findIt)->removeParent();
    mAudioPorts.erase( findIt );
  }
  else
  {
    std::cerr << "Component::unregisterAudioPort(): port was not registered." << std::endl;
  }
}

void ComponentImplementation::registerAudioPort( AudioPortBaseImplementation* port )
{
  AudioPortContainer & vec = getAudioPortList( );
  AudioPortContainer::const_iterator findIt = findAudioPortEntry( port->name() );
  if( findIt != vec.end( ) )
  {
    throw std::invalid_argument( "Component::registerAudioPort(): port with given name already exists" );
  }
  vec.push_back( port );
}

struct ComparePorts
{
  explicit ComparePorts( char const * name ): mName( name ) {}

  bool operator()( PortBaseImplementation const * lhs ) const
  {
    return ::strcmp( lhs->name(), mName ) == 0;
  }
private:
  char const * mName;
};

ComponentImplementation::AudioPortContainer::iterator ComponentImplementation::findAudioPortEntry( char const * portName )
{
  AudioPortContainer::iterator findIt
    = std::find_if( mAudioPorts.begin( ), mAudioPorts.end( ), ComparePorts( portName ) );
  return findIt;
}

ComponentImplementation::AudioPortContainer::const_iterator ComponentImplementation::findAudioPortEntry( char const * portName ) const
{
  AudioPortContainer::const_iterator findIt
    = std::find_if( mAudioPorts.begin( ), mAudioPorts.end( ), ComparePorts( portName ) );
  return findIt;
}

AudioPortBase const * ComponentImplementation::findAudioPort( char const * portName ) const
{
  AudioPortContainer::const_iterator findIt = findAudioPortEntry( portName );
  if( findIt == audioPortEnd() )
  {
    return nullptr;
  }
  return &((*findIt)->containingPort());
}

AudioPortBase * ComponentImplementation::findAudioPort( char const * portName )
{
  AudioPortContainer::iterator findIt = findAudioPortEntry( portName );
  if( findIt == audioPortEnd( ) )
  {
    return nullptr;
  }
  return &((*findIt)->containingPort());
}

// Parameter port related stuff
ComponentImplementation::ParameterPortContainer::const_iterator 
ComponentImplementation::parameterPortBegin() const
{
  return portBegin<ParameterPortBaseImplementation>();
}

ComponentImplementation::ParameterPortContainer::const_iterator 
ComponentImplementation::parameterPortEnd() const
{
  return portEnd<ParameterPortBaseImplementation>();
}

ComponentImplementation::ParameterPortContainer::iterator
ComponentImplementation::parameterPortBegin( )
{
  return portBegin<ParameterPortBaseImplementation>( );
}

ComponentImplementation::ParameterPortContainer::iterator
ComponentImplementation::parameterPortEnd( )
{
  return portEnd<ParameterPortBaseImplementation>( );
}

void ComponentImplementation::registerParameterPort( ParameterPortBaseImplementation * port )
{
  ParameterPortContainer::const_iterator findIt = findParameterPortEntry( port->name() );
  if( findIt != mParameterPorts.end() )
  {
    throw std::invalid_argument( "ComponentImplementation::registerParameterPort(): port with given name already exists" );
  }
  mParameterPorts.push_back( port );
}

bool ComponentImplementation::unregisterParameterPort( ParameterPortBaseImplementation * port )
{
  // According to C++11, findIt may be a const iterator, but the standard library of gcc 4.8 does not permit that.
  ParameterPortContainer::iterator findIt = std::find( mParameterPorts.begin(), mParameterPorts.end(), port );
  if( findIt != mParameterPorts.end() )
  {
    mParameterPorts.erase( findIt );
  }
  else
  {
    std::cerr << "Component::unregisterParameterPort(): port was not registered." << std::endl;
    return false;
  }
  return true;
}

ComponentImplementation::ParameterPortContainer::iterator ComponentImplementation::findParameterPortEntry( char const * portName )
{
  return findPortEntry<ParameterPortBaseImplementation>( portName );
}

ComponentImplementation::ParameterPortContainer::const_iterator ComponentImplementation::findParameterPortEntry( char const * portName ) const
{
  return findPortEntry<ParameterPortBaseImplementation>( portName );
}

ParameterPortBase const * ComponentImplementation::findParameterPort( char const * portName ) const
{
  ParameterPortContainer::const_iterator findIt = findParameterPortEntry( portName );
  if( findIt == parameterPortEnd() )
  {
    return nullptr;
  }
  return &((*findIt)->containingPort());
}

ParameterPortBase * ComponentImplementation::findParameterPort( char const * portName )
{
  ParameterPortContainer::iterator findIt = findParameterPortEntry( portName );
  if( findIt == parameterPortEnd() )
  {
    return nullptr;
  }
  return &((*findIt)->containingPort());
}

template<>
VISR_CORE_LIBRARY_SYMBOL ComponentImplementation::PortContainer<AudioPortBaseImplementation> const & ComponentImplementation::ports() const { return mAudioPorts; }

template<>
VISR_CORE_LIBRARY_SYMBOL ComponentImplementation::PortContainer<ParameterPortBaseImplementation> const & ComponentImplementation::ports() const { return mParameterPorts; }

template<>
VISR_CORE_LIBRARY_SYMBOL ComponentImplementation::PortContainer<AudioPortBaseImplementation> & ComponentImplementation::ports() { return mAudioPorts; }

template<>
VISR_CORE_LIBRARY_SYMBOL ComponentImplementation::PortContainer<ParameterPortBaseImplementation> & ComponentImplementation::ports() { return mParameterPorts; }

// Strange workaround needed for Visual Studio to prevent an error when using the return type
// TypedPortContainer = ComponentImplementation::PortContainer<PortType>::(const_)iterator direclty in the findPortEntry() definitions below.
// This resulted in error C2244: 'unable to match function definition to an existing declaration'
template <class PortType>
using TypedPortContainer = ComponentImplementation::PortContainer<PortType>;

template<class PortType>
typename TypedPortContainer<PortType>::const_iterator ComponentImplementation::findPortEntry( char const * portName ) const
{
  typename PortContainer<PortType>::const_iterator findIt
    = std::find_if( portBegin<PortType>(), portEnd<PortType>(), ComparePorts( portName ) );
  return findIt;
}
// Explicit instantiations for findPortEntry.
// Note: The individual documentations are necessary to avoid doxygen warnings.

/**
 * @relates ComponentImplementation
 * Explicit instantiation of template findPortEntry for port type AudioPortBaseImplementation, const variant..
 */
template ComponentImplementation::PortContainer<AudioPortBaseImplementation>::const_iterator
ComponentImplementation::findPortEntry<AudioPortBaseImplementation>( char const * portName ) const;
/**
 * @relates ComponentImplementation
 * Explicit instantiation of template findPortEntry for port type ParameterPortBaseImplementation, const variant..
 */
template ComponentImplementation::PortContainer<ParameterPortBaseImplementation>::const_iterator
ComponentImplementation::findPortEntry<ParameterPortBaseImplementation>( char const * portName ) const;

template<class PortType>
typename TypedPortContainer<PortType>::iterator ComponentImplementation::findPortEntry( char const * portName )
{
  typename PortContainer<PortType>::iterator findIt
    = std::find_if( portBegin<PortType>(), portEnd<PortType>(), ComparePorts( portName ) );
  return findIt;
}
// Explicit instantiations
/**
 * @relates ComponentImplementation
 * Explicit instantiation of template findPortEntry for port type AudioPortBaseImplementation, non-const variant.
 */
template ComponentImplementation::PortContainer<AudioPortBaseImplementation>::iterator ComponentImplementation::findPortEntry<AudioPortBaseImplementation>( const char* portName );
/**
 * @relates ComponentImplementation
 * Explicit instantiation of template findPortEntry for port type ParameterPortBaseImplementation, non-const variant..
 */
template ComponentImplementation::PortContainer<ParameterPortBaseImplementation>::iterator ComponentImplementation::findPortEntry<ParameterPortBaseImplementation>( char const * portName );

} // namespace impl
} // namespace visr
