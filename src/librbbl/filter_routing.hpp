/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_RBBL_FILTER_ROUTING_PARAMETER_HPP_INCLUDED
#define VISR_RBBL_FILTER_ROUTING_PARAMETER_HPP_INCLUDED

#include "export_symbols.hpp"

#include <climits>
#include <iosfwd>
#include <initializer_list>
#include <set>
#include <string>

namespace visr
{
namespace rbbl
{

/**
 * Class for defining a routing entry for a multichannel filter engine.
 * A routing entry consists of a input channel index, an output channel index, an index for the used filter coefficient, and a gain value.
 */
struct VISR_RBBL_LIBRARY_SYMBOL FilterRouting
{
  friend class FilterRoutingList;
public:
  using IndexType = std::size_t;
  using GainType = double; //for the moment, use a fixed type for gains instead of making it a template parameter

  /**
   * Default constructor to create an object with invalid input, outpur, and filter indices.
   */
  FilterRouting()
    : inputIndex( cInvalidIndex )
    , outputIndex( cInvalidIndex )
    , filterIndex( cInvalidIndex )
    , gainLinear( 0.0 )
  {}

  /**
   * Constructor with arguments.
   * @param pInput Input index (zero-offset)
   * @param pOutput Output index (zero-offset)
   * @param pFilter Filter index (zero-offset)
   * @param pGain Initial gain value, linear scale. Default: 1.0
   */
  FilterRouting( IndexType pInput, IndexType pOutput, IndexType pFilter, GainType pGain = 1.0 )
    : inputIndex( pInput )
    , outputIndex( pOutput )
    , filterIndex( pFilter )
    , gainLinear( pGain )
  {
  }

  /**
   * Special value to denote invalid indices
   * @note: std::numeric_limits<IndexType> would be nicer, but MSVC does not support constexpr yet.
   */
  static const IndexType cInvalidIndex = UINT_MAX;

  /**
   * The input channel index (zero-offset)
   */
  IndexType inputIndex;
  /**
   * The output channel index (zero-offset)
   */
  IndexType outputIndex;
  /**
   * The filter channel index (zero-offset)
   */
  IndexType filterIndex;
  /**
   * The gain value for the routing point (linear scale)
   */
  GainType gainLinear;
};

/**
 * Class to contain and manipulate a list of routing entries.
 */
class VISR_RBBL_LIBRARY_SYMBOL FilterRoutingList
{
public:
  using IndexType = FilterRouting::IndexType;

  /**
  * Default constructor, creates an empty list
  */
  FilterRoutingList( ) {}

  /**
   * Create a routing list from an C++11 initialiser list in a C++ source file.
   * @param entries Initialiser list consisting of a list of FilterRouting structs.
   */
  FilterRoutingList( std::initializer_list<FilterRouting> const & entries );

  /**
   * Copy constructor
   * @param rhs The object to be copied.
   */
  FilterRoutingList( const FilterRoutingList & rhs ) = default;

  /**
   * Construct an object from an JSON initialiser string following the 'named constructor' idiom.
   * @param initString A string containing a JSON array of filter routing points
   * @see parseJson
   */
  static FilterRoutingList const fromJson( std::string const & initString );

  /**
   * Construct an object from a JSON string provided in an istream object.
   * This function represents a named constructor.
   * @param initStream An input stream containing a JSON array with FilterReoutingParameter elements.
   * @see parseJson
   */
  static FilterRoutingList const fromJson( std::istream & initStream );

  /**
   * Function object used for ordering the entries in the internal data structure holding the entries.
   */
  class CompareEntries
  {
  public:
    bool operator()( FilterRouting const & lhs, FilterRouting const & rhs ) const
    {
      if( lhs.inputIndex < rhs.inputIndex )
      {
        return true;
      }
      else if( lhs.inputIndex > rhs.inputIndex )
      {
        return false;
      }
      else // lhs.inputIndex == rhs.inputIndex
      {
        return lhs.outputIndex < rhs.outputIndex;
      }
    }
  };

  /**
   * Data type used for representing routings.
   */
  using RoutingsType = std::set< FilterRouting, CompareEntries >;

  /**
   * Exchange the contents with another FilterRoutingList object.
   * Does not throw exceptions.
   * @param rhs The object to be swapped with.
   */
  void swap( FilterRoutingList& rhs );

  /**
   * Query whether the object contains no routings.
   * @return true if the onbject is empty, false otherwise.
   */
  bool empty() const { return mRoutings.empty(); }

  /**
   * Return the number of contained routings.
   * @return Number of current routings.
   */
  std::size_t size() const { return mRoutings.size(); }

  /**
   * Assign the content from another FilterRoutingList object.
   * @param rhs The object whose contents is copied to this object.
   */
  FilterRoutingList & operator=( FilterRoutingList const & rhs) = default; 

  RoutingsType::const_iterator begin() const { return mRoutings.begin(); }

  RoutingsType::const_iterator end() const { return mRoutings.end(); }

  /**
   * Set a new routing using specified by single parameters.
   * In case an entry already exists for this input-output routing, it is replaced by the new one.
   * @param inputIdx Index of the input channel (zero-offset)
   * @param outputIdx Index of the output channel (zero-offset)
   * @param filterIdx Index of the filter (zero-offset)
   * @param gain Gain value of the routing entry, linear scale. Default: 1.0
   */
  void addRouting( FilterRouting::IndexType inputIdx, 
                   FilterRouting::IndexType outputIdx,
                   FilterRouting::IndexType filterIdx,
                   FilterRouting::GainType gain = 0.0 )
  {
    addRouting( FilterRouting( inputIdx, outputIdx, filterIdx, gain) );
  }

  /**
   * Add a new routing entry. If a routing for this input-output combination 
   * already exists, it is replaced by the new one.
   * @param newEntry The new routing entry.
   * @throw std::logic_error If the insertion fails for any reason.
   */
  void addRouting( FilterRouting const & newEntry );

  /**
   * Remove a routing entry speficied by the input and output indices of \p entry.
   * The filterIndex and gainValue attributes of \p entry are ignored.
   * @param entry The routing entry (spcified by the input and output index) to be removed.
   * @return \p true if the removal was successful, and \p false if no element has been removed.
   */
  bool removeRouting( FilterRouting const & entry );

  /**
  * Remove a routing entry speficied by the input and the output index.
  * @param inputIdx The input index of the routing to be removed.
  * @param outputIdx The output index of the routing to be removed.
  * @return \p true if the removal was successful, and \p false if no element has been removed.
  */
  bool removeRouting( IndexType inputIdx, IndexType outputIdx );

  /**
   * Return a routing entry for a given pair of input and output index.
   * @param inputIdx The input index of the reuested routing entry.
   * @param outputIdx The output index of the reuested routing entry.
   * @return A reference to the requested routing entry, or a reference to an object with invalidated index entries if the specified roputing is not found.
   */
  FilterRouting const & getEntry( IndexType inputIdx, IndexType outputIdx ) const
  {
    static FilterRouting const sInvalidRouting;
    RoutingsType::const_iterator const findIt = mRoutings.find( FilterRouting( inputIdx, outputIdx, FilterRouting::cInvalidIndex ) );
    return findIt == mRoutings.end() ? sInvalidRouting : *findIt;
  }

  /**
   * Parse a JSON string containing a routing specification.
   * The top-level JSON object must be an array of elements in the form
   * { "input": nn, "output": nn, "filter": nn, "gain": x.x }, 
   * whereas the element "gain" is optional with a default value of 1.0.
   * Each of the entries "input", "output", "filter" might be either scalars of init lists according to the 
   * syntax defined by IndexSequence and FloatSequence, respectively. If more than one entry is non-scala, 
   * the other entries must be scalar or have the same size as the former. In this case, the scalar entries 
   * are repeated to form sequences of the same length as the non-scalar ones. A routing entry is generated 
   * for each set of corresponding sequence elements.
   * The previous content is erased if the operation is successful.
   * @param encoded A string containing a JSON message.
   * @throw std::invalid_argument If the parsing fails. In this case, the state prior to the call is retained
   * (strong exception safety)
   */
  void parseJson( std::string const & encoded );

  /**
  * Parse a JSON string containing a routing specification from an input stream.
  * The top-level JSON object must be an array of elements in the form
  * { "input": nn, "output": nn, "filter": nn, "gain": x.x },
  * whereas the element "gain" is optional with a default value of 1.0.
  * Each of the entries "input", "output", "filter" might be either scalars of init lists according to the
  * syntax defined by IndexSequence and FloatSequence, respectively. If more than one entry is non-scala,
  * the other entries must be scalar or have the same size as the former. In this case, the scalar entries
  * are repeated to form sequences of the same length as the non-scalar ones. A routing entry is generated
  * for each set of corresponding sequence elements.
  * The previous content is erased if the operation is successful.
  * @param encoded A string containing a JSON message.
  * @throw std::invalid_argument If the parsing fails. In this case, the state prior to the call is retained
  * (strong exception safety)
  */
  void parseJson( std::istream & encoded );
private:
  /**
   * The data structure containing the routing entries.
   */
  RoutingsType mRoutings;
};

} // namespace rbbl
} // namespace visr

#endif // VISR_RBBL_FILTER_ROUTING_PARAMETER_HPP_INCLUDED
