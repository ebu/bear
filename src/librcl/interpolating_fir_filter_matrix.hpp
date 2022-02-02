/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_INTERPOLATING_FIR_FILTER_MATRIX_HPP_INCLUDED
#define VISR_LIBRCL_INTERPOLATING_FIR_FILTER_MATRIX_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/parameter_input.hpp>

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>

#include <libpml/indexed_value_parameter.hpp>
#include <libpml/interpolation_parameter.hpp>
#include <libpml/message_queue_protocol.hpp>

#include <librbbl/filter_routing.hpp>
#include <librbbl/interpolation_parameter.hpp>

#include <cstddef> // for std::size_t
#include <memory>

namespace visr
{
// forward declaration
namespace rbbl
{
template< typename SampleType >
class InterpolatingConvolverUniform;
}

namespace rcl
{
/**
 * A multiple input/multiple output FIR filter
 * This class has one input port named "in" and one output port named "out".
 * The widths of the input and the output port are set by the parameters \p
 * numberOfInputs and \p numberOfOutputs in the setup() method.
 */
class VISR_RCL_LIBRARY_SYMBOL InterpolatingFirFilterMatrix
 : public AtomicComponent
{
  using SampleType = visr::SampleType;

public:
  /**
   * Enumeration to select the instantiated control ports.
   */
  enum class ControlPortConfig
  {
    None = 0,                ///< No control inputs
    Filters = 1 << 0,        ///< Filter control input active
    Routings = 1 << 1,       ///< Routing control input active
    Interpolants = 1 << 2,   ///< Interpolation weight control input activated
    All = Filters | Routings ///< All control inputs active
  };

  /**
   * Constructor.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing
   * composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p
   * nullptr in case of a top-level component
   * @param numberOfInputs The number of input signals.
   * the multichannel input and output waveforms. .
   * @param numberOfOutputs The number of output signals.
   * @param filterLength The maximum admissible FIR filter length.
   * @param maxFilters The maximum number of filter specifications (single FIR
   * filters) that can be stored within the component.
   * @param maxRoutings The maximum number of routings (i.e., the maximum number
   * of simultaneously convolved signal/filter combinations supported by this
   * filter.
   * @param numberOfInterpolants The number of filters used in the calculation
   * of any given filter for convolution.
   * @param transitionSamples The duration of a crossfade (in samples)
   * @param filters The initial FIR filters stored as matrix rows. The filters
   * 0...filters.numberOfRows() are initialised to these FIR filters, the
   * remaining filter specifications are set to zero. Default: empty matrix,
   * i.e., all filters are zero-initialised.
   * @param initialInterpolants List of initial interpolation parameters, i.e.,
   * a set of interpolation weights per routing point.
   * @param routings Initial set of filter routings. Default value: empty
   * routing list, i.e., no signal routings are active initially.
   * @param controlInputs Enumeration to select which parameter update ports are
   * instantiated. Default: ControlPortConfig::None
   * @param fftImplementation name of the FFt library to be used. See
   * rbbl::FftWrapperFactory for available names. Optional parameter, default is
   * "default", i.e., the default FFt library for the platform.
   */
  explicit InterpolatingFirFilterMatrix(
      SignalFlowContext const & context,
      char const * name,
      CompositeComponent * parent,
      std::size_t numberOfInputs,
      std::size_t numberOfOutputs,
      std::size_t filterLength,
      std::size_t maxFilters,
      std::size_t maxRoutings,
      std::size_t numberOfInterpolants,
      std::size_t transitionSamples,
      efl::BasicMatrix< SampleType > const & filters =
          efl::BasicMatrix< SampleType >(),
      rbbl::InterpolationParameterSet const & initialInterpolants =
          rbbl::InterpolationParameterSet(),
      rbbl::FilterRoutingList const & routings = rbbl::FilterRoutingList(),
      ControlPortConfig controlInputs = ControlPortConfig::None,
      char const * fftImplementation = "default" );

  /**
   * Desctructor
   */
  ~InterpolatingFirFilterMatrix();

  /**
   * The process method performs the multichannel convolution.
   */
  void process();

  /**
   * Clear all routings points.
   * The filters remain initialised.
   */
  void clearRoutings();

  /**
   * Add a routing point in the filter matrix.
   * If there is already an entry for the combination (\p inputIdx, \p outputIdx
   * ), the existing rntry is replaced.
   * @param inputIdx The input signal index for the routing.
   * @param outputIdx The output signal index for this routing.
   * @param filterIdx The filter index for this routing (pointing to an entry to
   * the filter container of this component)
   * @param gain An optional, frequency-independent gain for this routing.
   * Optional argument, defaults to 1.0.
   * @throw std::invalid_argument If \p inputIdx, \p outputIdx, or \p filterIdx
   * exceed their respective admissible ranges
   */
  void addRouting( std::size_t inputIdx,
                   std::size_t outputIdx,
                   std::size_t filterIdx,
                   SampleType const gain = static_cast< SampleType >( 1.0 ) );

  /**
   * Add a single filter routing.
   */
  void addRouting( rbbl::FilterRouting const & routing );

  /**
   * Add a set of filter routings.
   */
  void addRoutings( rbbl::FilterRoutingList const & routings );

  /**
   * Set a sequence of filter routings, clear all existing routings beforehand.
   */
  void setRoutings( rbbl::FilterRoutingList const & routings );

  /**
   * Remove a single filter routing.
   * @return True if the specified routing was found, false otherwise.
   */
  bool removeRouting( std::size_t inputIdx, std::size_t outputIdx );

  /**
   * Set all stored filters to zero.
   */
  void clearFilters();

  /**
   * Set a single filter from the provided impulse response.
   * @param filterIdx The index of the filter entry to be set.
   * @param impulseResponse The impulse response.
   * @param filterLength The length of the impulse response.
   * @param alignment The alignment of \p impulseResponse, defaults to 0 (no
   * alignment guarantee).
   * @throw std::out_of_range If \p filterIdx exceeds the number of filter
   * entries of the component
   * @throw std::invalid_argument If the filter length exceeds the maximum
   * length set for the component.
   */
  void setFilter( std::size_t filterIdx,
                  SampleType const * const impulseResponse,
                  std::size_t filterLength,
                  std::size_t alignment = 0 );

  /**
   * Set the complete set of filters.
   * If the number of rows in \p filterSet is smaller than the number of filter
   * entries in the component, the remaining filters are zeroed.
   * @throw std::invalid_argument If the number of rows in \p filterSet exceeds
   * the number of filter entries in the component.
   * @throw std::invalid_argument If the length of the filters (number of
   * columns) exceeds the maximum filter length set for the component.
   */
  void setFilters( efl::BasicMatrix< SampleType > const & filterSet );

  /**
   * Set an interpolation parameter, i.e., a set of interpolation indices and
   * weights for one filter routing.
   * @param interpolant New interpolation parameter, including routing id,
   * indices of the filters to be interpolated, and interpolation weights.
   * @param startTransition Whether to start a new crossfade transition. If
   * false, the new interpolant is set immediately, or a currently active
   * crossfade is continued with the new target value,
   */
  void setInterpolant( rbbl::InterpolationParameter const & interpolant,
                       bool startTransition );

  /**
   * Set the interpolant indices and weigths for a filter routing.
   * @param id Index of the filter routing
   * @param indices The filter indices of the interpolants. Must match the
   * number of interpolants per routing entry set in the constructor.
   * @param weights The interpolation weights for superimposing the interpolant
   * filters. Must match the number of interpolants per routing entry set in the
   * constructor.
   * @param startTransition Whether to start a new crossfade transition. If
   * false, the new interpolant is set immediately, or a currently active
   * crossfade is continued with the new target value,
   */
  void setInterpolant( std::size_t id,
                       std::vector< std::size_t > const & indices,
                       std::vector< float > const & weights,
                       bool startTransition );

  /**
   * Set the interpolant indices and weigths for a filter routing using
   * initialiser lists.
   * @param id Index of the filter routing
   * @param indices The filter indices of the interpolants. Must match the
   * number of interpolants per routing entry set in the constructor.
   * @param weights The interpolation weights for superimposing the interpolant
   * filters. Must match the number of interpolants per routing entry set in the
   * constructor.
   * @param startTransition Whether to start a new crossfade transition. If
   * false, the new interpolant is set immediately, or a currently active
   * crossfade is continued with the new target value,
   */
  void setInterpolant( std::size_t id,
                       std::initializer_list< std::size_t > const & indices,
                       std::initializer_list< float > const & weights,
                       bool startTransition );

private:
  /**
   * The audio input port for this component.
   */
  AudioInput mInput;

  /**
   * The audio output port for this component.
   */
  AudioOutput mOutput;

  std::unique_ptr< ParameterInput<
      pml::MessageQueueProtocol,
      pml::IndexedValueParameter< std::size_t, std::vector< SampleType > > > >
      mSetFilterInput;

  std::unique_ptr<
      ParameterInput< pml::MessageQueueProtocol, pml::InterpolationParameter > >
      mInterpolantInput;

  std::unique_ptr< rbbl::InterpolatingConvolverUniform< SampleType > >
      mConvolver;
};

/**
 * Bitwise operator to combine control input flags.
 */
VISR_RCL_LIBRARY_SYMBOL InterpolatingFirFilterMatrix::ControlPortConfig
operator|( InterpolatingFirFilterMatrix::ControlPortConfig lhs,
           InterpolatingFirFilterMatrix::ControlPortConfig rhs );

/**
 * Bitwise operator to extract mask control input flags.
 */
VISR_RCL_LIBRARY_SYMBOL InterpolatingFirFilterMatrix::ControlPortConfig
operator&( InterpolatingFirFilterMatrix::ControlPortConfig lhs,
           InterpolatingFirFilterMatrix::ControlPortConfig rhs );

} // namespace rcl
} // namespace visr

#endif // #ifndef VISR_LIBRCL_INTERPOLATING_FIR_FILTER_MATRIX_HPP_INCLUDED
