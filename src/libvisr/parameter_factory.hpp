/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PARAMETER_FACTORY_HPP_INCLUDED
#define VISR_PARAMETER_FACTORY_HPP_INCLUDED

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
template<class ConcreteType, class ParameterConfig, ParameterType> class TypedParameterBase;

class VISR_CORE_LIBRARY_SYMBOL ParameterFactory
{
public:
  static std::unique_ptr<ParameterBase> create(ParameterType const & type, ParameterConfigBase const & config);

  /**
   * Return the number of registered parameter types.
   */
  static std::size_t numberOfParameterTypes() noexcept;

  /**
   * Query if a parameter type with the given id is registered in the factory
   * @return True if the parameter type is registered, false otherwise.
   */
  static bool typeExists( ParameterType type ) noexcept;

  template< class ConcreteParameterType >
  static void registerParameterType( ParameterType const &  type );

  /**
   * Special registration function for ParameterType subtypes that inherit from
  * TypedParameterBase.
  */
  template< class TypedParameterType >
  static void registerParameterType();


  template< class ConcreteParameterType >
  static void unregisterParameterType(ParameterType const &  type) noexcept;

  template< class ConcreteParameterType >
  static void unregisterParameterType() noexcept;

  /**
   * Template class to register parameter types.
   * Creating a static object invokes the class registration function.
   */
  template< class ConcreteParameterType >
  class Registrar
  {
  public:
    explicit Registrar( ParameterType type )
    {
      creatorTable().insert( std::make_pair( type, TCreator<ConcreteParameterType>() ) );
    }
  };

private:
  struct VISR_CORE_LIBRARY_SYMBOL Creator
  {
    using CreateFunction = std::function< ParameterBase* ( ParameterConfigBase const & config ) >;

    /**
     * Constructor, taking a creation function as parameter.
     */
    explicit Creator( CreateFunction fcn );

    /**
     * Move constructor, used to pass a Creator object into the creator table without copy construction.
     */
    Creator( Creator && rhs );

    /**
     * Deleted copy constructor to prohibit pass-by-value.
     */
    Creator( Creator const & rhs ) = delete;

    ~Creator();

    std::unique_ptr<ParameterBase> create( ParameterConfigBase const & config ) const;
 private:
    CreateFunction mCreateFunction;
  };

  template< class ConcreteParameterType >
  class TCreator: public Creator
  {
  public:
    TCreator( )
      : Creator( &TCreator<ConcreteParameterType>::construct )
    {
    }

    ~TCreator()
    {

    }

    TCreator(TCreator const & rhs) = delete;

    TCreator(TCreator && rhs) = default;

    static ParameterBase* construct( ParameterConfigBase const & config )
    {
      ParameterBase* obj = new ConcreteParameterType( config );
      return obj;
    }
  };

  struct CreatorTable : public std::map<ParameterType, Creator >
  {
    CreatorTable()
    {
    }

    ~CreatorTable()
    {
    }

  };

  static CreatorTable & creatorTable();
};

template< class ConcreteParameterType >
void ParameterFactory::registerParameterType( ParameterType const & type )
{
  creatorTable().insert( std::make_pair( type, TCreator<ConcreteParameterType>() ) );
}

template< class TypedParameterType >
void ParameterFactory::registerParameterType()
{
  registerParameterType<TypedParameterType>(TypedParameterType::staticType());
}

template< class ConcreteParameterType >
void ParameterFactory::unregisterParameterType(ParameterType const & type) noexcept
{
  auto & table = creatorTable();
  auto findIt = table.find( type );
  if (findIt != table.end())
  {
    table.erase( findIt );
  }
}

template< class ConcreteParameterType >
void ParameterFactory::unregisterParameterType() noexcept
{
  unregisterParameterType<ConcreteParameterType>(ConcreteParameterType::staticType());
}

/**
 * @name ParameterRegistrar
 * Helper template class to register and unregister parameter types
 * following the RAII idiom.
 * By creating a variable of a specific template type, an arbitrary number of parameter types can be registered.
 * They are automatically unregistered when the lifetime of the variable ends.
 * To this end, the template accepts an arbitary number of parameter types as template parameters.
 *
 * Example usage (using a static variable to tie the lifetime of the object to the lifetime of the application or shared library):
 * @code
 * static visr::ParameterRegistrar< ParameterType1, ParameterType2 > sParameterRegistrar;
 * @endcode
 */
 ///@{
 /**
  * Base case of the recursive variadic template.
  * An empty registrar object (with no associated parameter types)
  */
template< class ... Parameter >
class ParameterRegistrar
{
public:
  /**
   * Constructor (trival).
   */
  ParameterRegistrar() = default;
  /**
   * Destructor (trival)
   */
  ~ParameterRegistrar() {}
};

/**
 * Variadic template, accepting one or more parameter types as template arguments.
 */
template< class Parameter, class ... Parameters >
class ParameterRegistrar< Parameter, Parameters ...> : public ParameterRegistrar< Parameters ... >
{
public:
  /**
   * Constructor, registers the first element of the template argument list and passes the remaining elements
   * recursively to a another ParameterRegistrar template, which forms the base class of the this class.
   */
  ParameterRegistrar()
    : ParameterRegistrar<Parameters...>()
  {
    visr::ParameterFactory::registerParameterType<Parameter>();
  }

  /**
   * Destructors, performs unregistration of the first parameter type in the
   * template argument list, while the remaining elements are unregistered
   * recursively.
   */
  ~ParameterRegistrar()
  {
    visr::ParameterFactory::unregisterParameterType<Parameter>();
  }
};
///@}


// The macro does not work for multiple uses in the same .cpp file
// (multiple definitions of 'maker'), stringization of names difficult
// because of template brackets and namespace names.
// #define REGISTER_PARAMETER( type, id ) namespace { static ParameterFactory::Registrar< type > maker( id ); }

} // namespace visr

#endif // #ifndef VISR_PARAMETER_FACTORY_HPP_INCLUDED
