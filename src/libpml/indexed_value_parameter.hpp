/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PML_INDEXED_STRING_PARAMETER_HPP_INCLUDED
#define VISR_PML_INDEXED_STRING_PARAMETER_HPP_INCLUDED

#include "empty_parameter_config.hpp"
#include "export_symbols.hpp"

#include <libvisr/parameter_type.hpp>
#include <libvisr/typed_parameter_base.hpp>

#include <string>
#include <utility>
#include <vector>

namespace visr
{
namespace pml
{

namespace // unnamed
{
/**
 * Type trait to assign a unique type id to each concrete IndexedValueParameter type.
 */
template<typename ValueType> struct IndexedValueParameterType {};

template<> struct IndexedValueParameterType<std::vector<float> >
{ static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "IndexedFloatVector" ); } };
template<> struct IndexedValueParameterType<std::vector<double> >
{ static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "IndexedDoubleVector" ); } };
template<> struct IndexedValueParameterType<std::string >
{ static constexpr const ParameterType ptype() { return detail::compileTimeHashFNV1( "IndexedString" ); }};
} // unnamed namespace



/**
 * @note Not sure whether we should introduce parameters to limit 
 */
template<typename IndexType, typename ValueType >
class VISR_PML_LIBRARY_SYMBOL IndexedValueParameter:
  public TypedParameterBase< IndexedValueParameter<IndexType, ValueType>, EmptyParameterConfig, IndexedValueParameterType<ValueType>::ptype() >
{
public:
  using DataType = std::pair<IndexType, ValueType >;

  IndexedValueParameter();

  explicit IndexedValueParameter( IndexType const & index, ValueType const & value );

  explicit IndexedValueParameter( ParameterConfigBase const & config );

  explicit IndexedValueParameter( const EmptyParameterConfig & config );

  virtual ~IndexedValueParameter( ) override;

  IndexType index( ) const
  {
    return mData.first;
  }

  ValueType const & value( ) const
  {
    return mData.second;
  }


  void setIndex( IndexType index )
  {
    mData.first = index;
  }

  void setValue( ValueType const & value )
  {
    mData.second = value;
  }


private:
  DataType mData;
};

// For some reason, the DEFINE_PARAMETER_TYPE macro fails if called with the full templated type.
// Note that the construct works for MatrixParameter (with only one template parameter)
using IndexedVectorDoubleType = visr::pml::IndexedValueParameter<std::size_t, std::vector<double> >;
using IndexedVectorFloatType = visr::pml::IndexedValueParameter<std::size_t, std::vector<float> >;
using IndexedStringType = visr::pml::IndexedValueParameter<std::size_t, std::string >;

} // namespace pml
} // namespace visr

DEFINE_PARAMETER_TYPE( visr::pml::IndexedStringType , visr::pml::IndexedStringType::staticType(), visr::pml::EmptyParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::IndexedVectorFloatType, visr::pml::IndexedVectorFloatType::staticType(), visr::pml::EmptyParameterConfig )
DEFINE_PARAMETER_TYPE( visr::pml::IndexedVectorDoubleType, visr::pml::IndexedVectorDoubleType::staticType(), visr::pml::EmptyParameterConfig )

#endif // VISR_PML_INDEXED_STRING_PARAMETER_HPP_INCLUDED
