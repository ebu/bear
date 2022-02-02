/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_BIQUAD_PARAMETER_HPP_INCLUDED
#define VISR_PML_BIQUAD_PARAMETER_HPP_INCLUDED

#include "matrix_parameter_config.hpp" // might be a forward declaration

#include "export_symbols.hpp"

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
 * Type trait to assign a unique type to each concrete BiquadParameterMatrix type.
 */
template<typename ElementType> struct BiquadMatrixParameterType{};

template<> struct BiquadMatrixParameterType<float>
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "BiquadFloatMatrix" ); }
};
template<> struct BiquadMatrixParameterType<double>
{
  static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "BiquadDoubleMatrix" ); }
};
} // unnamed

template<typename CoeffType >
class VISR_PML_LIBRARY_SYMBOL BiquadParameterMatrix: public TypedParameterBase<BiquadParameterMatrix<CoeffType>, MatrixParameterConfig, BiquadMatrixParameterType<CoeffType>::ptype() > ,
  public rbbl::BiquadCoefficientMatrix<CoeffType>
{
public:
  /**
   * Inherit all BiquadCoefficientMatrix constructors
   */
  using rbbl::BiquadCoefficientMatrix<CoeffType>::BiquadCoefficientMatrix;

  explicit BiquadParameterMatrix( MatrixParameterConfig const & config );

  explicit BiquadParameterMatrix( ParameterConfigBase const & config );

  virtual ~BiquadParameterMatrix() override;
};

} // namespace pml
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::pml::BiquadParameterMatrix<float>, visr::pml::BiquadParameterMatrix<float>::staticType(), visr::pml::MatrixParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::BiquadParameterMatrix<double>, visr::pml::BiquadParameterMatrix<double>::staticType(), visr::pml::MatrixParameterConfig )

#endif // VISR_PML_BIQUAD_PARAMETER_HPP_INCLUDED
