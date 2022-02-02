/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "initialise_library.hpp"

#include "vector_functions.hpp"

#include "../vector_functions_reference.hpp"

namespace visr
{
namespace efl
{
namespace armv7l_neon_32bit
{

bool initialiseLibrary( char const * processor /*= ""*/ )
{
  VectorAddWrapper< float >::set( &armv7l_neon_32bit::vectorAdd<float> );
  VectorAddWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorAdd<std::complex<float> > );
  VectorAddInplaceWrapper< float >::set( &armv7l_neon_32bit::vectorAddInplace<float> );
  VectorAddInplaceWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorAddInplace<std::complex<float> > );
  VectorAddConstantWrapper< float >::set( &armv7l_neon_32bit::vectorAddConstant<float> );
  VectorAddConstantWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorAddConstant<std::complex<float> > );
  VectorAddConstantInplaceWrapper< float >::set( &armv7l_neon_32bit::vectorAddConstantInplace<float> );
  VectorAddConstantInplaceWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorAddConstantInplace<std::complex<float> > );

  VectorMultiplyWrapper< float >::set( &armv7l_neon_32bit::vectorMultiply<float> );
  VectorMultiplyWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiply<std::complex<float> > );
  VectorMultiplyInplaceWrapper< float >::set( &armv7l_neon_32bit::vectorMultiplyInplace<float> );
  VectorMultiplyInplaceWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiplyInplace<std::complex<float> > );
  VectorMultiplyConstantWrapper< float >::set( &armv7l_neon_32bit::vectorMultiplyConstant<float> );
  VectorMultiplyConstantWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiplyConstant<std::complex<float> > );
  VectorMultiplyConstantInplaceWrapper< float >::set( &armv7l_neon_32bit::vectorMultiplyConstantInplace<float> );
  VectorMultiplyConstantInplaceWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiplyConstantInplace<std::complex<float> > );

  VectorMultiplyAddWrapper< float >::set( &armv7l_neon_32bit::vectorMultiplyAdd<float> );
  VectorMultiplyAddWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiplyAdd<std::complex<float> > );
  VectorMultiplyAddInplaceWrapper< float >::set( &armv7l_neon_32bit::vectorMultiplyAddInplace<float> );
  VectorMultiplyAddInplaceWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiplyAddInplace<std::complex<float> > );
  VectorMultiplyConstantAddWrapper< float >::set( &armv7l_neon_32bit::vectorMultiplyConstantAdd<float> );
  VectorMultiplyConstantAddWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiplyConstantAdd<std::complex<float> > );
  VectorMultiplyConstantAddInplaceWrapper< float >::set( &armv7l_neon_32bit::vectorMultiplyConstantAddInplace<float> );
  VectorMultiplyConstantAddInplaceWrapper< std::complex<float> >::set( &armv7l_neon_32bit::vectorMultiplyConstantAddInplace<std::complex<float> > );

  return true;
}

bool uninitialiseLibrary()
{
  VectorAddWrapper< float >::set( &reference::vectorAdd<float> );
  VectorAddWrapper< std::complex<float> >::set( &reference::vectorAdd<std::complex<float> > );
  VectorAddInplaceWrapper< float >::set( &reference::vectorAddInplace<float> );
  VectorAddInplaceWrapper< std::complex<float> >::set( &reference::vectorAddInplace<std::complex<float> > );
  VectorAddConstantWrapper< float >::set( &reference::vectorAddConstant<float> );
  VectorAddConstantWrapper< std::complex<float> >::set( &reference::vectorAddConstant<std::complex<float> > );
  VectorAddConstantInplaceWrapper< float >::set( &reference::vectorAddConstantInplace<float> );
  VectorAddConstantInplaceWrapper< std::complex<float> >::set( &reference::vectorAddConstantInplace<std::complex<float> > );

  VectorMultiplyWrapper< float >::set( &reference::vectorMultiply<float> );
  VectorMultiplyWrapper< std::complex<float> >::set( &reference::vectorMultiply<std::complex<float> > );
  VectorMultiplyInplaceWrapper< float >::set( &reference::vectorMultiplyInplace<float> );
  VectorMultiplyInplaceWrapper< std::complex<float> >::set( &reference::vectorMultiplyInplace<std::complex<float> > );
  VectorMultiplyConstantWrapper< float >::set( &reference::vectorMultiplyConstant<float> );
  VectorMultiplyConstantWrapper< std::complex<float> >::set( &reference::vectorMultiplyConstant<std::complex<float> > );
  VectorMultiplyConstantInplaceWrapper< float >::set( &reference::vectorMultiplyConstantInplace<float> );
  VectorMultiplyConstantInplaceWrapper< std::complex<float> >::set( &reference::vectorMultiplyConstantInplace<std::complex<float> > );

  VectorMultiplyAddWrapper< float >::set( &reference::vectorMultiplyAdd<float> );
  VectorMultiplyAddWrapper< std::complex<float> >::set( &reference::vectorMultiplyAdd<std::complex<float> > );
  VectorMultiplyAddInplaceWrapper< float >::set( &reference::vectorMultiplyAddInplace<float> );
  VectorMultiplyAddInplaceWrapper< std::complex<float> >::set( &reference::vectorMultiplyAddInplace<std::complex<float> > );
  VectorMultiplyConstantAddWrapper< float >::set( &reference::vectorMultiplyConstantAdd<float> );
  VectorMultiplyConstantAddWrapper< std::complex<float> >::set( &reference::vectorMultiplyConstantAdd<std::complex<float> > );
  VectorMultiplyConstantAddInplaceWrapper< float >::set( &reference::vectorMultiplyConstantAddInplace<float> );
  VectorMultiplyConstantAddInplaceWrapper< std::complex<float> >::set( &reference::vectorMultiplyConstantAddInplace<std::complex<float> > );
  
  return true;
}

} // namespace armv7l_neon_32bit
} // namespace efl
} // namespace visr

