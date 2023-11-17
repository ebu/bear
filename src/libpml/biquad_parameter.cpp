/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "biquad_parameter.hpp"

#include <stdexcept>

namespace visr
{
namespace pml
{
template< typename CoeffType >
BiquadCoefficientParameter< CoeffType >::BiquadCoefficientParameter() = default;

template< typename CoeffType >
BiquadCoefficientParameter< CoeffType >::BiquadCoefficientParameter(
    EmptyParameterConfig const & config )
 : rbbl::BiquadCoefficient< CoeffType >()
{
}

template< typename CoeffType >
BiquadCoefficientParameter< CoeffType >::BiquadCoefficientParameter(
    ParameterConfigBase const & config )
try : BiquadCoefficientParameter(
    dynamic_cast< EmptyParameterConfig const & >( config ) )
{
}
catch( std::bad_cast const & )
{
  throw std::invalid_argument(
      "BiquadCoefficientParameter: Attempt to construct from wrong parameter "
      "config." );
}

template< typename CoeffType >
BiquadCoefficientParameter< CoeffType >::BiquadCoefficientParameter(
    rbbl::BiquadCoefficient< CoeffType > const rhs )
 : rbbl::BiquadCoefficient< CoeffType >( rhs )
{
}

template< typename CoeffType >
BiquadCoefficientParameter< CoeffType >::~BiquadCoefficientParameter() = default;

// ==========================================================================

template< typename CoeffType >
BiquadCoefficientListParameter< CoeffType >::BiquadCoefficientListParameter(
   VectorParameterConfig const & config )
 : rbbl::BiquadCoefficientList< CoeffType >( config.numberOfElements() )
{
}

template< typename CoeffType >
BiquadCoefficientListParameter< CoeffType >::BiquadCoefficientListParameter(
    ParameterConfigBase const & config )
try : BiquadCoefficientListParameter< CoeffType >(
    dynamic_cast< VectorParameterConfig const & >( config ) )
{
}
catch( std::bad_cast const & )
{
  throw std::invalid_argument(
      "BiquadCoefficientListParameter: Attempt to construct from wrong parameter "
      "config." );
}

template< typename CoeffType >
BiquadCoefficientListParameter< CoeffType >::BiquadCoefficientListParameter(
    rbbl::BiquadCoefficientList< CoeffType > const & base )
 : rbbl::BiquadCoefficientList< CoeffType >( base )
{
}

template< typename CoeffType >
BiquadCoefficientListParameter< CoeffType >::~BiquadCoefficientListParameter() =
    default;

// ==========================================================================
template< typename CoeffType >
BiquadParameterMatrix<CoeffType>::BiquadParameterMatrix( MatrixParameterConfig const & config )
  : BiquadParameterMatrix( config.numberOfRows(), config.numberOfColumns() )
{
}

template<typename CoeffType>
BiquadParameterMatrix<CoeffType>::BiquadParameterMatrix( ParameterConfigBase const & config )
try
 : BiquadParameterMatrix(
   dynamic_cast< MatrixParameterConfig const & >( config ) )
{
}
catch( std::bad_cast const & )
{
  throw std::invalid_argument( "BiquadParameterMatrix: Attempt to construct from wrong parameter config." );
}

template< typename CoeffType >
BiquadParameterMatrix< CoeffType >::BiquadParameterMatrix(
    rbbl::BiquadCoefficientMatrix< CoeffType > const & base )
 : rbbl::BiquadCoefficientMatrix< CoeffType >( base )
{
}


template<typename CoeffType>
BiquadParameterMatrix<CoeffType>::~BiquadParameterMatrix() = default;

// Explicit instantiations
template class BiquadCoefficientParameter<float>;
template class BiquadCoefficientListParameter<float>;
template class BiquadParameterMatrix<float>;

template class BiquadCoefficientParameter<double>;
template class BiquadCoefficientListParameter<double>;
template class BiquadParameterMatrix<double>;

} // namespace pml
} // namespace visr
