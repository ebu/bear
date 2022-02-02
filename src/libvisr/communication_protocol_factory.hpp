/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_COMMUNICATION_PROTOCOL_FACTORY_HPP_INCLUDED
#define VISR_COMMUNICATION_PROTOCOL_FACTORY_HPP_INCLUDED

#include <memory>

#include "communication_protocol_base.hpp"
#include "communication_protocol_type.hpp"
#include "export_symbols.hpp"
#include "parameter_type.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace visr
{

// Forward declarations
class ParameterBase;
class ParameterConfigBase;

/**
 * Class encapsulating functionality to retrieve information about and to create communication protocol objects and related objects.
 *
 */
class VISR_CORE_LIBRARY_SYMBOL CommunicationProtocolFactory
{
public:
  /**
   * Deleted default constructor as this class is to be used only through static functions.
   */
  CommunicationProtocolFactory() = delete;

  /**
   * Create a communication protocol object based of a given protocol type.
   * This function does not need to be used by user API users.
   * @param protocolType The type id of the requested communication protocol.
   * @param paramType The type id of the parameter type to be transmitted by the protocol.
   * @param config A parameter configuration object holding configuration data about the parameter objects to be created and transmitted by the protocol.
   * This object is to be copied internally, therefore the lifetime of the referenced object needs to persist only for the durarion of the function call.
   * @return A dynamically allocated object contained in a std::unique_ptr. The caller is responsible for freeing this object (done automatically if it is kept in a smart pointer).
   * @throw std::out_of_range If no protocol with type id \p protocolType is registered in the factory.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static std::unique_ptr<CommunicationProtocolBase> createProtocol( CommunicationProtocolType const & protocolType,
                                                                                                 ParameterType const & paramType,
                                                                                                 ParameterConfigBase const & config );

  /**
   * Create an input end point which accepts the data from a protocol (typically sitting in a parameter input port).
   * @param protocolType The type id of the communication protocol for which the input end point is going to be created.
   * @return A dynamically allocated object contained in a std::unique_ptr. The caller is responsible for freeing this object (done 
   * automatically if it is kept in a smart pointer).
   * @throw std::out_of_range If no protocol with type id \p protocolType is registered in the factory.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static std::unique_ptr<CommunicationProtocolBase::Input> createInput( CommunicationProtocolType const & protocolType );

  /**
  * Create an output end point that receives data and transmits it to a protocol object (typically sitting in a parameter output port).
  * @param protocolType The type id of the communication protocol for which the output end point is going to be created.
  * @return A dynamically allocated object contained in a std::unique_ptr. The caller is responsible for freeing this object (done
  * automatically if it is kept in a smart pointer).
  * @throw std::out_of_range If no protocol with type id \p protocolType is registered in the factory.
  */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static std::unique_ptr<CommunicationProtocolBase::Output> createOutput( CommunicationProtocolType const & protocolType );

  /**
   * Return the number of registered protocols.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static std::size_t numberOfProtocols() noexcept;

  /**
   * Return the registered name of a protocol.
   * @throw std::invalid_argument if no protocol with this name exists.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static CommunicationProtocolType typeFromName( char const * name );

  /**
   * Lookup the protocol type for a name.
   * @throw std::invalid_argument if no protocol with this name has
   * been registered.
   * @todo Consider changing the return type to char const *
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static std::string typeToName( CommunicationProtocolType type );

  /**
   * Query if a protocol with the given id is registered in the factory
   * @return True if the protocol is regitered, false otherwise.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static bool typeExists( CommunicationProtocolType type ) noexcept;

  /**
   * Template method to register a concrete communication protocol in the factory.
   * This method also registers creation methods for the input and output endpoints that are used by createInput() and createOutput().
   * @tparam ConcreteCommunicationProtocol The protocol class type. The full definition must be visible for instantiating this template.
   * @param protocolType The type id used to register the protocol.
   * @param name A name associated with this protocol, used for tuntime lookup of type ids and associated types.
   * @throw std::invalid_argument If a protocol is already registered for this protocol id or if the name is already used for a different type.
   */
  template< class ConcreteCommunicationProtocol >
  static void registerCommunicationProtocol( CommunicationProtocolType const & protocolType, char const * name );

  /**
   * Template method to register a concrete communication protocol in the factory.
   * Information about the protocol type and the name are taken from the  CommunicationProtocolToId and IdToCommunicationProtocol type traits
   * (normally set by the DEFINE_COMMUNICATION_PROTOCOL macro). These specialisations must be visible at the point where the template method is called.
   * This method also registers creation methods for the input and output endpoints that are used by createInput() and createOutput().
   * @tparam ConcreteCommunicationProtocol The protocol class type. The full definition must be visible for instantiating this template.
   * @throw std::invalid_argument If a protocol is already registered for this protocol id or if the name is already used for a different type.
   */
  template< class ConcreteCommunicationProtocol >
  static void registerCommunicationProtocol();

  template< class ConcreteCommunicationProtocol >
  static void unregisterCommunicationProtocol();

private:
  /**
   * Internal data structure to hold creation functions as well as the name of a particular protocol.
   */
  struct VISR_CORE_LIBRARY_SYMBOL Creator
  {
    /**
     * Function pointer type to create communication protocols.
     */
    using CreateFunction = std::function< std::unique_ptr<CommunicationProtocolBase>( ParameterType const &, ParameterConfigBase const & ) >;

    /**
     * Function pointer type to create input endpoint of a communication protocols.
     */
    using InputCreateFunction = std::function< std::unique_ptr<CommunicationProtocolBase::Input>() >;

    /**
     * Function pointer type to create output endpoint of a communication protocols.
     */
    using OutputCreateFunction = std::function< std::unique_ptr<CommunicationProtocolBase::Output>() >;

    /**
     * Constructor.
     * @param fcn Creation function for a communication protocol
     * @param inputCreator Creation function for a communication protocol input endpoint
     * @param outputCreator Creation function for a communication protocol output endpoint
     * @param name The name under which the communication protocol is to be registered.
     */
    /*VISR_CORE_LIBRARY_SYMBOL*/ explicit Creator( CreateFunction fcn,
                                                   InputCreateFunction inputCreator,
                                                   OutputCreateFunction outputCreator,
                                                   char const * name );

    ~Creator();

    /**
     * Creation function for a communication protocol.
     * @see CommunicationProtocolFactory::createProtocol
     */
    /*VISR_CORE_LIBRARY_SYMBOL*/ std::unique_ptr<CommunicationProtocolBase> createProtocol( ParameterType const & paramType, ParameterConfigBase const & config ) const;
    /*VISR_CORE_LIBRARY_SYMBOL*/ std::unique_ptr<CommunicationProtocolBase::Input> createInput() const;
    /*VISR_CORE_LIBRARY_SYMBOL*/ std::unique_ptr<CommunicationProtocolBase::Output> createOutput() const;

    /*VISR_CORE_LIBRARY_SYMBOL*/ std::string const & name() const;
 private:

    CreateFunction mCreateFunction;
    InputCreateFunction mInputCreateFunction;
    OutputCreateFunction mOutputCreateFunction;
    std::string mName;
  };

  /**
   * Internal registration function.
   * @throw std::invalid_argument If a creator is already registered for the given protocol type \p type or if the \p name 
   * is already used for a different protocol.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ static void registerCommunicationProtocol( CommunicationProtocolType type, Creator&& creator );

  static void unregisterCommunicationProtocol(CommunicationProtocolType type );

  template< class ConcreteCommunicationProtocolType >
  class TCreator: public Creator
  {
  public:
    TCreator( char const * name)
      : Creator( &TCreator<ConcreteCommunicationProtocolType>::construct,
                 &TCreator<ConcreteCommunicationProtocolType>::constructInput,
                 &TCreator<ConcreteCommunicationProtocolType>::constructOutput,
                 name )
    {
    }

    static std::unique_ptr<CommunicationProtocolBase> construct( ParameterType const & paramType,
                                                                 ParameterConfigBase const & config )
    {
      return std::unique_ptr<CommunicationProtocolBase>( new ConcreteCommunicationProtocolType(paramType, config ));
    }

    static std::unique_ptr<CommunicationProtocolBase::Input> constructInput()
    {
      return std::unique_ptr<CommunicationProtocolBase::Input>( new typename ConcreteCommunicationProtocolType::InputBase() );
    }
    static std::unique_ptr<typename CommunicationProtocolBase::Output> constructOutput()
    {
      return std::unique_ptr<CommunicationProtocolBase::Output>( new typename ConcreteCommunicationProtocolType::OutputBase() );
    }
  };

  using KeyType = CommunicationProtocolType;

  using CreatorTable = std::map< KeyType, Creator >;

  static CreatorTable & creatorTable();
};

template< class ConcreteCommunicationProtocolType >
void CommunicationProtocolFactory::registerCommunicationProtocol( CommunicationProtocolType const & protocolType, char const * name )
{
  registerCommunicationProtocol( protocolType, TCreator<ConcreteCommunicationProtocolType>(name) );
}

template< class ConcreteCommunicationProtocolType >
void CommunicationProtocolFactory::registerCommunicationProtocol()
{
  registerCommunicationProtocol<ConcreteCommunicationProtocolType>( 
    CommunicationProtocolToId<ConcreteCommunicationProtocolType>::id, 
    CommunicationProtocolToId<ConcreteCommunicationProtocolType>::name ) ;
}

template< class ConcreteCommunicationProtocolType >
void CommunicationProtocolFactory::unregisterCommunicationProtocol()
{
  unregisterCommunicationProtocol(ConcreteCommunicationProtocolType::staticType() );
}


/**
 * Helper template class to register and unregister protocol types
 * following the RAII idiom.
 * By creating a variable of a specific template type, an arbitrary number of protocol types can be registered.
 * They are automatically unregistered when the lifetime of the variable ends.
 * To this end, the template accepts an arbitary number of protocol types as template arguments.
 *
 * Example usage (using a static variable to tie the lifetime of the object to the lifetime of the application or shared library):
 *
 * @code
 *
 * static visr::CommunicationProtocolRegistrar< ProtocolType1, ProtocolType2, ProtocolType3 > sProtocolRegistrar;
 *
 * @endcode
 *
 */
 //@{
 /**
  * Base case of the recursive variadic template.
  * An empty registrar object (with no associated protocol types)
  */
template< class ... Protocol >
class CommunicationProtocolRegistrar
{
public:
  /**
   * Constructor (trival).
   */
  CommunicationProtocolRegistrar() = default;
  /**
   * Destructor (trival)
   */
  ~CommunicationProtocolRegistrar() {}
};

/**
 * Variadic template, accepting one or more parameter types as template protocols.
 */
template< class Protocol, class ... Protocols >
class CommunicationProtocolRegistrar< Protocol, Protocols ...> : public CommunicationProtocolRegistrar< Protocols ... >
{
public:
  /**
   * Constructor, registers the first element of the template argument list and passes the remaining elements
   * recursively to a another CommunicationProtocolRegistrar template, which forms the base class of the this class.
   */
  CommunicationProtocolRegistrar()
    : CommunicationProtocolRegistrar<Protocols...>()
  {
    visr::CommunicationProtocolFactory::registerCommunicationProtocol<Protocol>();
  }

  /**
   * Destructors, performs unregistration of the first parameter type in the
   * template argument list, while the remaining elements are unregistered
   * recursively.
   */
  ~CommunicationProtocolRegistrar()
  {
    visr::CommunicationProtocolFactory::unregisterCommunicationProtocol<Protocol>();
  }
};
//@}

} // namespace visr

#endif // #ifndef VISR_COMMUNICATION_PROTOCOL_FACTORY_HPP_INCLUDED
