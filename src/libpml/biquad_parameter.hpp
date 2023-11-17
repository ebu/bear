/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_BIQUAD_PARAMETER_HPP_INCLUDED
#define VISR_PML_BIQUAD_PARAMETER_HPP_INCLUDED

#include "export_symbols.hpp"

#include "empty_parameter_config.hpp"
#include "matrix_parameter_config.hpp"
#include "vector_parameter_config.hpp"

#include <librbbl/biquad_coefficient.hpp>

#include <libvisr/typed_parameter_base.hpp>
#include <libvisr/parameter_type.hpp>

#include <boost/property_tree/ptree_fwd.hpp>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iosfwd>
#include <stdexcept>
#include <vector>

namespace visr
{
namespace pml
{

namespace // unnamed
{
/**
 * Type trait to assign a unique type to each concrete BiquadCoefficientParameter type.
 */
template<typename ElementType> struct BiquadCoefficientParameterType{};

template<>
struct BiquadCoefficientParameterType< float >
{
  static constexpr const ParameterType ptype()
  {
    return detail::compileTimeHashFNV1( "BiquadCoefficientFloat" );
  }
};
template<>
struct BiquadCoefficientParameterType< double >
{
  static constexpr const ParameterType ptype()
  {
    return detail::compileTimeHashFNV1( "BiquadCoefficientDouble" );
  }
};

/**
 * Type trait to assign a unique type to each concrete
 * BiquadCoefficientListParameter type.
 */
template< typename ElementType >
struct BiquadCoefficientListParameterType
{
};

template<>
struct BiquadCoefficientListParameterType< float >
{
  static constexpr const ParameterType ptype()
  {
    return detail::compileTimeHashFNV1( "BiquadCoefficientListFloat" );
  }
};
template<>
struct BiquadCoefficientListParameterType< double >
{
  static constexpr const ParameterType ptype()
  {
    return detail::compileTimeHashFNV1( "BiquadCoefficientListDouble" );
  }
};


/**
 * Type trait to assign a unique type to each concrete BiquadParameterMatrix
 * type.
 */
template< typename ElementType >
struct BiquadMatrixParameterType
{
};

template<> struct BiquadMatrixParameterType<float>
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "BiquadFloatMatrix" ); }
};
template<> struct BiquadMatrixParameterType<double>
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "BiquadDoubleMatrix" ); }
};
} // unnamed

/**
 * Parameter type encapsulating an rbbl::BiquadCoefficient object, i.e., a
 * single IIR biquad section.
 * Explicitly instantiated for coefficient types float and double.
 * @tparam CoeffType The data type of the filter coefficients, generally a
 * floating-point type.
 */
template< typename CoeffType >
class VISR_PML_LIBRARY_SYMBOL BiquadCoefficientParameter
 : public TypedParameterBase< BiquadCoefficientParameter< CoeffType >,
                              EmptyParameterConfig,
                              BiquadCoefficientParameterType< CoeffType >::ptype() >
 , public rbbl::BiquadCoefficient< CoeffType >
{
public:
  /**
   * Inherit all BiquadCoefficient constructors
   */
  using rbbl::BiquadCoefficient< CoeffType >::BiquadCoefficient;

  /**
   * Default constructor, creates a 'flat' biquad coefficient.
   */
  BiquadCoefficientParameter();

  /**
   * Create a BiquadCoefficientParameter from a raw BiquadCoefficient object.
   */
  explicit BiquadCoefficientParameter(
      rbbl::BiquadCoefficient< CoeffType > const rhs );

  explicit BiquadCoefficientParameter( EmptyParameterConfig const & config );

  explicit BiquadCoefficientParameter( ParameterConfigBase const & config );

  virtual ~BiquadCoefficientParameter() override;
};


/**
 * Parameter type encapsulating an rbbl::BiquadCoefficientList object, i.e., a
 * single IIR filter consisting of multiple biquad sections.
 * Explicitly instantiated for coefficient types float and double.
 * @tparam CoeffType The data type of the filter coefficients, generally a
 * floating-point type.
 */
template< typename CoeffType >
class VISR_PML_LIBRARY_SYMBOL BiquadCoefficientListParameter
 : public TypedParameterBase<
       BiquadCoefficientListParameter< CoeffType >,
       VectorParameterConfig,
       BiquadCoefficientListParameterType< CoeffType >::ptype() >
 , public rbbl::BiquadCoefficientList< CoeffType >
{
public:
  /**
   * Inherit all BiquadCoefficientList constructors
   */
  using rbbl::BiquadCoefficientList< CoeffType >::BiquadCoefficientList;

  /**
   * Constructs a parameter object from a (raw) biquad coefficient list object.
   */
  explicit BiquadCoefficientListParameter(
      rbbl::BiquadCoefficientList< CoeffType > const & base );

  /**
   * Construct a BiquadCoefficientListParameter object from the corresponding
   * parameter configuration object. Creates a coefficient list with the given
   * number of biquads, all set to the 'flat' default value.
   */
  explicit BiquadCoefficientListParameter( VectorParameterConfig const & config );

  /**
   * Constructs a BiquadCoefficientListParameter object from a base ParameterConfig object.
   * @throw std::invalid_argument If the concrete type of @p config differs from
   * the required configuration object type for this class (i.e.,
   * pml::VectorParameterConfig).
   */
  explicit BiquadCoefficientListParameter( ParameterConfigBase const & config );

  /**
   * Destructor (virtual).
   */
  virtual ~BiquadCoefficientListParameter() override;
};


template<typename CoeffType >
class VISR_PML_LIBRARY_SYMBOL BiquadParameterMatrix
 : public TypedParameterBase< BiquadParameterMatrix< CoeffType >,
                              MatrixParameterConfig,
                              BiquadMatrixParameterType< CoeffType >::ptype() >
 ,
  public rbbl::BiquadCoefficientMatrix<CoeffType>
{
public:
  /**
   * Inherit all BiquadCoefficientMatrix constructors
   */
  using rbbl::BiquadCoefficientMatrix<CoeffType>::BiquadCoefficientMatrix;

  explicit BiquadParameterMatrix(
      rbbl::BiquadCoefficientMatrix< CoeffType > const & base );

  explicit BiquadParameterMatrix( MatrixParameterConfig const & config );

  explicit BiquadParameterMatrix( ParameterConfigBase const & config );

  virtual ~BiquadParameterMatrix() override;
};

template< typename CoeffType >
using BiquadCoefficientMatrixParameter = BiquadParameterMatrix< CoeffType >;

} // namespace pml
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::pml::BiquadCoefficientParameter< float >,
                       visr::pml::BiquadCoefficientParameter< float >::staticType(),
                       visr::pml::MatrixParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::BiquadCoefficientParameter< double >,
                       visr::pml::BiquadCoefficientParameter< double >::staticType(),
                       visr::pml::MatrixParameterConfig )

DEFINE_PARAMETER_TYPE(
    visr::pml::BiquadCoefficientListParameter< float >,
    visr::pml::BiquadCoefficientListParameter< float >::staticType(),
    visr::pml::MatrixParameterConfig )
DEFINE_PARAMETER_TYPE(
    visr::pml::BiquadCoefficientListParameter< double >,
    visr::pml::BiquadCoefficientListParameter< double >::staticType(),
    visr::pml::MatrixParameterConfig )

DEFINE_PARAMETER_TYPE( visr::pml::BiquadParameterMatrix< float >,
                       visr::pml::BiquadParameterMatrix< float >::staticType(),
                       visr::pml::MatrixParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::BiquadParameterMatrix< double >,
                       visr::pml::BiquadParameterMatrix< double >::staticType(),
                       visr::pml::MatrixParameterConfig )

#endif // VISR_PML_BIQUAD_PARAMETER_HPP_INCLUDED
