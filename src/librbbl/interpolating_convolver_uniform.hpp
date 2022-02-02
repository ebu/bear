/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRBBL_INTERPOLATING_CONVOLVER_UNIFORM_HPP_INCLUDED
#define VISR_LIBRBBL_INTERPOLATING_CONVOLVER_UNIFORM_HPP_INCLUDED

// #include "core_convolver_uniform.hpp"
#include "crossfading_convolver_uniform.hpp"
#include "export_symbols.hpp"
#include "filter_routing.hpp"
#include "interpolation_parameter.hpp"

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>

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
class VISR_RBBL_LIBRARY_SYMBOL InterpolatingConvolverUniform
{
public:
  using FrequencyDomainType =
      typename CrossfadingConvolverUniform< SampleType >::FrequencyDomainType;

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
   * @param numberOfInterpolants The number of filters used for interpolating a
   * single filter.
   * @param transitionSamples Duration of a filter crossfade in samples.
   * @param initialRoutings The initial set of routing points.
   * @param initialInterpolants List of initial interpolation coefficients to
   * determine the filters for convolution.
   * @param initialFilters The initial set of filter coefficients. The matrix
   * rows represent the distinct filters.
   * @param alignment The alignment (given as a multiple of the sample type
   * size) to be used to allocate all data structure. It also guaranteees the
   * alignment of the input and output samples to the process call.
   * @param fftImplementation A string to determine the FFT wrapper to be used.
   * The default value results in using the default FFT implementation for the
   * given data type.
   */
  explicit InterpolatingConvolverUniform(
      std::size_t numberOfInputs,
      std::size_t numberOfOutputs,
      std::size_t blockLength,
      std::size_t maxFilterLength,
      std::size_t maxRoutingPoints,
      std::size_t maxFilterEntries,
      std::size_t numberOfInterpolants,
      std::size_t transitionSamples,
      FilterRoutingList const & initialRoutings = FilterRoutingList(),
      InterpolationParameterSet const & initialInterpolants =
          InterpolationParameterSet(),
      efl::BasicMatrix< SampleType > const & initialFilters =
          efl::BasicMatrix< SampleType >(),
      std::size_t alignment = 0,
      char const * fftImplementation = "default" );

  /**
   * Destructor.
   */
  ~InterpolatingConvolverUniform();

  std::size_t numberOfInputs() const { return mConvolver.numberOfInputs(); }

  std::size_t numberOfOutputs() const { return mConvolver.numberOfOutputs(); }

  std::size_t blockLength() const { return mConvolver.blockLength(); }

  std::size_t maxNumberOfRoutingPoints() const
  {
    return mConvolver.maxNumberOfRoutingPoints();
  }

  std::size_t maxNumberOfFilterEntries() const
  {
    return mFilters.numberOfRows();
  }

  std::size_t maxFilterLength() const { return mConvolver.maxFilterLength(); }

  std::size_t numberOfRoutingPoints() const
  {
    return mConvolver.numberOfRoutingPoints();
  }

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
   * Manipulation of the contained filter representations.
   */
  //@{
  /**
   * Reset all filters to zero.
   */
  void clearFilters();

  /**
   * Load a new set of impulse responses, resetting all prior loaded filters.
   * If there are fewer filters in the matrix than the maximum admissible
   * number, the remaining filters are set to zero. Likewise, if the length of
   * the new filters is less than the maximum allowed filter length, the
   * remaining elements are set to zero.
   * @param newFilters The matrix of new filters, with each row representing a
   * filter.
   * @throw std::invalid_argument If the number of filters (number of rows)
   * exceeds the maximum admissible number of filters.
   * @throw std::invalid_argumeent If the length of the filters (number of
   * matrix columns) exceeds the maximum admissible length,
   */
  void initFilters( efl::BasicMatrix< SampleType > const & newFilters );

  /**
   * Set on entry in the set of stored filters to a new impulse response.
   * @param ir The new impulse response
   * @param filterLength Length of the new IR in samples.
   * @param filterIdx Filter index in the set of stored filters where the new filter is written to.
   * @param alignment Alignment of the \p ir parameter, in number of elements.
   */
  void setImpulseResponse( SampleType const * ir,
                           std::size_t filterLength,
                           std::size_t filterIdx,
                           std::size_t alignment = 0 );

  /**
   * Handling of interpolants (consisting of filter indices and the
   * corresponding interpolation weights).
   */
  //@{
  /**
   * Return the number of filters involved in one interpolation process.
   */
  std::size_t numberOfInterpolants() const;

  /**
   * Set new interpolation weights for a single filter entry.
   * @param param Interpolation parameter structure, including id.
   * @param startTransition Whether this starts a new crossfading transition,
   * or directly updates the interpolant without starting a new crossfade.
   * If there is a currently unfinished transition, it is continued with the 
   * new target filter.
   */
  void setInterpolant( rbbl::InterpolationParameter const & param,
                       bool startTransition );

  /**
   * Set filter interpolants for a sequence of routing entries
   * @param param Interpolation parameter sets.
   * @param startTransition Whether this starts a new crossfading transition,
   * or directly updates the interpolanta without starting a new crossfade.
   * If there are currently unfinished transitions, they are continued with the 
   * new target filter.
   * @todo Decide whether to clear the existing interpolants.
   */
  void setInterpolants( InterpolationParameterSet const & param,
                        bool startTransition );

  /**
   * Set all interpolation weights and filter indices to zero.
   */
  void clearInterpolants();
  //@}

private:
  CrossfadingConvolverUniform< SampleType > mConvolver;

  /**
   * Database of transformed filter representations.
   * These are not directly used in the convolution, but are the input arguments
   * of the interpolation.
   */
  efl::BasicMatrix<
      typename CoreConvolverUniform< SampleType >::FrequencyDomainType >
      mFilters;

  /**
   * Temporary storage to hold a frequency-domain filter representation during
   * computation.
   */
  efl::BasicVector<
      typename CoreConvolverUniform< SampleType >::FrequencyDomainType >
      mTempFilter;

  std::size_t const mNumberOfInterpolants;
};
} // namespace rbbl
} // namespace visr

#endif // #ifndef VISR_LIBRBBL_INTERPOLATING_CONVOLVER_UNIFORM_HPP_INCLUDED
