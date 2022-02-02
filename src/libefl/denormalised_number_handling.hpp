/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_DENORMALISED_NUMBER_HANDLING_HPP_INCLUDED
#define VISR_LIBEFL_DENORMALISED_NUMBER_HANDLING_HPP_INCLUDED

#include "export_symbols.hpp"

#include <cstdint>
#include <ciso646>

namespace visr
{
namespace efl
{

/**
 * Structure to collect the several types and functions for setting and storing the 
 * handling of denormalised numbers in the processor.
 * Denormalised numbers are floating-=point numbers very close to zero.
 * On processor architectures, floating-point instructions operating on denormalised numbers 
 * are orders of magnitudes slower than for other values.
 * For audio signal processing, accurate handling of denormalised numbers is not sensible in almost all cases,
 * and they can be treated as zero without negative effects.
 * @note This could be also implemented as a namespace (just remove the 'static')
 */
struct VISR_EFL_LIBRARY_SYMBOL DenormalisedNumbers
{
  /**
   * State to abstract from the internal data types for setting, 
   * storing and restoring the denormalised number handling of the processor.
   */
  struct State
  {
    State()
     : flushToZero( false ), denormalsAreZero( false )
    {
    }

    explicit State( bool pFlushToZero, bool pDenormsAreZero )
     : flushToZero( pFlushToZero ), denormalsAreZero( pDenormsAreZero )
    {
    }

    /** Flush to Zero flag, means that denormalised numbers occuring as the result of an
     * arithmetic instruction are automatically converted to zero.
     */
    bool flushToZero;

    /**
     */
    bool denormalsAreZero;
  };

  /**
   * Set the processors into a mode suitable for realtime audio signal processing.
   * This means that denormalised numbers should not incur a performance penalty.
   * @return The previous state of the processor.
   */
  static State setDenormHandling( );

  /**
   * Reset the handling of denormalised numbers into a previous state.
   * @param stateToRestore The denorm handling mode which should be restord.
   */
  static void resetDenormHandling( State stateToRestore );

  /**
   * Set the denormalised numbers handling into a user-defined mode.
   * This is the most flexible interface which als provides the implementation for the 
   * simpler interfaces above.
   * @param newState The denorm handling mode to be set.
   * @return The state before the new state is set.
   */
  static State setDenormHandling( State newState );
};

} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_DENORMALISED_NUMBER_HANDLING_HPP_INCLUDED
