/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRBBL_LAGRANGE_INTERPOLATOR_HPP_INCLUDED
#define VISR_LIBRBBL_LAGRANGE_INTERPOLATOR_HPP_INCLUDED

#include "export_symbols.hpp"

#include "fractional_delay_base.hpp"

#include <libefl/basic_vector.hpp>
#include <libefl/lagrange_coefficient_calculator.hpp>

#include <cstddef>
#include <vector>

namespace visr
{
namespace rbbl
{

/**
 * Abstract base class for interpolation algorithms on discrete-time sequences.
 */
template <typename SampleType, std::size_t order >
class VISR_RBBL_LIBRARY_SYMBOL LagrangeInterpolator: public FractionalDelayBase<SampleType>
{
public:

  /**
   * Constructor, initialises internal data structures.
   * @param maxNumSamples The maximum number o samples that shall be processed in one call to interpolate().
   * @param alignmentElements The minimum alignment of the internal data structures and for the result 
   * buffer of the interpolate() call.
   */
  explicit LagrangeInterpolator( std::size_t maxNumSamples,
                                 std::size_t alignmentElements = 0);

  virtual ~LagrangeInterpolator();

  SampleType methodDelay() const override;

  virtual void interpolate( SampleType const * basePointer,
                            SampleType * result,
                            std::size_t numSamples,
                            SampleType startDelay, SampleType endDelay,
                            SampleType startGain, SampleType endGain ) override;
private:

  efl::BasicVector<SampleType> mDelays;

  efl::BasicVector<SampleType> mGains;

  efl::LagrangeCoefficientCalculator<SampleType, order, true> const mCoeffCalculator;

  /**
   * The method delay in samples, in samples. Literal constant. 
   * @note order == 0 means nearest sample interpolation    */
  static constexpr SampleType cMethodDelay = static_cast<SampleType>(0.5) * static_cast<SampleType>(order);
};

} // namespace rbbl
} // namespace visr
  
#endif // #ifndef VISR_LIBRBBL_LAGRANGE_INTERPOLATOR_HPP_INCLUDED
