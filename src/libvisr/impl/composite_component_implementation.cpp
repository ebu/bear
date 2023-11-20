/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "composite_component_implementation.hpp"

#include "component_implementation.hpp"
#include "audio_port_base_implementation.hpp"
#include "parameter_port_base_implementation.hpp"

#include "../audio_port_base.hpp"
#include "../channel_list.hpp"
#include "../parameter_port_base.hpp"
#include "../detail/compose_message_string.hpp"

#include <algorithm>
#include <ciso646>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace visr
{
namespace impl
{

CompositeComponentImplementation::CompositeComponentImplementation( CompositeComponent & component,
                                                                    SignalFlowContext const & context,
                                                                    char const * componentName,
                                                                    CompositeComponentImplementation * parent )
 : ComponentImplementation( component, context, componentName, parent )
{
}

CompositeComponentImplementation::~CompositeComponentImplementation()
{
  for( auto comp : mComponents )
  {
    comp->setParent( nullptr );
  }
}

/*virtual*/ bool CompositeComponentImplementation::isComposite() const
{
  return true;
}

void CompositeComponentImplementation::registerChildComponent( char const * name, ComponentImplementation * child )
{
  ComponentTable::iterator findComp = findComponentEntry( name );
  if( findComp != mComponents.end() )
  {
    throw std::invalid_argument( detail::composeMessageString("CompositeComponentImplementation::registerChildComponent(): Component named \"",
                                                              name, "\" already exists.") );
  }
  mComponents.push_back( child );
}

void CompositeComponentImplementation::unregisterChildComponent( ComponentImplementation * child )
{
  ComponentTable::iterator findComp = findComponentEntry( child->name().c_str() );
  if( findComp != mComponents.end() )
  {
    mComponents.erase( findComp );
  }
  else
  {
    std::cout << "CompositeComponent::unregisterChildComponent(): Child \"" << child->name() << "\" not found." << std::endl;
  }
}

CompositeComponentImplementation::ComponentTable::const_iterator
CompositeComponentImplementation::componentBegin() const
{
  return mComponents.begin();
}

CompositeComponentImplementation::ComponentTable::const_iterator
  CompositeComponentImplementation::componentEnd() const
{
  return mComponents.end();
}

CompositeComponentImplementation::ComponentTable::iterator
CompositeComponentImplementation::findComponentEntry( char const *componentName )
{
  ComponentTable::iterator findIt = std::find_if( mComponents.begin(), mComponents.end(),
    [componentName]( ComponentImplementation const * comp ) { return strcmp( componentName, comp->name().c_str()) == 0; } );
  return findIt;
}

CompositeComponentImplementation::ComponentTable::const_iterator
CompositeComponentImplementation::findComponentEntry( char const *componentName ) const
{
  ComponentTable::const_iterator findIt = std::find_if( mComponents.begin(), mComponents.end(),
    [componentName]( ComponentImplementation const * comp ) { return strcmp( componentName, comp->name().c_str()) == 0; } );
  return findIt;
}


ComponentImplementation * CompositeComponentImplementation::findComponent( char const * componentName )
{
  if( (componentName == nullptr) or (strlen( componentName ) == 0) or (strcmp( componentName, "this" ) == 0) )
  {
    return this;
  }
  ComponentTable::iterator findIt = findComponentEntry( componentName );
  return ( findIt == componentEnd() ) ? nullptr : *findIt;
}

ComponentImplementation const * CompositeComponentImplementation::findComponent( char const * componentName ) const
{
  if( (componentName == nullptr) or (strlen( componentName ) == 0) or (strcmp( componentName, "this" ) == 0) )
  {
    return this;
  }
  ComponentTable::const_iterator findIt = findComponentEntry( componentName );
  return (findIt == componentEnd()) ? nullptr : *findIt;
}

AudioPortBase * CompositeComponentImplementation::findAudioPort( char const * componentName, char const * portName )
{
  ComponentImplementation * comp = findComponent( componentName );
  if( not comp )
  {
    return nullptr; // Consider turning this into an exception and provide a meaningful message.
  }

  return comp->findAudioPort( portName );
}

ParameterPortBase * CompositeComponentImplementation::findParameterPort( char const * componentName, char const * portName )
{
  ComponentImplementation * comp = findComponent( componentName );
  if( not comp )
  {
    return nullptr; // Consider turning this into an exception and provide a meaningful message.
  }
  return comp->findParameterPort( portName );
}

namespace // unnamed
{
/**
 * Check whether the provided port argument is either an external port of the given composite component \p container or a port of a direct child
 * of \p container.
 * @throw std::invalid_argument If this condition does not hold.
 * @param port The port in question.
 * @param container The containing component.
 * @param portType A string describing the port type, e.g., "audio send" that is used to construct a human-readable error message.
 */
void checkPortParent( PortBaseImplementation const & port, CompositeComponentImplementation const & container, char const * portType )
{
  // An external port of the container.
  if( &port.parent() == &container )
  {
    return;
  }
  CompositeComponentImplementation const * parentContainer = port.parent().parent();
  if( parentContainer != &container )
  {
    throw std::invalid_argument( detail::composeMessageString(
      container.fullName(), ": the component \"", port.parent().fullName(), "\" containing the ", portType ,
        " port \"", port.name(), "\" is not a direct child of \"", container.fullName(), "\" ." ));
  }
}

} // unnamed namespace

void CompositeComponentImplementation::registerParameterConnection( char const * sendComponent,
                                                                    char const * sendPort,
                                                                    char const * receiveComponent,
                                                                    char const * receivePort )
{
  ParameterPortBase * sender = findParameterPort( sendComponent, sendPort );
  if( not sender )
  {
    throw std::invalid_argument( detail::composeMessageString( "CompositeComponent::registerParameterConnection(): send port \"",
        sendComponent, ":", sendPort, "\" could not be found.") );
  }
  ParameterPortBase * receiver = findParameterPort( receiveComponent, receivePort );
  if( not receiver )
  {
    throw std::invalid_argument( detail::composeMessageString( "CompositeComponent::registerParameterConnection(): receiver port \"",
        receiveComponent, ":", receivePort, "\" could not be found.") );
  }

  ParameterConnection newConnection( &(sender->implementation()), &(receiver->implementation()) );
  mParameterConnections.insert( std::move( newConnection ) );
}

void CompositeComponentImplementation::registerParameterConnection( ParameterPortBase & sendPort,
                                                                    ParameterPortBase & receivePort )
{
  checkPortParent( sendPort.implementation(), *this, "parameter send" );
  checkPortParent( receivePort.implementation(), *this, "parameter receive" );

  ParameterConnection newConnection( &(sendPort.implementation()), &(receivePort.implementation()) );
  mParameterConnections.insert( std::move( newConnection ) );
}

void CompositeComponentImplementation::audioConnection( char const * sendComponent,
                                                        char const * sendPort,
                                                        ChannelList const & sendIndices,
                                                        char const * receiveComponent,
                                                        char const * receivePort,
                                                        ChannelList const & receiveIndices )
{
  AudioPortBase * sender = findAudioPort( sendComponent, sendPort );
  if( not sender )
  {
    throw std::invalid_argument( detail::composeMessageString("CompositeComponent::audioConnection(): send port \"",
        sendComponent, ":", sendPort, "\" could not be found.") );
  }
  AudioPortBase * receiver = findAudioPort( receiveComponent, receivePort );
  if( not receiver )
  {
    throw std::invalid_argument( detail::composeMessageString("CompositeComponent::audioConnection(): receive port \"",
        receiveComponent, ":", receivePort, "\" could not be found.") );
  }
  AudioConnection newConnection( &(sender->implementation()), sendIndices, &(receiver->implementation()), receiveIndices );

  mAudioConnections.insert( std::move( newConnection ) );
}

namespace // unnamed
{
  // Local function to retrieve the maximum channel index
  ChannelList::IndexType maxChannelIndex( ChannelList const & list )
  {
    ChannelList::const_iterator findIt = std::max_element( list.begin(), list.end() );
    if( findIt == list.end() )
    {
      return 0;
    }
    return *findIt;
  }

  // Return a comma-separated list of channel indices exceeding a given width.
  std::string excessChannelList( ChannelList const & indices, ChannelList::IndexType width )
  {
    char const * sep = ", ";
    std::stringstream excessChannels;
    std::copy_if( indices.begin(), indices.end(),
                  std::ostream_iterator<ChannelList::IndexType>(excessChannels, sep ),
                  [width]( ChannelList::IndexType i ){ return i >= width;} );

    std::string ret = excessChannels.str();
    auto const pos = ret.rfind( sep );
    if( pos != std::string::npos )
    {
      ret.erase( pos );
    }
    return ret;
  }

} // unnamed namespace

void CompositeComponentImplementation::audioConnection( AudioPortBase & sendPort,
                                                        ChannelList const & sendIndices,
                                                        AudioPortBase & receivePort,
                                                        ChannelList const & receiveIndices )
{
  checkPortParent( sendPort.implementation(), *this, "audio send" );
  checkPortParent( receivePort.implementation(), *this, "audio receive" );
  if( sendIndices.size() != receiveIndices.size() )
  {
    throw std::invalid_argument( detail::composeMessageString("CompositeComponent::audioConnection(): send and receive index lists have different sizes (",
                                                              sendIndices.size(), " vs. ", receiveIndices.size(), ").") );
  }

  // Allow empty send and receive ranges.
  if( sendIndices.size() == 0 )
  {
    return; // No operation performed
  }
  if( maxChannelIndex( sendIndices ) >= sendPort.width() )
  {
    throw std::invalid_argument( detail::composeMessageString("CompositeComponent::audioConnection(): The send channel indices [",
                                                              excessChannelList( sendIndices, sendPort.width() ),
                                                              "] exceed the port width ", sendPort.width(), ".") );
  }
  if( maxChannelIndex( receiveIndices ) >= receivePort.width() )
  {
    throw std::invalid_argument( detail::composeMessageString("CompositeComponent::audioConnection(): The receive channel indices [",
                                                              excessChannelList( receiveIndices, receivePort.width() ),
                                                              "] exceed the port width ", receivePort.width(), ".") );
  }
  AudioConnection newConnection( &(sendPort.implementation()), sendIndices,
                                 &(receivePort.implementation()), receiveIndices );
  mAudioConnections.insert( std::move( newConnection ) );
}

void CompositeComponentImplementation::audioConnection( AudioPortBase & sendPort,
                                                        AudioPortBase & receivePort )
{
  AudioPortBaseImplementation & sendPortImpl = sendPort.implementation();
  AudioPortBaseImplementation & receivePortImpl = receivePort.implementation();
  std::size_t const sendWidth = sendPortImpl.width();
  std::size_t const receiveWidth = receivePortImpl.width();
  if( sendWidth != receiveWidth )
  {
    std::string const sendName = sendPort.implementation().parent().name() + "." + sendPort.implementation().name();
    std::string const receiveName = receivePort.implementation().parent().name() + "." + receivePort.implementation().name();
    throw std::invalid_argument( detail::composeMessageString( "CompositeComponent::audioConnection(): width of send port\"", sendName, "\" (", sendWidth, ") and of receive port \"",
                                                               receiveName, "\" (", receiveWidth, ") do not match.") );
  }
  ChannelList const indices( ChannelRange( 0, sendWidth ) );
  audioConnection( sendPort, indices, receivePort, indices );
}

AudioConnectionTable const & CompositeComponentImplementation::audioConnections() const
{
  return mAudioConnections;
}

AudioConnectionTable::const_iterator CompositeComponentImplementation::audioConnectionBegin() const
{
  return mAudioConnections.begin();
}

AudioConnectionTable::const_iterator CompositeComponentImplementation::audioConnectionEnd() const
{
  return mAudioConnections.end();
}

ParameterConnectionTable::const_iterator CompositeComponentImplementation::parameterConnectionBegin() const
{
  return mParameterConnections.begin();
}

ParameterConnectionTable::const_iterator CompositeComponentImplementation::parameterConnectionEnd() const
{
  return mParameterConnections.end();
}

} // namespace impl
} // namespace visr
