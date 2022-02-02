/* Copyright Institute of Sound and Vibration Research - All rights reserved */

/**
* @file libefl/lagrange_coefficient_calculator.hpp
* @author Andreas Franck a.franck@soton.ac.uk
*/

#ifndef VISR_LIBEFL_LAGRANGE_COEFFICIENT_CALCULATOR_HPP_INCLUDED
#define VISR_LIBEFL_LAGRANGE_COEFFICIENT_CALCULATOR_HPP_INCLUDED

#include "export_symbols.hpp"

#include <array>

// #define LAGRANGE_COEFFICIENT_CALCULATOR_GENERATE_STAGE_TABLES

namespace visr
{
namespace efl
{

/**
 * Class template to calculate the parameters of a Lagrange interpolator.
 * @param DataType The numerical type of the sequence samples to be interpolated.
 * @param N The order of Lagrange interpolation
 * @tparam reverseOrder Whether the output coefficients are stored in normal FIR filter order (false) or reversed (true).
 * The latter  enables filtering via a dot product without reversing one of the sequences.
 */
template< typename DataType, std::size_t N, bool reverseCoeffs = false >
class VISR_EFL_LIBRARY_SYMBOL LagrangeCoefficientCalculator
{
public:
  /**
   * Default constructor, initialises the constant coefficients.
   */
  LagrangeCoefficientCalculator();

  /**
   * Calculation function.
   * @param mu The intersample position. Negative values are further into the past (left of the sample), smaller values mean less delay.
   * @param result Buffer to hold the result, must provide space for \p N+1 elements.
   */
  void calculateCoefficients( DataType mu, DataType * result ) const;

private:
  static constexpr std::size_t storageSize() { return internalStorageSize( N + 1 ); }

  static constexpr std::size_t internalStorageSize( std::size_t length )
  {
    return length <= 2 ? length : length + internalStorageSize( length / 2 );
  }

  static constexpr std::size_t numberOfStages()
  {
    return numberOfStagesImpl( N+1 );
  }

  static constexpr std::size_t stageSize( std::size_t stage )
  {
    return stage == 0 ? N+1 : stageSize( stage - 1 ) / 2;
  }

  static constexpr std::size_t stageOffset( std::size_t stage )
  {
    return stage == 0 ? 0 : stageOffset( stage - 1 ) + stageSize( stage - 1 );
  }

  template<std::size_t length>
  void multiplyAndShuffle( DataType * coeffs ) const;

  static DataType offset( std::size_t idx );

  static DataType scaleFactor( std::size_t idx );

  static constexpr std::size_t numberOfStagesImpl( std::size_t length )
  {
    return length <= 1 ? 1 : 1 + numberOfStagesImpl( length / 2 );
  }

  static std::array<DataType, N+1> generateOffsets();

  static std::array<DataType, N+1> generateScaleFactors();

#if LAGRANGE_COEFFICIENT_CALCULATOR_GENERATE_STAGE_TABLES
  static std::array<std::size_t, LagrangeCoefficientCalculator<DataType,N>::numberOfStages()> generateStageSizes();

  static std::array<std::size_t, LagrangeCoefficientCalculator<DataType,N>::numberOfStages()> generateStageOffsets();
#endif
  mutable std::array<DataType, internalStorageSize(N+1)> mInternalCoeffs;

  const std::array<DataType, N+1> cOffsets;

  const std::array<DataType, N+1> cScaleFactors;

#if LAGRANGE_COEFFICIENT_CALCULATOR_GENERATE_STAGE_TABLES
  static const std::array<std::size_t, LagrangeCoefficientCalculator<DataType,N>::numberOfStages()> cStageSizes;

  static const std::array<std::size_t, LagrangeCoefficientCalculator<DataType,N>::numberOfStages()> cStageOffsets;
#endif
};

} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_LAGRANGE_COEFFICIENT_CALCULATOR_HPP_INCLUDED
