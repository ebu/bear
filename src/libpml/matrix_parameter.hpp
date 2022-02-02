/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_MATRIX_PARAMETER_HPP_INCLUDED
#define VISR_PML_MATRIX_PARAMETER_HPP_INCLUDED

#include "export_symbols.hpp"
#include "matrix_parameter_config.hpp"

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
template<typename ElementType> struct MatrixParameterType{};

template<> struct MatrixParameterType<float>
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1("FloatMatrix"); }
};

template<> struct MatrixParameterType<double>
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "DoubleMatrix" ); }
};
template<> struct MatrixParameterType<std::complex<float> >
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "ComplexFloatMatrix" ); }
};
template<> struct MatrixParameterType<std::complex<double> >
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "ComplexDoubleMatrix" ); }
};
} // unnamed namespace

/**
 * A type for passing matrixes between processing components.
 * The template class is explicitly instantiated for the element types float and double.
 * @tparam ElementType The data type of the elements of the matrix.
 */
template<typename ElementType >
class VISR_PML_LIBRARY_SYMBOL MatrixParameter: public efl::BasicMatrix<ElementType>,
                                               public TypedParameterBase<MatrixParameter<ElementType>, MatrixParameterConfig, MatrixParameterType<ElementType>::ptype() >
{
public:

  /**
   * Default constructor, creates an empty matrix of dimension 0 x 0.
   * @param alignment The alignment of the data, given in in multiples of the eleement size.
   */
  MatrixParameter( std::size_t alignment = 0 );

  explicit MatrixParameter( ParameterConfigBase const & config );

  explicit MatrixParameter( MatrixParameterConfig const & config );

  /**
   * Construct a parameter matrix with the given dimensions.
   * The matrix is zero-initialised.
   * @param numRows The number of matrix rows.
   * @param numColumns The number of columns.
   * @param alignment The alignment of the data, given in in multiples of the eleement size.
   */
  explicit MatrixParameter( std::size_t numRows, std::size_t numColumns, std::size_t alignment = 0 );

  explicit MatrixParameter( std::size_t numRows, std::size_t numColumns,
                            std::initializer_list<std::initializer_list<ElementType> > const & initMtx,
                            std::size_t alignment = 0 );

  /**
   * Copy constructor.
   * Basically needed to enable the 'named constructor' functions below.
   * @param rhs The object to be copied.
   */
  MatrixParameter( MatrixParameter<ElementType> const & rhs );

  virtual ~MatrixParameter() override;

  MatrixParameter& operator=( MatrixParameter<ElementType> const & rhs );


  /**
   * Named constructors to create and initialise matrices from various representations. 
   */
  //@{
  /**
   */
  static MatrixParameter fromString( std::string const & textMatrix, std::size_t alignment = 0 );

  static MatrixParameter fromStream( std::istream & stream, std::size_t alignment = 0 );

#ifdef VISR_PML_USE_SNDFILE_LIBRARY
  static MatrixParameter fromAudioFile( std::string const & fileName, std::size_t alignment = 0 );
#endif

  static MatrixParameter fromTextFile( std::string const & fileName, std::size_t alignment = 0 );
  //@}

  /**
   * Change the matrix dimension.
   * The content of the matrix is not kept, but reset to zeros.
   * @param numRows The new number of rows.
   * @param numColumns The new number of columns
   * @throw std::bad_alloc If the creation of the new matrix fails.
   */
  void resize( std::size_t numRows, std::size_t numColumns );

private:
};

} // namespace pml
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::pml::MatrixParameter<float>, visr::pml::MatrixParameter<float>::staticType(), visr::pml::MatrixParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::MatrixParameter<double>, visr::pml::MatrixParameter<double>::staticType(), visr::pml::MatrixParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::MatrixParameter<std::complex<float> >, visr::pml::MatrixParameter<std::complex<float> >::staticType(), visr::pml::MatrixParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::MatrixParameter<std::complex<double> >, visr::pml::MatrixParameter<std::complex<double> >::staticType(), visr::pml::MatrixParameterConfig )

#endif // VISR_PML_MATRIX_PARAMETER_HPP_INCLUDED
