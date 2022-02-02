//
//  LoudspeakerArray.h
//
//  Created by Marcos F. Sim�n G�lvez on 02/02/2015.
//  Copyright (c) 2014 ISVR, University of Southampton. All rights reserved.
//


#ifndef VISR_LIBRCL_LISTENER_COMPENSATION_HPP_INCLUDED
#define VISR_LIBRCL_LISTENER_COMPENSATION_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libefl/basic_matrix.hpp>

#include <libvisr/atomic_component.hpp>
#include <libvisr/parameter_input.hpp>
#include <libvisr/parameter_output.hpp>

#include <libpanning/defs.h>
#include <libpanning/XYZ.h>
#include <libpanning/LoudspeakerArray.h>

#include <libefl/basic_vector.hpp>

#include <libpml/double_buffering_protocol.hpp>
#include <libpml/listener_position.hpp>
#include <libpml/vector_parameter.hpp>

#include <memory>

namespace visr
{

namespace rcl
{

class VISR_RCL_LIBRARY_SYMBOL ListenerCompensation: public AtomicComponent
{
public:
  using SampleType = visr::SampleType;
public:
  /**
   * Constructor.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p nullptr in case of a top-level component.
   * @param arrayConfig The loudspeaker configuration.
   */
  explicit ListenerCompensation( SignalFlowContext const & context,
                                 char const * name,
                                 CompositeComponent * parent,
                                 panning::LoudspeakerArray const & arrayConfig );

  /**
   * Disabled (deleted) copy constructor
   */
  ListenerCompensation(ListenerCompensation const &) = delete;

  /**
   * The process function.
   * It takes a listener position as input and calculates a gain vector and a delay vector.
   */
  void process() override;

private:
  std::size_t getNumSpeakers( )  const
  {
    return m_array.getNumSpeakers( );
  }

  int setListenerPosition( Afloat x, Afloat y, Afloat z )
  { //assigning the position of the listener
    m_listenerPos.set( x, y, z );
    return 0;
  }

  /**
   * Internal method to calculate the compensation gains.
   * @param [out] gainComp The result vector for the calculated gains (linear scale). It must have the dimension 'numberOfLoudspeakers'.
   * @return 0 in case of success.
   */
  int calcGainComp( efl::BasicVector<Afloat> & gainComp ); // this function calculates the gain compensation

  /**
  * Internal method to calculate the compensation delays.
  * @param [out] delayComp The result vector for the calculated delays (in seconds). It must have the dimension 'numberOfLoudspeakers'.
  * @return 0 in case of success.
  */
  int calcDelayComp( efl::BasicVector<Afloat> & delayComp ); // this function calculates the delay compensation

  panning::LoudspeakerArray m_array; //passing the address of the loudspeaker array
  panning::XYZ m_listenerPos; //position of the listener
  std::size_t const mNumberOfLoudspeakers;

  ParameterInput<pml::DoubleBufferingProtocol, pml::ListenerPosition > mPositionInput;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::VectorParameter<Afloat> > mGainOutput;
  ParameterOutput<pml::DoubleBufferingProtocol, pml::VectorParameter<Afloat> > mDelayOutput;
};//class Listener Compensation

} // namespace rcl
} // namespace visr


#endif // #ifndef VISR_LIBRCL_LISTENER_COMPENSATION_HPP_INCLUDED
