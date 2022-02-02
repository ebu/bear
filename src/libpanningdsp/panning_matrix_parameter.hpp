/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PANNINGDSP_PANNING_MATRIX_PARAMETER_HPP_INCLUDED
#define VISR_PANNINGDSP_PANNING_MATRIX_PARAMETER_HPP_INCLUDED

#include "export_symbols.hpp"
#include "types.hpp"

#include <libefl/basic_matrix.hpp>

#include <libpml/matrix_parameter_config.hpp>

#include <libvisr/constants.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/typed_parameter_base.hpp>

#include <complex>
#include <initializer_list>
#include <istream>
#include <limits>

namespace visr
{
namespace panningdsp
{

static constexpr const ParameterType sPanningMatrixParameterType{
  detail::compileTimeHashFNV1("panningdsp::PanningMatrixParameter") };

/**
 * A type for passing matrixes between processing components.
 * The template class is explicitly instantiated for the element types float and double.
 */
class VISR_PANNINGDSP_LIBRARY_SYMBOL
  PanningMatrixParameter:
   public TypedParameterBase<PanningMatrixParameter, pml::MatrixParameterConfig,
     sPanningMatrixParameterType >
{
public:
  /**
   * Constructor, creates a default-initialised transition parameter structure.
   * That is, all gains are set to zero, and the transition start and duration times are set to infinity,
   * meaning no active transition.
   * @param numberOfObjects The number of audio objects processed in the panning gain mattrix.
   * @param numberOfLoudspeakers Number of loudspeaker gains per object.
   * @param alignment The alignment of the data, given in in multiples of the element size.
   */
  explicit PanningMatrixParameter( std::size_t numberOfObjects, std::size_t numberOfLoudspeakers,
    std::size_t alignment = 0 );

  /**
   * Constructor, initialises the gains and transition start and duration times from matrices and vectors.
   * @param gains The panning gains, linear scale. Dimension is numberOfObjects x numberOfLoudspeakers.
   * @param timeStamps Start time of the transitions relative to the sample clock of the renderer. If the value is set to infinity for a given object, no active transition is trnasmitted for this object.
   * @param transitionTimes Duration of the transiton, in samples. A value of zero denotes an immediate change (jump position).
   */ 
  explicit PanningMatrixParameter(visr::efl::BasicMatrix<SampleType> const & gains,
    visr::efl::AlignedArray<TimeType> const & timeStamps,
    visr::efl::AlignedArray<TimeType> const & transitionTimes );

  explicit PanningMatrixParameter(visr::efl::BasicMatrix<SampleType> const & gains,
    std::initializer_list<TimeType> const & timeStamps,
    std::initializer_list<TimeType> const & transitionTimes );

  explicit PanningMatrixParameter(
    std::initializer_list < std::initializer_list <SampleType > > const & gains,
    std::initializer_list<TimeType> const & timeStamps,
    std::initializer_list<TimeType> const & transitionTimes,
    std::size_t alignment = 0 );

  /**
   * Constructor, create a PanningMatricParameter from an ParameterConfigBase object.
   * @throw std::bad_cast unless \p config is a ParameterConfigBase object.
   */
  explicit PanningMatrixParameter( ParameterConfigBase const & config );

  /**
   * Constructor, create a default-initialised object from a pml::MatrixParameterConfig object.
   * The number of objects is set from \p config.numberOfRows(), and the number of panning loudspeakers is inferred from \p config.numberOfColumns().
   */
  explicit PanningMatrixParameter( pml::MatrixParameterConfig const & config );

  /**
   * Copy constructor.
   * Basically needed to enable 'virtual clone contructors' of the TypedParameterBase base class.
   * @param rhs The object to be copied.
   */
  PanningMatrixParameter( PanningMatrixParameter const & rhs );

  virtual ~PanningMatrixParameter() override;

  /**
   * Assignment operator.
   * Used by the base class TypedParameterBase to implment a polymorpbhic assignment function.
   */
  PanningMatrixParameter& operator=(PanningMatrixParameter const & rhs);


  /**
   * Return the number of audio  objects decribed by this PanningMatrixParameter structure.
   */
  std::size_t numberOfObjects() const; 
  
  /**
   * REturn the number of panning gains per object.
   */
  std::size_t numberOfLoudspeakers() const;

  /**
   * Return the alignment of the gain matrix.
   */
  std::size_t alignmentElements() const;

  /**
   * Return the gain mtrix, const version.
   * Dimension numberOfObjects x numberOfLoudpeakers.
   */
  GainMatrixType const & gains() const;

  /**
   * Return the gain mtrix, nonconst version.
   * Dimension numberOfObjects x numberOfLoudpeakers.
   */
  GainMatrixType & gains();

  /**
   * Return the start time stamps of the transitions, const version
   * Dimension: numberOfObjects
   * A value of cTimeStampInfinity denotes that there is no transition
   * for the object at this index.
   */
  TimeVector const & timeStamps() const;

  /**
   * Return the start time stamps of the transitions.
   * Dimension: numberOfObjects
   * A value of cTimeStampInfinity denotes that there is no transition
   * for the object at this index.
   */
  TimeVector & timeStamps();

  TimeVector const & transitionTimes() const;

  TimeVector & transitionTimes();

private:
  GainMatrixType mGains;

  TimeVector mTimeStamps;

  TimeVector mTransitionTimes;
};

} // namespace panningdsp
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::panningdsp::PanningMatrixParameter, detail::compileTimeHashFNV1("panningdsp::PanningMatrixParameter"), visr::pml::MatrixParameterConfig )

#endif // VISR_PANNINGDSP_PANNING_MATRIX_PARAMETER_HPP_INCLUDED
