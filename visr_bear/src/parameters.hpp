#pragma once

#include <libpml/empty_parameter_config.hpp>
#include <libvisr/parameter_type.hpp>
#include <libvisr/typed_parameter_base.hpp>

#include "bear/api.hpp"
#include "listener_impl.hpp"

namespace bear {
template <typename ValueT>
struct ParameterTypeDetail {
};

template <>
struct ParameterTypeDetail<ObjectsInput> {
  static constexpr visr::ParameterType ptype() { return visr::detail::compileTimeHashFNV1("ObjectsInput"); }
};

template <>
struct ParameterTypeDetail<DirectSpeakersInput> {
  static constexpr visr::ParameterType ptype()
  {
    return visr::detail::compileTimeHashFNV1("DirectSpeakersInput");
  }
};

template <>
struct ParameterTypeDetail<HOAInput> {
  static constexpr visr::ParameterType ptype() { return visr::detail::compileTimeHashFNV1("HOAInput"); }
};

template <typename ValueType>
struct ADMParameter : public visr::TypedParameterBase<ADMParameter<ValueType>,
                                                      visr::pml::EmptyParameterConfig,
                                                      ParameterTypeDetail<ValueType>::ptype()> {
  ADMParameter(const visr::ParameterConfigBase &) {}
  ADMParameter(size_t index, ValueType value) : index(index), value(std::move(value)) {}

  size_t index;
  ValueType value;
};

using ADMParameterObjectsInput = ADMParameter<ObjectsInput>;
using ADMParameterDirectSpeakersInput = ADMParameter<DirectSpeakersInput>;
using ADMParameterHOAInput = ADMParameter<HOAInput>;

struct ListenerParameter
    : public visr::TypedParameterBase<ListenerParameter,
                                      visr::pml::EmptyParameterConfig,
                                      visr::detail::compileTimeHashFNV1("ListenerParameter")>,
      public ListenerImpl {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  ListenerParameter(const visr::ParameterConfigBase &) {}
  ListenerParameter(const ListenerImpl &listener) : ListenerImpl(listener) {}
  ListenerParameter() {}
};

void init_parameters();
}  // namespace bear

DEFINE_PARAMETER_TYPE(bear::ADMParameterObjectsInput,
                      bear::ADMParameterObjectsInput::staticType(),
                      visr::pml::EmptyParameterConfig)

DEFINE_PARAMETER_TYPE(bear::ADMParameterDirectSpeakersInput,
                      bear::ADMParameterDirectSpeakersInput::staticType(),
                      visr::pml::EmptyParameterConfig)

DEFINE_PARAMETER_TYPE(bear::ADMParameterHOAInput,
                      bear::ADMParameterHOAInput::staticType(),
                      visr::pml::EmptyParameterConfig)

DEFINE_PARAMETER_TYPE(bear::ListenerParameter,
                      bear::ListenerParameter::staticType(),
                      visr::pml::EmptyParameterConfig)
