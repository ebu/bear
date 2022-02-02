/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_TIME_FREQUENCY_PARAMETER_CONFIG_HPP_INCLUDED
#define VISR_PML_TIME_FREQUENCY_PARAMETER_CONFIG_HPP_INCLUDED

#include "export_symbols.hpp"

#include <libvisr/parameter_config_base.hpp>

#include <initializer_list>
#include <istream>

namespace visr
{
namespace pml
{
/**
 * Parameter configuration for TimeFrequencyParameter messages.
 * This ensures that the components involved in a time frequency domain
 * connection have agreed on compatible parameters.
 */
class VISR_PML_LIBRARY_SYMBOL TimeFrequencyParameterConfig
 : public ParameterConfigBase
{
public:
  /**
   * Constructor, creates an instance of TimeFrequencyParameterConfig.
   * @param numberOfDftBins The number of discrete frequency samples per frame
   * and audio channel. Note that in case of real-valued transforms, this number
   * normally differs from the size of the DFT (i.e., the number of time-domain
   * samples transformed in a block.)
   * @param numberOfChannels The number of separate audio signals contained in
   * the multi-channel time-frequency representation.
   * @param numberOfDftFrames The number of consecutive time-frequency blocks
   * (per audio signal) contained in a single message.
   */
  TimeFrequencyParameterConfig( std::size_t numberOfDftBins,
                                std::size_t numberOfChannels,
                                std::size_t numberOfDftFrames = 1 );

  /**
   * Destructor (virtual)
   */
  virtual ~TimeFrequencyParameterConfig() override;

  /**
   * Clone (virtual constructor) method.
   * Enables the polymorphic duplication of instances.
   */
  std::unique_ptr< ParameterConfigBase > clone() const override;

  /**
   * Polymorphic comparison function.
   * Throws an error if the dynamic type od \p rhs differs from
   * TimeFrequencyParameterConfig
   * Return \p true if the paramters of \p rhs are identical to this instance.
   */
  bool compare( ParameterConfigBase const & rhs ) const override;

  /**
   * Specialised compare function with an TimeFrequencyParameterConfig instance.
   * Called from the polymorphic compare() function if the dynamic type matches.
   * Return \p true if the paramters of \p rhs are identical to this instance.
   */
  bool compare( TimeFrequencyParameterConfig const & rhs ) const;

  /**
   * Return the number of points (frequency-domain values) of the DFT.
   * For real-valued DFTs, this number typically differs from the
   * number of time-domain samples transformed in one block of the DFT, because
   * only a smaller number of DFT samples (e.g., ceil((N+1)/2) is stored due to
   * symmetry of the frequency-domain values..
   */
  std::size_t numberOfDftBins() const { return mNumberOfDftBins; }

  /**
   * Number of separate audio signals in the TimeFrequencyParameter messages.
   */
  std::size_t numberOfChannels() const { return mNumberOfChannels; }

  /**
   * The number of DFT frames contained in each message.
   */
  std::size_t numberOfFrames() const { return mNumberOfFrames; }

private:
  std::size_t const mNumberOfDftBins;

  std::size_t const mNumberOfFrames;

  std::size_t const mNumberOfChannels;
};

} // namespace pml
} // namespace visr

#endif // VISR_PML_TIME_FREQUENCY_PARAMETER_CONFIG_HPP_INCLUDED
