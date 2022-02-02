/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "communication_protocol_factory.hpp"

#include "communication_protocol_type.hpp"
#include "detail/compose_message_string.hpp"
#include "parameter_type.hpp"

#include <algorithm>
#include <ciso646>
#include <cstring>
#include <stdexcept>
#include <string>

namespace visr
{

void CommunicationProtocolFactory::registerCommunicationProtocol( CommunicationProtocolType type, Creator && creator )
{
  CreatorTable & table = creatorTable();
  auto const findIt = table.find( type );
  
  if( findIt != table.cend() )
  {
    throw std::invalid_argument( std::string("CommunicationProtocolFactory: Entry for protocol type \"") + findIt->second.name() + "\" already exists." );
  }

  auto const findNameIt = std::find_if( table.begin(), table.end(), [creator]( CreatorTable::value_type const & val ){ return val.second.name() == creator.name(); } );
  if( findNameIt != table.cend() )
  {
    throw std::invalid_argument( std::string( "CommunicationProtocolFactory: Protocol name\"") +  creator.name() +  "\" already used for different protocol type." );
  }


  auto const res = table.insert( std::make_pair( type, creator ) );
  if( not res.second)
  {
    throw std::invalid_argument( "CommunicationProtocolFactory: Error while registering protocol type." );

  }
}

void CommunicationProtocolFactory::unregisterCommunicationProtocol( CommunicationProtocolType type )
{
  CreatorTable & table = creatorTable();
  auto const findIt = table.find(type);

  if (findIt != table.cend())
  {
    table.erase(findIt);
  }
}


CommunicationProtocolFactory::Creator::Creator( CreateFunction fcn, 
                                                InputCreateFunction inputCreator,
                                                OutputCreateFunction outputCreator,
                                                char const * name )
 : mCreateFunction( fcn )
 , mInputCreateFunction( inputCreator )
 , mOutputCreateFunction( outputCreator )
 , mName( name )
{
}

CommunicationProtocolFactory::Creator::~Creator() = default;

std::string const & CommunicationProtocolFactory::Creator::name() const
{
  return mName;
}

std::unique_ptr<CommunicationProtocolBase >
CommunicationProtocolFactory::Creator::createProtocol( ParameterType const & paramType, ParameterConfigBase const & config ) const
{
  return mCreateFunction( paramType, config );
}

std::unique_ptr<CommunicationProtocolBase::Input> 
CommunicationProtocolFactory::Creator::createInput() const
{
  return mInputCreateFunction();
}

std::unique_ptr<CommunicationProtocolBase::Output> 
CommunicationProtocolFactory::Creator::createOutput() const
{
  return mOutputCreateFunction();
}


/*static*/ CommunicationProtocolFactory::CreatorTable &
CommunicationProtocolFactory::creatorTable()
{
  static CommunicationProtocolFactory::CreatorTable sCreatorTable;
  return sCreatorTable;
}

/*static*/ std::unique_ptr<CommunicationProtocolBase>
CommunicationProtocolFactory::createProtocol( CommunicationProtocolType const & protocolType,
                                              ParameterType const & parameterType,
                                              ParameterConfigBase const & config )
{
  CreatorTable::const_iterator findIt
    = creatorTable().find( protocolType );
  if( findIt == creatorTable().end() )
  {
    throw std::out_of_range( "CommunicationProtocolFactory: The requested protocol type has not been registered." );
  }
  // todo: Need to catch construction errors?
  return std::unique_ptr<CommunicationProtocolBase>( findIt->second.createProtocol( parameterType, config ) );
}

/*static*/ std::unique_ptr<CommunicationProtocolBase::Input> 
CommunicationProtocolFactory::createInput( CommunicationProtocolType const & protocolType )
{
  CreatorTable::const_iterator findIt
    = creatorTable().find( protocolType );
  if( findIt == creatorTable().end() )
  {
    throw std::out_of_range( "CommunicationProtocolFactory: The requested protocol type is not registered." );
  }
  // todo: Need to catch construction errors?
  return std::unique_ptr<CommunicationProtocolBase::Input>( findIt->second.createInput() );
}

/*static*/ std::unique_ptr<CommunicationProtocolBase::Output> 
CommunicationProtocolFactory::createOutput( CommunicationProtocolType const & protocolType )
{
  CreatorTable::const_iterator findIt
    = creatorTable().find( protocolType );
  if( findIt == creatorTable().end() )
  {
    throw std::out_of_range( "CommunicationProtocolFactory: The requested protocol type is not registered." );
  }
  // todo: Need to catch construction errors?
  return std::unique_ptr<CommunicationProtocolBase::Output>( findIt->second.createOutput() );
}


/*static*/ std::size_t CommunicationProtocolFactory::numberOfProtocols() noexcept
{
  return creatorTable().size();
}

/*static*/ CommunicationProtocolType
CommunicationProtocolFactory::typeFromName( char const * name )
{
  auto const findIt = std::find_if( creatorTable().begin(), creatorTable().end(),
   [name](CreatorTable::value_type const & entry ){ return std::strcmp(entry.second.name().c_str(), name ) == 0; } );
  if( findIt == creatorTable().end() )
  {
    throw std::out_of_range( "CommunicationProtocolFactory: No protocol with the given name is registered." );
  }
  return findIt->first;
}

/*static*/ std::string 
CommunicationProtocolFactory::typeToName( CommunicationProtocolType type )
{
  auto const findIt = creatorTable().find( type );
  if( findIt == creatorTable().end() )
  {
    throw std::out_of_range( "CommunicationProtocolFactory: Requested protocol type not registered." );
  }
  return findIt->second.name();
}

/*static*/ bool
CommunicationProtocolFactory::typeExists( CommunicationProtocolType type ) noexcept
{
  return creatorTable().find( type ) != creatorTable().end();
}


} // namespace visr
