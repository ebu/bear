/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_IMPL_COMPONENT_IMPLEMENTATION_HPP_INCLUDED
#define VISR_IMPL_COMPONENT_IMPLEMENTATION_HPP_INCLUDED

#include "time_implementation.hpp"

#include <libvisr/constants.hpp>
#include <libvisr/export_symbols.hpp>
#include <libvisr/status_message.hpp>
#include <libvisr/signal_flow_context.hpp>
#include <libvisr/time.hpp>

#include <cstddef>
#include <string>
#include <vector>
#include <memory>

namespace visr
{
// Forward declaration(s)
class AudioPortBase;
class Component;
class ParameterPortBase;

namespace impl
{

class CompositeComponentImplementation;
class AudioPortBaseImplementation;
class ParameterPortBaseImplementation;

/**
 *
 *
 */
class VISR_CORE_LIBRARY_SYMBOL ComponentImplementation
{
public:
  friend class AudioPortBaseImplementation; // For registering/ unregistering audio ports
  friend class ParameterPortBaseImplementation; // For registering / unregistering audio ports.
  friend class CompositeComponentImplementation; // For unsetting the parent member during destruction.
  
  /*VISR_CORE_LIBRARY_SYMBOL*/ explicit ComponentImplementation( visr::Component & component,
                                                                 SignalFlowContext const & context,
                                                                 char const * componentName,
                                                                 CompositeComponentImplementation * parent );

  /**
   *
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ virtual ~ComponentImplementation();

  ComponentImplementation() = delete;

  ComponentImplementation( ComponentImplementation const & ) = delete;

  ComponentImplementation( ComponentImplementation && ) = delete;

  ComponentImplementation& operator=( ComponentImplementation const & ) = delete;

  ComponentImplementation& operator=( ComponentImplementation && ) = delete;


  /*VISR_CORE_LIBRARY_SYMBOL*/ static const std::string cNameSeparator;

  /**
   * Return the 'local', non-hierarchical name.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::string const & name() const;

  /**
   * Return the full, hierarchical name of the component.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::string fullName() const;

  /**
   * Query whether the corresponding component is atomic or composite.
   * @todo Check whether the current approach of storing this information in the
   * class hierarchy of the externally visible components is the right way to go,
   * or whether this information should be held local in the internal object.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ virtual bool isComposite() const;

  /**
   * Return the sampling frequency of the containing signal flow.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ SamplingFrequencyType samplingFrequency() const;

  /**
   * Return the period of the containing signal processing graph,
   * i.e., the number of samples processed in each invocation of the
   * process function of the derived audio components.
   * This methods can be called at any point of the lifetime of the
   * derived component, i.e., for instance in the constructor.
   * @todo: Check whether this should be made inline again (adding the dependency to the runtime container (aka SignalFlow).
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::size_t period() const;

  template< class PortType >
  using PortContainer = std::vector<PortType*>;

  using AudioPortContainer = PortContainer< AudioPortBaseImplementation >;

  /**
   * Allow access to the port lists 
   */
  //@{
  /*VISR_CORE_LIBRARY_SYMBOL*/ AudioPortContainer const & audioPorts() const { return mAudioPorts; }

  /*VISR_CORE_LIBRARY_SYMBOL*/ AudioPortContainer & audioPorts() { return mAudioPorts; }

  /*VISR_CORE_LIBRARY_SYMBOL*/ AudioPortContainer::const_iterator audioPortBegin() const { return mAudioPorts.begin(); }

  /*VISR_CORE_LIBRARY_SYMBOL*/ AudioPortContainer::const_iterator audioPortEnd( ) const { return mAudioPorts.end(); }
  //@}

  /**
   * Parameter port support
   */
  //@{
  using ParameterPortContainer = PortContainer<ParameterPortBaseImplementation>;

  /*VISR_CORE_LIBRARY_SYMBOL*/ ParameterPortContainer const & parameterPorts() const { return mParameterPorts; }

  /*VISR_CORE_LIBRARY_SYMBOL*/ ParameterPortContainer & parameterPorts() { return mParameterPorts; }

  /*VISR_CORE_LIBRARY_SYMBOL*/ ParameterPortContainer::const_iterator parameterPortBegin() const;
  /*VISR_CORE_LIBRARY_SYMBOL*/ ParameterPortContainer::const_iterator parameterPortEnd( ) const;

  /*VISR_CORE_LIBRARY_SYMBOL*/ ParameterPortContainer::iterator parameterPortBegin( );
  /*VISR_CORE_LIBRARY_SYMBOL*/ ParameterPortContainer::iterator parameterPortEnd( );

  /**
   * Uniform access to audio and parameter ports using templates
   */
  //@{


  /**
   * Return the port container for the specified port type, const version .
   * This template method is explicitly instantiated for the two possible port types AudioPort and ParameterPortBase
   * @tparam PortType
   * @return a const reference to the port container.
   */
  template<class PortType>
  /*VISR_CORE_LIBRARY_SYMBOL*/ PortContainer<PortType> const & ports() const;

  /**
   * Return the port container for the specified port type, non-const version.
   * This template method is explicitly instantiated for the two possible port types AudioPort and ParameterPortBase
   * @tparam PortType
   * @return a modifiable reference to the port container.
   */
  template<class PortType>
  /*VISR_CORE_LIBRARY_SYMBOL*/ PortContainer<PortType> & ports();

  template<class PortType>
  typename PortContainer<PortType>::iterator portBegin() { return ports<PortType>().begin(); }
  template<class PortType>
  typename PortContainer<PortType>::iterator portEnd() { return ports<PortType>().end(); }
  template<class PortType>
  typename PortContainer<PortType>::const_iterator portBegin() const { return ports<PortType>().begin(); }
  template<class PortType>
  typename PortContainer<PortType>::const_iterator portEnd() const { return ports<PortType>().end(); }

  template<class PortType>
  typename PortContainer<PortType>::const_iterator findPortEntry( char const * portName ) const;

  template<class PortType>
  typename PortContainer<PortType>::iterator findPortEntry( char const * portName );
  //@}

  /**
   * Register a parameter port in the component. Generally performed in the port's constructor.
   * @todo consider making this a template method to share the implementation between audio and parameter ports.
   */
  void registerParameterPort( ParameterPortBaseImplementation * port );

  /**
   * Unregister a parameter port in the component. Generally performed in the port's destructor.
   * @todo consider making this a template method to share the implementation between audio and parameter ports.
   */
  bool unregisterParameterPort( ParameterPortBaseImplementation * port );

  /**
   * Find a named parameter port within the component and return an iterator into the port container.
   * @return A valid iterator into the port container for parameter ports, or the end() iterator if a port of this name is not found.
   * @todo Templatise these calls as well
   */
  ParameterPortContainer::iterator findParameterPortEntry( char const * portName );

  /**
   * Find a named parameter port within the component and return an iterator into the port container, const verstion.
   * @return A valid iterator into the port container for parameter ports, or the end() iterator if a port of this name is not found.
   * @todo Templatise these calls as well
   */
  ParameterPortContainer::const_iterator findParameterPortEntry( char const * portName ) const;

  /**
   * @return pointer to port, nullptr in case the port is not found.
   */
  ParameterPortBase const * findParameterPort( char const * portName ) const;

  /**
  * @return pointer to port, nullptr in case the port is not found.
  */
  ParameterPortBase * findParameterPort( char const * portName );

  /**
  * @return pointer to port, nullptr in case the port is not found.
  */
  AudioPortBase* findAudioPort( char const * name );

  /**
  * @return pointer to port, nullptr in case the port is not found.
  */
  AudioPortBase const * findAudioPort( char const * name ) const;

  /**
   * Report a status message, either informational or an error condition.
   */
  void status( StatusMessage::Kind statusId, char const * message );

  /**
   * Query whether the component is at the top level of a signal flow.
   * @note Not needed for user API
   */
  bool isTopLevel() const { return mParent == nullptr; }

  /**
   * Return a pointer to the implementation object of the parent component, or nullptr
   * if this is a top-level component.
   */
  CompositeComponentImplementation * parent() { return mParent; }

  /**
   * Return a const pointer to the implementation object of the parent component, or nullptr
   * if this is a top-level component.
   */
  CompositeComponentImplementation const * parent() const { return mParent; }

  /**
   * Return a reference to the externally visible part of the component.
   * This is required for atomic components to have their (overridden) process() method called.
   * @todo Maybe implement a separate impl::AtomicComponent class for that
   */
  //@{
  visr::Component & component() { return mComponent; }

  visr::Component const & component() const { return mComponent; }
  //@}

  SignalFlowContext const & context( ) const { return mContext; }

  /**
   * Retrieve a reference to the object representing the time subsystem, const version,
   */
  Time const & time() const;
   
  /**
    * Retrieve a reference to the object representing the time subsystem, nonconst version.
    */
  Time & time();
    
  /**
   * Retrieve the implementation object of the time subsystem.
   *  This method is public to be used by the runtime system.
   */
  TimeImplementation const & timeImplementation() const;

  /**
   * Retrieve the implementation object of the time subsystem, nonconst version
   *  This method is public to be used by the runtime system.
   */
  TimeImplementation & timeImplementation();
    
private:
  /**
   * Set the parent component. Providing nullptr effectively removes
   * the association to parent.
   * @todo Need to update the reference to the time method?
   */
  void setParent( CompositeComponentImplementation * parent );


  /**
   * Register a port with a type and a unique name within the port.
   * @param port The port to be registered.
   * @throw In case of a non-unique or invalid port name
   */
  void registerAudioPort( AudioPortBaseImplementation* port );
  void unregisterAudioPort( AudioPortBaseImplementation* port );

  AudioPortContainer mAudioPorts;

  AudioPortContainer const & getAudioPortList( ) const;

  AudioPortContainer& getAudioPortList( );

  AudioPortContainer::iterator findAudioPortEntry( char const * portName );

  AudioPortContainer::const_iterator findAudioPortEntry( char const * portName ) const;

  /**
   * The Component object corresponding to this 'internal' representation.
   */
  visr::Component & mComponent;

  /**
   * The context object hiolding buffer size and sampling frequency.
   * These vakues are not supposed to be changed during the lifetime of a component.
   */
  SignalFlowContext const mContext;

  /**
   * Parameter port subsystem
   */
  //@{
  ParameterPortContainer mParameterPorts;

  //@}
  /**
   * The local, i.e., nonhierarchical, name of the component.
   */
  std::string const mName;

  /**
   * The direct parent component if this component is part of a
   * superordinate signal flow graph, and nullptr if this is the
   * top-level component.
   * Note: We link directly to the implementation object (might be renamed to 'internal')
   */
  CompositeComponentImplementation * mParent;
    
  /**
   * Internal representation for the time interface exposed to components.
   */
  Time mTime;
};

} // namespace impl
} // namespace visr

#endif // #ifndef VISR_IMPL_COMPONENT_IMPLEMENTATION_HPP_INCLUDED
