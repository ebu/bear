/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_COMPONENT_HPP_INCLUDED
#define VISR_COMPONENT_HPP_INCLUDED

#include "constants.hpp"

#include "export_symbols.hpp"
#include "status_message.hpp"
#include "detail/compose_message_string.hpp"

#include <cstddef>
#include <string>
#include <sstream>

namespace visr
{

// Forward declaration(s)
class AudioPortBase;
class CompositeComponent;
class ParameterPortBase;
class SignalFlowContext;
class Time;

namespace impl
{
class ComponentImplementation;
}

/**
 * Base class for processing components.
 * Components may contain ports to exchange data (either audio signal or parameter)
 * with other components or with the exterior.
 * A component may have a parent, that is, a composite component it is contained in. 
 * If the parent is null it is a top-level component.
 * Components also have a name, which must be unique within a containing composite component.
 */
class VISR_CORE_LIBRARY_SYMBOL Component
{
public:

  /**
   * Constructor, constructs a component.
   * @param context Configuration object containing basic execution parameters
   * (such as sampling frequency and period (block length))
   * @param componentName The name of the component. If this component is contained
   * in a higher-level parent component, the name must be unique within that parent component
   * @param parent Pointer to the containing composite component, if there is one. Otherwise,
   * that is, if the present component is at the top level, pass \p nullptr.
   */
  explicit Component( SignalFlowContext const & context,
                      char const * componentName,
                      CompositeComponent * parent );

  /**
   * Constructor.
   * Convenvience function, accepts a standard string instead of a C chararacter pointer.
   * @param context Configuration object containing basic execution parameters
   * (such as sampling frequency and period (block length))
   * @param componentName The name of the component. If this component is contained
   * in a higher-level parent component, the name must be unique within that parent component.
   * @param parent Pointer to the containing composite component, if there is one. Otherwise,
   * that is, if the present component is at the top level, pass \p nullptr.
   */
  explicit Component( SignalFlowContext const & context,
                      std::string const & componentName,
                      CompositeComponent * parent);

  /**
   * Deleted copy constructor to avoid copy construction of this and derived classes.
   */
  Component( Component const & ) = delete;

   /**
   * Deleted move constructor to avoid moving of this and derived classes.
   */
  Component( Component && ) = delete;

  /**
   * Deleted assignment operator to prohibit (copy) assignment of this and derived classes.
   */
  Component & operator=( Component const & ) = delete;

  /**
  * Deleted assignment operator to prohibit move assignment of this and derived classes.
  */
  Component & operator=( Component && ) = delete;

  /**
   * Separator used to form hierarchical names.
   */
  static const std::string & nameSeparator();

  /**
   * Destructor (virtual)
   */
  virtual ~Component();

  /**
   * Return the 'local', non-hierarchical name.
   */
  std::string const & name() const;

  /**
   * Return the full, hierarchical name of the component.
   */
  std::string fullName() const;

  /**
   * Signal informational messages or the error conditions.
   * Depending on the value of the \p status parameter, this might result
   * in a message conveyed to the user or abortion of the audio processing.
   * @param status The class of the status message
   * @param message An informational message string.
   */
  void status( StatusMessage::Kind status, char const * message );

  /**
  * Signal informational messages or the error conditions where the message string is 
  * constructed from an arbitrary sequence of arguments.
  * Depending on the value of the \p status parameter, this might result
  * in a message conveyed to the user or abortion of the audio processing.
  * @tparam MessageArgs List of argument types to be printed. Normally they are automatically 
  * determined by the compiler, so there is no need to specify them.
  * @param status The class of the status message
  * @param args Comma-seprated list of parameters with unspecified types. The main requirement 
  * is that all types support an "<<" operator.
  */
  template<typename ... MessageArgs >
  void status( StatusMessage::Kind status, MessageArgs ... args );

  /**
   * Query whether this component is atomic (i.e., a piece of code implementing a rendering 
   * functionality) or a composite consisting of an interconnection of atomic (or further composite) components.
   */
  bool isComposite() const;

  AudioPortBase& audioPort( char const * portName );

  AudioPortBase const& audioPort( char const * portName ) const;

  AudioPortBase& audioPort( std::string const & portName );

  AudioPortBase const& audioPort( std::string const & portName ) const;


  ParameterPortBase& parameterPort( char const * portName );

  ParameterPortBase const& parameterPort( char const * portName ) const;

  ParameterPortBase& parameterPort( std::string const & portName );

  ParameterPortBase const& parameterPort( std::string const & portName ) const;


  /**
   * Return the sampling frequency of the containing signal flow.
   */
  SamplingFrequencyType samplingFrequency() const;

  /**
   * Return the period of the containing signal processing graph,
   * i.e., the number of samples processed in each invocation of the
   * process function of the derived audio components.
   * This methods can be called at any point of the lifetime of the
   * derived component, i.e., for instance in the constructor.
   * @todo: Check whether this should be made inline again (adding the dependency to the runtime container (aka SignalFlow).
   */
  std::size_t period() const;

  //@}

  /**
   * Query whether the component is at the top level of a signal flow.
   * @note Not needed for user API
   */
  bool isTopLevel() const;

  /**
   * Return a reference to the time (i.e., clock) interface.
   * @note At the moment, only const access is supported.
   * @todo Consider whether components should have access to modifying operations, and
   * in this add a non-const access function.
   */
  Time const & time() const;
    
  /**
   * Provide a pointer to an external implementation object.
   * The type of this implementation object is opaque, i.e., not visible from the public VISR API.
   * @note This method is not supposed to be called in user code. It is public because it is is used
   * by the VISR runtime system.
   */
  impl::ComponentImplementation & implementation();

  /**
   * Provide a pointer to an external implementation object, constant version.
   * The type of this implementation object is opaque, i.e., not visible from the public VISR API.
   * @note This method is not supposed to be called in user code. It is public because it is is used
   * by the VISR runtime system.
   */
  impl::ComponentImplementation const & implementation() const;

protected:
  /**
   * Constructor that receives the internal implementation object.
   * This overload has to be called by the other constructors (including those of subclasses) to make 
   * sure that the implementation object is instantiated.
   * The motivation for this constructor is to provide different implementation objects for different subclasses.
   */
  explicit Component( impl::ComponentImplementation * impl );

private:
  /**
   * Pointer to the private implementation object.
   * The type of the impl object might differ due to the actual type of the component.
   * @note This is intentionally a plain pointer (as opposed to a smart pointer)in order not to reveal or 
   * constrain the possible implementations within the runtime system.
   */
  impl::ComponentImplementation* mImpl;
};

// template method implementation.
template<typename ... MessageArgs >
inline void Component::status( StatusMessage::Kind statusId, MessageArgs ... args )
{
  status( statusId, detail::composeMessageString( args ... ).c_str() );
}

} // namespace visr

#endif // #ifndef VISR_COMPONENT_HPP_INCLUDED
