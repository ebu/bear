/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRBBL_CROSSFADING_CONVOLVER_UNIFORM_HPP_INCLUDED
#define VISR_LIBRBBL_CROSSFADING_CONVOLVER_UNIFORM_HPP_INCLUDED

#include "core_convolver_uniform.hpp"
#include "export_symbols.hpp"
#include "filter_routing.hpp"

#include <libefl/basic_matrix.hpp>

#include <cassert>
#include <complex>
#include <initializer_list>
#include <map>
#include <memory>
#include <vector>

namespace visr
{
namespace rbbl
{
/**
 * Generic class for MIMO convolution using a uniformly partioned fast
 * convolution algorithm. It supports a arbitrary numbers of input and output
 * channels and enables filters and an optional gain factor for arbitrary
 * input-output connections.
 * @tparam SampleType The floating-point type of the signal samples
 */
template< typename SampleType >
class VISR_RBBL_LIBRARY_SYMBOL CrossfadingConvolverUniform
{
public:
  using FrequencyDomainType =
      typename CoreConvolverUniform< SampleType >::FrequencyDomainType;

  /**
   * Constructor.
   * @param numberOfInputs The number of input signals processed.
   * @param numberOfOutputs The number of output channels produced.
   * @param blockLength The numbers of samples processed for each input or
   * output channels in one process() call.
   * @param maxFilterLength The maximum length of the FIR filters (in samples).
   * @param maxRoutingPoints The maximum number of routing points between input
   * and output channels, i.e., the number of filter operations to be
   * calculated.
   * @param maxFilterEntries The maximum number of filters that can be stored
   * within the filter. This number can be different from \p maxRoutingPoints,
   * as the convolver can both reuse the same filter representation multiple
   * times, or store multiple filter representations to enable switching of the
   * filter characteristics at runtime.
   * @param transitionSamples The duration of the crossfade (in samples)
   * @param initialRoutings The initial set of routing points.
   * @param initialFilters The initial set of filter coefficients. The matrix
   * rows represent the distinct filters.
   * @param alignment The alignment (given as a multiple of the sample type
   * size) to be used to allocate all data structure. It also guaranteees the
   * alignment of the input and output samples to the process call.
   * @param fftImplementation A string to determine the FFT wrapper to be used.
   * The default value results in using the default FFT implementation for the
   * given data type.
   */
  explicit CrossfadingConvolverUniform(
      std::size_t numberOfInputs,
      std::size_t numberOfOutputs,
      std::size_t blockLength,
      std::size_t maxFilterLength,
      std::size_t maxRoutingPoints,
      std::size_t maxFilterEntries,
      std::size_t transitionSamples,
      FilterRoutingList const & initialRoutings = FilterRoutingList(),
      efl::BasicMatrix< SampleType > const & initialFilters =
          efl::BasicMatrix< SampleType >(),
      std::size_t alignment = 0,
      char const * fftImplementation = "default" );

  /**
   * Destructor.
   */
  ~CrossfadingConvolverUniform();

  std::size_t numberOfInputs() const { return mCoreConvolver.numberOfInputs(); }

  std::size_t numberOfOutputs() const
  {
    return mCoreConvolver.numberOfOutputs();
  }

  std::size_t blockLength() const { return mCoreConvolver.blockLength(); }

  std::size_t dftBlockRepresentationSize() const
  {
    return mCoreConvolver.dftBlockRepresentationSize();
  }

  std::size_t dftFilterRepresentationSize() const
  {
    return mCoreConvolver.dftFilterRepresentationSize();
  }

  std::size_t alignment() const { return mCoreConvolver.alignment(); }

  std::size_t complexAlignment() const
  {
    return mCoreConvolver.complexAlignment();
  }

  std::size_t maxNumberOfRoutingPoints() const
  {
    return mMaxNumberOfRoutingPoints;
  }

  std::size_t maxNumberOfFilterEntries() const
  {
    return mCoreConvolver.maxNumberOfFilterEntries();
  }

  std::size_t maxFilterLength() const
  {
    return mCoreConvolver.maxFilterLength();
  }

  std::size_t numberOfRoutingPoints() const { return mRoutingTable.size(); }

  void process( SampleType const * const input,
                std::size_t inputStride,
                SampleType * const output,
                std::size_t outputStride,
                std::size_t alignment = 0 );

  /**
   * Manipulation of the routing table.
   */
  //@{
  /**
   * Remove all entries from the routing table.
   */
  void clearRoutingTable();

  /**
   * Initialize the routing table from a set of entries.
   * All pre-existing entries are cleared beforehand.
   * @param routings A vector of routing entries
   * @throw std::invalid_argument If the number of new entries exceeds the
   * maximally permitted number of routings
   * @throw std::invalid_argument If any input, output, or filter index exceeds
   * the admissible range for the respective type.
   */
  void initRoutingTable( FilterRoutingList const & routings );

  /**
   * Add a new routing to the routing table.
   * @throw std::invalid_argument If adding the entry would exceed the maximally
   * permitted number of routings
   * @throw std::invalid_argument If any input, output, or filter index exceeds
   * the admissible range for the respective type.
   */
  void setRoutingEntry( FilterRouting const & routing );

  /**
   * Add a new routing to the routing table.
   * @throw std::invalid_argument If adding the entry would exceed the maximally
   * permitted number of routings
   * @throw std::invalid_argument If any input, output, or filter index exceeds
   * the admissible range for the respective type.
   */
  void setRoutingEntry( std::size_t inputIdx,
                        std::size_t outputIdx,
                        std::size_t filterIdx,
                        SampleType gain );

  /**
   * @return \p true if the entry was removes, \p false if not (i.e., the entry
   * did not exist).
   */
  bool removeRoutingEntry( std::size_t inputIdx, std::size_t outputIdx );

  /**
   * Query the currently active number of routings.
   */
  //  std::size_t numberOfRoutings( ) const { return
  //  mCoreConvolver.numberOfRoutings(); }
  //@}

  /**
   * Manipulation of the contained filter representation.
   */
  //@{
  /**
   * Transform an impulse response into the partitioned frequency-domain
   * representation
   * @todo The semantics of the \p alignment parameter would be clearer if it
   * would be in bytes.
   */
  void transformImpulseResponse( SampleType const * ir,
                                 std::size_t irLength,
                                 FrequencyDomainType * result,
                                 std::size_t alignment = 0 ) const;

  /**
   * Reset all contained filters to zero.
   * This also resets the currenly running, interpolated filters, effectively
   * muting the output of the convolution.
   * @note The semantics of this function differs from setFilters /
   * setImpulseResponse / setTransformedFilter, which
   */
  void clearFilters();

  /**
   * Load a new set of impulse responses, resetting all prior loaded filters.
   * If there are fewer filters in the matrix than the maximum admissible
   * number, the remaining filters are set to zero. Likewise, if the length of
   * the new filters is less than the maximum allowed filter length, the
   * remaining elements are set to zero.
   * @param newImpulseResponses The matrix of new filters, with each row
   * representing an impulse response.
   * @throw std::invalid_argument If the number of filters (number of rows)
   * exceeds the maximum admissible number of filters.
   * @throw std::invalid_argumeent If the length of the filters (number of
   * matrix columns) exceeds the maximum admissible length,
   * @note This function does not affect the currently set (interpolated)
   * filters used in the running convolution.
   */
  void initFilters(
      efl::BasicMatrix< SampleType > const & newImpulseResponses );

  /**
   * Set one entry in the set of stored filters to a new impulse response.
   * @param ir The impulse response to be set.
   * @param filterLength of the new impulse response in samples.
   * @param filterIdx Index within the internal filter storage where the filter
   * data is written to.
   * @param startTransition Whether to start a new crossfading transition. If
   * false, the filter set replaced directly without a crossfade, or a currently
   * active crossfade is continued with the new target filter.
   * @param alignment Alignment of the parameter \p ir, in number of elements.
   */
  void setImpulseResponse( SampleType const * ir,
                           std::size_t filterLength,
                           std::size_t filterIdx,
                           bool startTransition,
                           std::size_t alignment );

  /**
   * Set a transformed filter (i.e., in the partitioned, zero-padded
   * representation) to a given filter (routing) slot.
   * @param fdFilter Vector containing complex-valued values of a partitioned
   * convolution filter.
   * @param filterIdx Index within the internal filter storage where the filter
   * data is written to.
   * @param startTransition Whether to start a new crossfading transition. If
   * false, the filter set replaced directly without a crossfade, or a currently
   * active crossfade is continued with the new target filter.
   * @param alignment Alignment of the parameter \p fdFilter, in number of
   * (complex) elements.
   * @note The transformed filter should be obtained using
   * transformImpulseResponse(), because the actual format (block size, padding,
   * etc.) depends on the convolver.
   */
  void setTransformedFilter( FrequencyDomainType const * fdFilter,
                             std::size_t filterIdx,
                             bool startTransition,
                             std::size_t alignment );

private:
  /**
   * Internal function to apply the filters and to set the oputputs.
   */
  void processOutputs( SampleType * const output,
                       std::size_t outputChannelStride,
                       std::size_t alignment );

  CoreConvolverUniform< SampleType > mCoreConvolver;

  struct RoutingEntry
  {
    explicit RoutingEntry( std::size_t in,
                           std::size_t out,
                           std::size_t filter,
                           SampleType gain = 1.0f )
     : inputIdx( in ), outputIdx( out ), filterIdx( filter ), gainLinear( gain )
    {
    }
    std::size_t inputIdx;
    std::size_t outputIdx;
    std::size_t filterIdx;
    SampleType gainLinear;
  };
  /**
   * Function object for ordering the routings within the routing table.
   * By grouping the entries according to the output indices, the ordering is
   * specifically tailored to the execution of the process() function.
   */
  struct CompareRoutings
  {
    bool operator()( RoutingEntry const & lhs, RoutingEntry const & rhs ) const
    {
      if( lhs.outputIdx == rhs.outputIdx )
      {
        return lhs.inputIdx < rhs.inputIdx;
      }
      else
      {
        return lhs.outputIdx < rhs.outputIdx;
      }
    }
  };

  using RoutingTable = std::multiset< RoutingEntry, CompareRoutings >;

  RoutingTable mRoutingTable;

  std::size_t const mMaxNumberOfRoutingPoints;

  /**
   * Dimension: 2 x dftRepresentationSize
   */
  efl::BasicMatrix<
      typename CoreConvolverUniform< SampleType >::FrequencyDomainType >
      mFrequencyDomainOutput;

  efl::BasicMatrix< SampleType > mTimeDomainTempOutput;

  /**
   * Crossfading-related data members
   */
  //@{

  /**
   * Redundant with CoreConvolver information, but used in multiple places as
   * the offset to the second filter bank.
   */
  std::size_t const mMaxNumFilters;

  std::size_t const mNumRampBlocks;

  std::vector< std::size_t > mCurrentFilterOutput;

  std::vector< std::size_t > mCurrentRampBlock;

  efl::BasicMatrix< SampleType > mCrossoverRamps;
  //@}
};
} // namespace rbbl
} // namespace visr

#endif // #ifndef VISR_LIBRBBL_CROSSFADING_CONVOLVER_UNIFORM_HPP_INCLUDED
