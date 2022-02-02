/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRCL_FIR_FILTER_MATRIX_HPP_INCLUDED
#define VISR_LIBRCL_FIR_FILTER_MATRIX_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/atomic_component.hpp>
#include <libvisr/audio_input.hpp>
#include <libvisr/audio_output.hpp>
#include <libvisr/constants.hpp>
#include <libvisr/parameter_input.hpp>

#include <libefl/basic_matrix.hpp>
#include <libefl/basic_vector.hpp>

#include <libpml/double_buffering_protocol.hpp>
#include <libpml/filter_routing_parameter.hpp>
#include <libpml/indexed_value_parameter.hpp>
#include <libpml/message_queue_protocol.hpp>

#include <librbbl/filter_routing.hpp>

#include <cstddef> // for std::size_t
#include <memory>

namespace visr
{
// forward declaration
namespace rbbl
{
template< typename SampleType >
class MultichannelConvolverUniform;
}
  
namespace rcl
{

/**
 * A multiple input/multiple output FIR filter
 * This class has one input port named "in" and one output port named "out".
 * The widths of the input and the output port are set by the parameters \p 
 * numberOfInputs and \p numberOfOutputs in the setup() method.
 */
class VISR_RCL_LIBRARY_SYMBOL FirFilterMatrix: public AtomicComponent
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
    AllRoutings = 1 << 2,    ///< Control input to replace all routings in a single message.  
    All = Filters | Routings | AllRoutings ///< All control inputs active
  };

  /**
   * Constructor.
   * @param context Configuration object containing basic execution parameters.
   * @param name The name of the component. Must be unique within the containing composite component (if there is one).
   * @param parent Pointer to a containing component if there is one. Specify \p nullptr in case of a top-level component
   */
  explicit FirFilterMatrix( SignalFlowContext const & context,
                            char const * name,
                            CompositeComponent * parent = nullptr );

  /**
  * Constructor to create a fully initialised component (no need to call setup() ).
  * @param context Configuration object containing basic execution parameters.
  * @param name The name of the component. Must be unique within the containing composite component (if there is one).
  * @param parent Pointer to a containing component if there is one. Specify \p nullptr in case of a top-level component
  * @param numberOfInputs The number of input signals.
  * @param numberOfOutputs The number of output signals.
  * @param filterLength The maximum admissible FIR filter length.
  * @param maxFilters The maximum number of filter specifications (single FIR filters) that can be stored within
  * the component.
  * @param maxRoutings The maximum number of routings (i.e., the maximum number of simultaneously convolved
  * signal/filter combinations supported by this filter.
  * @param filters The initial FIR filters stored as matrix rows. The filters 0...filters.numberOfRows() are initialised
  * to these FIR filters, the remaining filter specifications are set to zero. Default: empty matrix, i.e.,
  * all filters are zero-initialised.
  * @param routings Initial set of filter routings. Default value: empty routing list, i.e., no signal routings are
  * active initially.
  * @param controlInputs Enumeration to select which parameter update ports are instantiated. Default: ControlPortConfig::None
  * @param fftImplementation name of the FFt library to be used. See rbbl::FftWrapperFactory for available names.
  * Optional parameter, default is "default", i.e., the default FFt library for the platform.
  */
  explicit FirFilterMatrix( SignalFlowContext const & context,
                            char const * name,
                            CompositeComponent * parent,
                            std::size_t numberOfInputs,
                            std::size_t numberOfOutputs,
                            std::size_t filterLength,
                            std::size_t maxFilters,
                            std::size_t maxRoutings,
                            efl::BasicMatrix<SampleType> const & filters = efl::BasicMatrix<SampleType>(),
                            rbbl::FilterRoutingList const & routings = rbbl::FilterRoutingList(),
                            ControlPortConfig controlInputs = ControlPortConfig::None,
                            char const * fftImplementation = "default" );

  /**
   * Desctructor
   */
  ~FirFilterMatrix();

  /**
   * The process method performs the multichannel convolution.
   */
  void process( );

  /**
   * Clear all routings points.
   * The filters remain initialised.
   */
  void clearRoutings();

  /**
   * Add a routing point in the filter matrix.
   * If there is already an entry for the combination (\p inputIdx, \p outputIdx ), the existing rntry is replaced.
   * @param inputIdx The input signal index for the routing.
   * @param outputIdx The output signal index for this routing.
   * @param filterIdx The filter index for this routing (pointing to an entry to the filter container of this component)
   * @param gain An optional, frequency-independent gain for this routing. Optional argument, defaults to 1.0.
   * @throw std::invalid_argument If \p inputIdx, \p outputIdx, or \p filterIdx exceed their respective admissible ranges
   */
  void addRouting( std::size_t inputIdx, std::size_t outputIdx, std::size_t filterIdx,
                   SampleType const gain = static_cast<SampleType>(1.0) );

  void addRouting( rbbl::FilterRouting const & routing );

  void addRoutings( rbbl::FilterRoutingList const & routings );

  void setRoutings( rbbl::FilterRoutingList const & routings );

  bool removeRouting( std::size_t inputIdx, std::size_t outputIdx );

  void clearFilters( );

  void setFilter( std::size_t filterIdx, SampleType const * const impulseResponse, std::size_t filterLength, std::size_t alignment =0 );

  void setFilters( efl::BasicMatrix<SampleType> const & filterSet );

private:
  /**
   * The audio input port for this component.
   */
  AudioInput mInput;

  /**
   * The audio output port for this component.
   */
  AudioOutput mOutput;

  using FilterInput = ParameterInput<pml::MessageQueueProtocol, pml::IndexedValueParameter< std::size_t, std::vector<SampleType > > >;

  using SingleRoutingInput = ParameterInput<pml::MessageQueueProtocol, pml::FilterRoutingParameter >;

  using AllRoutingsInput = ParameterInput<pml::DoubleBufferingProtocol, pml::FilterRoutingListParameter >;

  std::unique_ptr< FilterInput > mSetFilterInput;

  std::unique_ptr< SingleRoutingInput > mSingleRoutingInput;

  std::unique_ptr< AllRoutingsInput > mAllRoutingsInput;

  std::unique_ptr<rbbl::MultichannelConvolverUniform<SampleType> > mConvolver;
};

/**
 * Bitwise operator to combine control input flags.
 */
VISR_RCL_LIBRARY_SYMBOL FirFilterMatrix::ControlPortConfig operator|( FirFilterMatrix::ControlPortConfig lhs,
                                                                 FirFilterMatrix::ControlPortConfig rhs );

/**
 * Bitwise operator to extract mask control input flags.
 */
VISR_RCL_LIBRARY_SYMBOL FirFilterMatrix::ControlPortConfig operator&( FirFilterMatrix::ControlPortConfig lhs,
                                                                 FirFilterMatrix::ControlPortConfig rhs );

} // namespace rcl
} // namespace visr
  
#endif // #ifndef VISR_LIBRCL_FIR_FILTER_MATRIX_HPP_INCLUDED
