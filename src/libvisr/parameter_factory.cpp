/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "parameter_factory.hpp"

#include "parameter_base.hpp"
#include "parameter_config_base.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace visr
{

struct CreatorCounter
{
  static std::size_t & instance()
  {
    static std::size_t sInstance = 0;
    return sInstance;
  }

  static std::size_t inc()
  {
    ++CreatorCounter::instance();
    return CreatorCounter::instance();
  }
};

ParameterFactory::Creator::Creator( CreateFunction fcn )
 : mCreateFunction( fcn )
{
  CreatorCounter::inc();
}

ParameterFactory::Creator::Creator( Creator && rhs ) = default;

ParameterFactory::Creator::~Creator()
{

}

std::unique_ptr<ParameterBase >
ParameterFactory::Creator::create( ParameterConfigBase const & config ) const
{
  return std::unique_ptr< ParameterBase >( mCreateFunction( config ) );
}

/*static*/ ParameterFactory::CreatorTable &
ParameterFactory::creatorTable()
{
  static ParameterFactory::CreatorTable sCreatorTable;
  return sCreatorTable;
}

/*static*/ std::unique_ptr<ParameterBase>
ParameterFactory::create(ParameterType const & type, ParameterConfigBase const & config)
{
  CreatorTable::const_iterator findIt
    = creatorTable().find( type );
  if( findIt == creatorTable().end() )
  {
    throw std::invalid_argument( "ParameterFactory: No creator function for requested parameter type " );
  }
  // todo: Need to catch construction errors?
  return std::unique_ptr<ParameterBase>( findIt->second.create( config ) );
}

/*static*/ std::size_t ParameterFactory::numberOfParameterTypes() noexcept
{
  return creatorTable().size();
}


/*static*/ bool
ParameterFactory::typeExists( ParameterType type ) noexcept
{
  return creatorTable().find( type ) != creatorTable().end();
}

} // namespace visr
