/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_TIME_FREQUENCY_PARAMETER_HPP_INCLUDED
#define VISR_PML_TIME_FREQUENCY_PARAMETER_HPP_INCLUDED

#include "export_symbols.hpp"
#include "time_frequency_parameter_config.hpp"

#include <libefl/basic_matrix.hpp>

#include <libvisr/parameter_type.hpp>
#include <libvisr/typed_parameter_base.hpp>

#include <complex>
#include <initializer_list>
#include <istream>

namespace visr
{
namespace pml
{

namespace // unnamed
{
  /**
   * Type trait to assign a unique ParameterType value to each concrete TimeFrequencyParameter type.
   */
  template<typename ElementType> struct TimeFrequencyParameterType {};

  template<> struct TimeFrequencyParameterType<float>
  {
    static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "TimeFrequencyParameterFloat" ); }
  };
  template<> struct TimeFrequencyParameterType<double>
  {
    static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "TimeFrequencyParameterDouble" ); }
  };
} // unnamed

/**
 * A type for passing time-frequency data between processing components.
 * The template class is explicitly instantiated for the element types float and double.
 * @tparam ElementType The underlying real-valued data type of the elements of the matrix.
 */
template<typename ElementType >
class VISR_PML_LIBRARY_SYMBOL TimeFrequencyParameter: public TypedParameterBase<TimeFrequencyParameter<ElementType>, TimeFrequencyParameterConfig, TimeFrequencyParameterType<ElementType>::ptype() >
{
public:
  using ComplexType = std::complex< ElementType >;


  /**
   * Static member function to compute the number of unique DFT values for a real-to-complex
   * DFT operation.
   */
  static std::size_t numberOfBinsRealToComplex( std::size_t dftSize )
  {
    return (dftSize + 2 ) / 2;
  }

  /**
   * Default constructor, creates an empty matrix of dimension 0 x 0.
   * @param alignment The alignment of the data, given in in multiples of the element size.
   */
  TimeFrequencyParameter( std::size_t alignment = 0 );

  /**
   * Construct a time-frequency parameter from a generic ParameterConfigBase object.
   * @param config The configuration object, must have the dynamic type TimeFrequencyParameterConfig.
   * @throw std::invalid_argument if the dynamic type of \p config does not match TimeFrequencyParameterConfig.
   */
  explicit TimeFrequencyParameter(ParameterConfigBase const & config);

  /**
   * Construct a tie-frequency parameter from a dedicated TimeFrequencyParameterConfig config object.
   * @param config The configuration object.
   */
  explicit TimeFrequencyParameter(TimeFrequencyParameterConfig const & config);

  /**
   * Construct a time-frequency parameter.
   * @param numberOfDftBins The size of the DFT samples per signal and frame.
   * For real-valued transforms, this number typically differs from the size of the transform,
   * because conjugate symmetric elements are stored just once.
   * @param numberOfChannels The number of audio channels that are simultaneously transmitted 
   * in a time-frequency parameter.
   * @param numberOfFrames The number of separate DFT vectors contained.
   * @param alignment The alignment of the data, given in in multiples of the element size.
   */
  explicit TimeFrequencyParameter( std::size_t numberOfDftBins,
                                   std::size_t numberOfChannels, 
                                   std::size_t numberOfFrames, 
                                   size_t alignment = 0 );

  /**
   * Copy constructor.
   * Basically needed to enable the 'named constructor' functions below.
   * @param rhs The object to be copied.
   */
  TimeFrequencyParameter( TimeFrequencyParameter<ElementType> const & rhs );

  virtual ~TimeFrequencyParameter() override;

  TimeFrequencyParameter& operator=( TimeFrequencyParameter<ElementType> const & rhs );

  /**
   * Change the matrix dimension.
   * The content of the matrix is not kept, but reset to zeros.
   * @param numberOfDftBins Number of DFT samples per channel and frame.
   * @param numberOfChannels Number of parallel audio channels conted in one parameter.
   * @param numberOfFrames The number of DFT coefficient sets in a single TimeFrequencyParameter
   * @throw std::bad_alloc If the creation of the new matrix fails.
   */
  void resize( std::size_t numberOfDftBins, std::size_t numberOfChannels, std::size_t numberOfFrames );

  /**
   * Return the data alignment (in number of complex elements) of the cotained DFT data.
   */
  std::size_t alignment() const { return mData.alignmentElements(); }

  /**
   * Return the number of DFT bins of the time/frequency representation.
   */
  std::size_t numberOfDftBins() const { return mData.numberOfColumns(); }

  /**
   * Return the number of audio channels containe in the time/frequency matrix.
   */
  std::size_t numberOfChannels() const { return mNumberOfChannels; }

  /**
   * Return the number of time/frequency frames in a single TimeFrequencyParameter message.
   */
  std::size_t numberOfFrames() const { return mData.numberOfRows() / mNumberOfChannels; }

  /**
   * Return the stride (in number of complex elements) between successive audio channels within the same frame.
   */
  std::size_t channelStride() const { return mData.stride(); }

  /**
   * Return the stride (counted in number of complex elements) between successive frames within the same
   * TimeFrequencyParameter message.
   */
  std::size_t frameStride() const { return mData.stride() * numberOfChannels(); }

  /**
   * Indexed access to a time/frequency sample.
   * Const version.
   */
  ComplexType const & at( std::size_t frameIdx,
    std::size_t channelIdx, std::size_t dftBinIdx ) const
  {
    return *(mData.row( frameIdx * numberOfChannels() + channelIdx )
      + dftBinIdx);
  }

  /**
   * Indexed access to a time/frequency sample.
   * Non-constant version, returns a writable reference.
   */
  ComplexType & at( std::size_t frameIdx,
    std::size_t channelIdx, std::size_t dftBinIdx )
  {
    return *(mData.row( frameIdx * numberOfChannels() + channelIdx )
      + dftBinIdx);
  }

  /**
   * Return a pointer to the complex data, const version
   */
  ComplexType const *  data() const
  {
    return mData.data();
  }

  /**
   * Return a pointer to the start of the complex data, nonconst version.
   */
  ComplexType * data()
  {
    return mData.data();
  }

  /**
   * Return a pointer to the start of an DFT frame, const version
   */
  ComplexType const * frameSlice( std::size_t frameIdx ) const
  {
    return mData.row( frameIdx * numberOfChannels() );
  }

  /**
   * Return a pointer to the start of an DFT frame, non-const version
   */
  ComplexType * frameSlice( std::size_t frameIdx )
  {
    return mData.row( frameIdx * numberOfChannels() );
  }

  /**
   * Return a pointer to the start of an DFT channel vector, const version
   */
  ComplexType const * channelSlice( std::size_t frameIdx, std::size_t channelIdx ) const
  {
    return mData.row( frameIdx * numberOfChannels() + channelIdx );
  }

  /**
   * Return a pointer to the start of an DFT channel vector, non-const version
   */
  ComplexType * channelSlice( std::size_t frameIdx, std::size_t channelIdx )
  {
    return mData.row( frameIdx * numberOfChannels() + channelIdx );
  }

private:
  efl::BasicMatrix< ComplexType > mData;

  std::size_t mNumberOfChannels;
};

} // namespace pml
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::pml::TimeFrequencyParameter<float>, visr::pml::TimeFrequencyParameter<float>::staticType(), visr::pml::TimeFrequencyParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::TimeFrequencyParameter<double>, visr::pml::TimeFrequencyParameter<double>::staticType(), visr::pml::TimeFrequencyParameterConfig )

#endif // VISR_PML_TIME_FREQUENCY_PARAMETER_HPP_INCLUDED
