/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "scalar_osc_decoder.hpp"

#include <libobjectmodel/object_vector.hpp>
#include <libobjectmodel/object_vector_parser.hpp>

#include <libpml/empty_parameter_config.hpp>

#include <oscpkt.hh>

#include <iostream>
#include <limits>

namespace visr
{
namespace rcl
{

ScalarOscDecoder::ScalarOscDecoder( SignalFlowContext const & context,
                              char const * name,
                              CompositeComponent * parent /*= nullptr*/ )
 : AtomicComponent( context, name, parent )
 , mDatagramInput( "datagramInput", *this, pml::EmptyParameterConfig() )
 , mOscParser( new oscpkt::PacketReader() )
{
}

ScalarOscDecoder::~ScalarOscDecoder()
{
}

void ScalarOscDecoder::setup( char const * dataType )
{
  if( strcmp( dataType, "bool") == 0 )
  {
    mBoolOutput.reset( new ParameterOutput< pml::MessageQueueProtocol, pml::ScalarParameter<bool> >( "dataOut", *this, pml::EmptyParameterConfig() ) );
  }
  else if( strcmp( dataType, "int" ) == 0 )
  {
    mIntOutput.reset( new ParameterOutput< pml::MessageQueueProtocol, pml::ScalarParameter<int> >("dataOut", *this, pml::EmptyParameterConfig()) );
  }
  else if( strcmp( dataType, "uint" ) == 0 )
  {
    mUIntOutput.reset( new ParameterOutput< pml::MessageQueueProtocol, pml::ScalarParameter<unsigned int> >("dataOut", *this, pml::EmptyParameterConfig()) );
  }
  else if( strcmp( dataType, "float" ) == 0 )
  {
    mFloatOutput.reset( new ParameterOutput< pml::MessageQueueProtocol, pml::ScalarParameter<float> >( "dataOut", *this, pml::EmptyParameterConfig() ) );
  }
  else if( strcmp( dataType, "double" ) == 0 )
  {
    mDoubleOutput.reset( new ParameterOutput< pml::MessageQueueProtocol, pml::ScalarParameter<double> >("dataOut", *this, pml::EmptyParameterConfig()) );
  }
  else
  {
    status( StatusMessage::Error, "Unsupported data type string \"", dataType, "\"." );
  }
}

void ScalarOscDecoder::process()
{
  while (not mDatagramInput.empty())
  {
    pml::StringParameter const & msg = mDatagramInput.front();

    try
    {
      mOscParser->init( msg.str(), msg.size() );

      oscpkt::Message* msgPkt;
      while( (msgPkt = mOscParser->popMessage()) != nullptr )
      {
        if( mBoolOutput )
        {
          bool val(false);
          msgPkt->arg().popBool( val );
          mBoolOutput->enqueue( pml::ScalarParameter<bool>(val) );
        }
        if( mIntOutput )
        {
          int val{std::numeric_limits<int>::max()};
          msgPkt->arg().popInt32( val );
          mIntOutput->enqueue( pml::ScalarParameter<int>( val ) );
        }
        if( mUIntOutput )
        {
          int val{ std::numeric_limits<int>::max() };
          msgPkt->arg().popInt32( val );
          if( val < 0 )
          {
            status( StatusMessage::Error, "Encountered negative value while decoding unsigned int." );
            continue;
          }
          mUIntOutput->enqueue( pml::ScalarParameter<unsigned int>( val ) );
        }
        if( mFloatOutput )
        {
          float val{ std::numeric_limits<float>::quiet_NaN() };
          msgPkt->arg().popFloat( val );
          mFloatOutput->enqueue( pml::ScalarParameter<float>( val ) );
        }
        if( mDoubleOutput )
        {
          double val{ std::numeric_limits<double>::quiet_NaN() };
          msgPkt->arg().popDouble( val );
          mDoubleOutput->enqueue( pml::ScalarParameter<double>( val ) );
        }
      }
    }
    catch( std::exception const & ex )
    {
      std::cerr << "SceneDecoder: Error decoding an OSC scene metadata message: " << ex.what() << std::endl;
    }
    mDatagramInput.pop();
  }
}

} // namespace rcl
} // namespace visr
