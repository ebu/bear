/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_PYTHON_EFL_CHECK_ERROR_HPP_INCLUDED
#define VISR_PYTHON_EFL_CHECK_ERROR_HPP_INCLUDED

#include <libefl/error_codes.hpp>

#include <libvisr/detail/compose_message_string.hpp>

namespace visr
{
namespace efl
{
namespace python
{
namespace detail
{

/**
 * Helper function to transoform efl error codes to exceptions
 */
inline void checkError( efl::ErrorCode res )
{
  if( res != efl::noError )
  {
    throw std::runtime_error( visr::detail::composeMessageString("Vector function failed with error: ",
                 errorMessage( res ) ) );
  }
}

} // namespace detail
} // namespace python
} // namespace efl
} // namespace visr

#endif // VISR_PYTHON_EFL_CHECK_ERROR_HPP_INCLUDED