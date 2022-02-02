/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_SIGNAL_ROUTING_PARAMETER_HPP_INCLUDED
#define VISR_PML_SIGNAL_ROUTING_PARAMETER_HPP_INCLUDED

#include "empty_parameter_config.hpp"
#include "export_symbols.hpp"

#include <libvisr/detail/compile_time_hash_fnv1.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/typed_parameter_base.hpp>

#include <algorithm>
#include <ciso646>
#include <cstdint>
#include <limits>
#include <initializer_list>
#include <stdexcept>
#include <set>
#include <tuple>

namespace visr
{
namespace pml
{

/**
 * A parameter class to represent potentially sparse routings between sets of input and output indices.
 * An output index can be routed to zero or one input index, while an input index can be connected to zer, one, or multiple outputs.
 * @note Not sure whether we should introduce parameters to limit
 */
class VISR_PML_LIBRARY_SYMBOL SignalRoutingParameter: public TypedParameterBase<SignalRoutingParameter, pml::EmptyParameterConfig, detail::compileTimeHashFNV1("SignalRouting") >
{
public:
  using IndexType = std::size_t;

  /**
   * Literal constant to denote invalid (or unspecified) channel indices.
   */
  static constexpr IndexType cInvalidIndex = std::numeric_limits<IndexType>::max();

  /**
   * Structure for a single routing entry
   * @note In order to enable initialisation from a brace-enclosed list, this class is a plain datatype without constructors or private members.
   */
  struct Entry
  {
  public:
    IndexType input;
    IndexType output;
  };

  class CompareEntries
  {
  public:
    bool operator()( Entry const & lhs, Entry const & rhs ) const
    {
      return lhs.output < rhs.output;
    }
  };

  /**
   * Data type used for representing routings.
   */
  using RoutingsType = std::set< Entry, CompareEntries >;


  /**
   * Default constructor, creates an empty list
   */
  SignalRoutingParameter() {}

  explicit SignalRoutingParameter( std::initializer_list<Entry> const & entries );

  SignalRoutingParameter( const SignalRoutingParameter & rhs ) = default;

  /**
   * Constructor using parameter configuration object.
   */
  explicit SignalRoutingParameter( const ParameterConfigBase & rhs );

  virtual ~SignalRoutingParameter() override;

  void swap( SignalRoutingParameter& rhs );

  bool empty() const { return mRoutings.empty(); }

  std::size_t size() const { return mRoutings.size(); }

  SignalRoutingParameter & operator=(SignalRoutingParameter const & rhs) = default;

  RoutingsType::const_iterator begin() const { return mRoutings.begin(); }

  RoutingsType::const_iterator end() const { return mRoutings.end(); }

  void addRouting( IndexType inputIdx, IndexType outputIdx )
  {
    addRouting( Entry{ inputIdx, outputIdx } );
  }

  /**
   * Add a routing entry for the (input, output) pair contained in the entry.
   * An existing routing entry for the output index is deleted.
   */
  void addRouting( Entry const & newEntry );

  /**
   * Remove a routing entry consisting of a input and an output index.
   * @return If a routing for this (input, output) pair existed before, false if not.
   */
  bool removeEntry( Entry const & entry );

  /**
   * Remove a routing for a given output index.
   * @return True if there was a routing for that output, false if no such entry existed.
   */
  bool removeEntry( IndexType outputIdx );

  /**
   * Clear all routing entries.
   */
  void clear();

  Entry const & getEntry( IndexType outputIdx ) const
  {
    static const Entry returnInvalid{ cInvalidIndex, cInvalidIndex };

    RoutingsType::const_iterator const findIt = mRoutings.find( Entry{ cInvalidIndex, outputIdx } );
    return findIt == mRoutings.end() ? returnInvalid : *findIt;
  }

  IndexType getInput( IndexType outputIdx ) const
  {
    RoutingsType::const_iterator const findIt = mRoutings.find( Entry{ cInvalidIndex, outputIdx } );
    return findIt == mRoutings.end() ? cInvalidIndex : findIt->input;
  }

  IndexType getOutput( IndexType inputIdx ) const;

  bool parse( std::string const & encoded );

private:
  RoutingsType mRoutings;
};

} // namespace pml
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::pml::SignalRoutingParameter, visr::pml::SignalRoutingParameter::staticType(), visr::pml::EmptyParameterConfig )

#endif // VISR_PML_SIGNAL_ROUTING_PARAMETER_HPP_INCLUDED
