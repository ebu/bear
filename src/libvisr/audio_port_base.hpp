/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_AUDIO_PORT_BASE_HPP_INCLUDED
#define VISR_AUDIO_PORT_BASE_HPP_INCLUDED

#include "port_base.hpp" // For the 'Direction' enum
#include "audio_sample_type.hpp"
#include "export_symbols.hpp"

#include <string>

namespace visr
{
// Forward declaration(s)
class Component;

namespace impl
{
class AudioPortBaseImplementation;
}

/**
 * Base class for audio ports.
 * Audio ports can form part of the external interface of components and denote start and end points od audio signal connections.
 * An audio port is characterised by a sample type (fundamental integral and floating-point data type as well as complex floating-point types),
 * the width, that is, the number of elementary audio signals represented by this port.
 */
class VISR_CORE_LIBRARY_SYMBOL AudioPortBase
{
public:
  /**
   * Constructor, construct a base audio port object with width zero. 
   * The width can be set later on during initialization using setWidth().
   * @param name Zero-terminated charracter array containing the name of the port. The port name must be unique within all audio ports of the containing component
   * @param container The component to contain that port.
   * @param sampleType Enumeration value denoting the data type of the samples used by this port.
   * @param direction The direction of the port (either input or output)
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ explicit AudioPortBase( char const * name, Component & container, AudioSampleType::Id sampleType, PortBase::Direction direction );

  /**
  * Constructor, construct a base audio port object with a width parameter.
  * @param name Zero-terminated charracter array containing the name of the port. The port name must be unique within all audio ports of the containing component
  * @param container The component to contain that port.
  * @param sampleType Enumeration value denoting the data type of the samples used by this port.
  * @param direction The direction of the port (either input or output)
  * @param width The width, i.e., the number of separate audio channels. This value can be changed before runtime.
  */
  /*VISR_CORE_LIBRARY_SYMBOL*/ explicit AudioPortBase( char const * name, Component& container, AudioSampleType::Id sampleType, PortBase::Direction direction, std::size_t width );

  /**
   * Deleted copy constructor to prohibit copy construction.
   */
  AudioPortBase( AudioPortBase const & ) = delete;

  /**
   * Deleted move constructor to prohibit moving.
   */
  AudioPortBase( AudioPortBase && ) = delete;
  
  /**
   * Destructor (virtual).
   * Audio ports can possibly be created and held polymorphically, although this is not done in standard components.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ virtual ~AudioPortBase();

  /**
   * Deleted assignment operator to prevent assignment.
   */
  AudioPortBase & operator=( AudioPortBase const & ) = delete;

  /**
   * Deleted move assignment operator to prevent move assignment.
   */
  AudioPortBase & operator=( AudioPortBase && ) = delete;

  /**
   * Set the width (i.e., the number of individual audio signals) contained within this port.
   * The new value overwrites any previously set values and can be called multiple times on a single port.
   * This method must only be called in the initialisation phase of the signal flow, not during runtime.
   * @param newWidth The new width (number of single audio signals).
   * @throw std::logic_error If the method is called during runtime.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ void setWidth( std::size_t newWidth );

  /**
   * Return the number of individual channels held by this port.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::size_t width() const noexcept;

  /**
   * Return the alignment of the channel vectors in bytes.
   * This method can be called at any point of the lifetime of an audio port, and the alignment is guaranteed not to change.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::size_t alignmentBytes() noexcept;

  /**
  * Return the guaranteed alignment of the channel vectors (in multiples of the sample size).
  * This function can be called at any point of the audio port's lifetime and the alignment remains constant througout that lifetime.
  */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::size_t alignmentSamples() noexcept;

  /**
   * Return the number of samples between the start of consecutive audio channels.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::size_t channelStrideSamples() const noexcept;

  /**
   * Return the number of bytesbetween the start of consecutive audio channels.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::size_t channelStrideBytes() const noexcept;

  /**
   * Return an enumeration value denoting the type of the audio samples used by this port.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ AudioSampleType::Id sampleType() const noexcept;

  /**
   * Return the size (in bytes) of the data type provided by this port
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ std::size_t sampleSize() const noexcept;

  /**
   * Return  apointer to the opaque implemenentation object.
   * This method is not to be used by implementation code.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ impl::AudioPortBaseImplementation & implementation();

  /**
  * Return  apointer to the opaque implemenentation object, const version.
  * This method is not to be used by implementation code.
  */
  /*VISR_CORE_LIBRARY_SYMBOL*/ impl::AudioPortBaseImplementation const & implementation() const;

protected:
  /**
   * Return the data pointer to fir first (technically zeroth) channel.
   * The type of this pointer is char and needs to be casted in derived, typed port classes.
   */
  /*VISR_CORE_LIBRARY_SYMBOL*/ void * basePointer();

  /**
  * Return the data pointer to fir first (technically zeroth) channel, costant versiob
  * The type of this pointer is char and needs to be casted in derived, typed port classes.
  */
  /*VISR_CORE_LIBRARY_SYMBOL*/ void const * basePointer() const;

private:
  /**
   * Pointer to the private, opaque implementation object.
   */
  impl::AudioPortBaseImplementation* mImpl;
};

} // namespace visr

#endif // #ifndef VISR_AUDIO_PORT_BASE_HPP_INCLUDED
