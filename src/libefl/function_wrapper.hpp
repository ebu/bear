/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_FUNCTION_WRAPPER_HPP_INCLUDED
#define VISR_LIBEFL_FUNCTION_WRAPPER_HPP_INCLUDED

#include "error_codes.hpp"

#include "export_symbols.hpp"

#include <functional>

namespace visr
{
namespace efl
{

/**
 * Helper template for type-safe function pointers that dispatch the efl API functions to a specific implementation.
 * @tparam Arguments A variadic template argument that contains the argument types of the function to wrap.
 *
 */
#define VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE( TemplateNameP, TemplateTypeP, ReturnTypeP, ... )\
template<typename ReturnType, class ... Arguments >\
class TemplateNameP ## Base\
{\
public:\
	static void set(std::function<ReturnType(Arguments...)> fcn) { sPtr = fcn; }\
	static bool isSet() { return sPtr != nullptr;};\
	static void reset() { sPtr = nullptr; }\
	static ReturnType call(Arguments ... args) { return sPtr(args...); }\
private:\
	static VISR_EFL_LIBRARY_SYMBOL std::function<ReturnType(Arguments...)> sPtr;\
};\
template <typename TemplateTypeP> \
using TemplateNameP = TemplateNameP ## Base< ReturnTypeP, __VA_ARGS__ >

// example: 
// VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE(VectorAddWrapper, T, ErrorCode, T const * const, T const * const, T * const, std::size_t, std::size_t);

/**
 * Helper template for type-safe function pointers that dispatch the efl API
 * functions to a specific implementation, specialised for functions with two
 * distinct template parameters.
 * @tparam Arguments A variadic template argument that contains the argument types of the function to wrap.
 *
 */
#define VISR_EFL_CREATE_FUNCTION_WRAPPER_TEMPLATE_TWO_TYPES( TemplateNameP, TemplateTypeP1, TemplateTypeP2, ReturnTypeP,  ... )\
template<typename ReturnType, class ... Arguments >\
class TemplateNameP ## Base\
{\
public:\
	static void set(std::function<ReturnType(Arguments...)> fcn) { sPtr = fcn; }\
	static bool isSet() { return sPtr != nullptr;};\
	static void reset() { sPtr = nullptr; }\
	static ReturnType call(Arguments ... args) { return sPtr(args...); }\
private:\
	static VISR_EFL_LIBRARY_SYMBOL std::function<ReturnType(Arguments...)> sPtr;\
};\
template < typename TemplateTypeP1, typename TemplateTypeP2 > \
using TemplateNameP = TemplateNameP ## Base< ReturnTypeP, __VA_ARGS__ >

} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_FUNCTION_WRAPPER_HPP_INCLUDED
