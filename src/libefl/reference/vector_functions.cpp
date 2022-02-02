/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "vector_functions_impl.hpp"

// #include <boost/preprocessor/seq/for_each.hpp>
// #include <boost/preprocessor/seq/enum.hpp>

namespace visr
{
namespace efl
{
namespace reference
{

template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorZero<float>( float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorZero<double>( double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorZero<std::complex<float> >( std::complex<float> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorZero<std::complex<double> >( std::complex<double> * const, std::size_t, std::size_t );

template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFill<float>( float const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFill<double>( double const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFill<std::complex<float> >( std::complex<float> const, std::complex<float> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFill<std::complex<double> >( std::complex<double> const, std::complex<double> * const, std::size_t, std::size_t );

template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRamp( float * const, std::size_t, float, float, bool, bool, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRamp( double * const, std::size_t, double, double, bool, bool, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRamp(std::complex<float> * const, std::size_t, std::complex<float>, std::complex<float>, bool, bool, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRamp(std::complex<double> * const, std::size_t, std::complex<double>, std::complex<double>, bool, bool, std::size_t);

template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<float>( float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<double>( double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<long double>( long double const * const, long double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<int8_t>( int8_t const * const, int8_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<uint8_t>( uint8_t const * const, uint8_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<int16_t>( int16_t const * const, int16_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<uint16_t>( uint16_t const * const, uint16_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<int32_t>( int32_t const * const, int32_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<uint32_t>( uint32_t const * const, uint32_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<int64_t>( int64_t const * const, int64_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<uint64_t>( uint64_t const * const, uint64_t * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<std::complex<float> >( std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopy<std::complex<double> >( std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t );

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAdd<float>( float const * const, float const * const ,
               float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAdd<double>( double const * const, double const * const ,
          double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAdd<std::complex<float> >(std::complex<float> const * const, std::complex<float> const * const,
std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAdd<std::complex<double> >( std::complex<double> const * const, std::complex<double> const * const,
                                                     std::complex<double> * const, std::size_t, std::size_t);


/** Explicit instantiation for types float and std::complex<double> */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddInplace<float>( float const * const,
                                            float * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddInplace<double>( double const * const,
                                             double * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddInplace<std::complex<float> >( std::complex<float> const * const,
                                                           std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddInplace<std::complex<double> >( std::complex<double> const * const,
                                                            std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstant<float>( float, float const * const,
                               float * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstant<double>( double, double const * const,
                                            double * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstant< std::complex<float> >(std::complex<float>, std::complex<float> const * const,
	std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstant<std::complex<double> >(std::complex<double>, std::complex<double> const * const,
	std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types std::complex<float> and std::complex<double> */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstantInplace<float>( float, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstantInplace<double>( double, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstantInplace<std::complex<float> >(std::complex<float>, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorAddConstantInplace<std::complex<double> >(std::complex<double>, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and std::complex<double> */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtract( float const * const, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtract( double const * const, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtract(std::complex<float> const * const, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtract(std::complex<double> const * const, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractInplace( float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractInplace( double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractInplace(std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractInplace(std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstant( float, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstant( double, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstant( std::complex<float>, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstant( std::complex<double>, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation of vectorSubtractConstantInplace*/
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstantInplace( float, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstantInplace( double, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstantInplace( std::complex<float>, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorSubtractConstantInplace( std::complex<double>, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiply( float const * const, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiply( double const * const, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiply( std::complex<double> const * const, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiply( std::complex<float> const * const, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t );

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyInplace( float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyInplace( double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyInplace( std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyInplace( std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstant( float, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstant( double, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstant( std::complex<float>, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstant( std::complex<double>, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t );

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantInplace( float, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantInplace( double, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantInplace( std::complex<float>, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantInplace( std::complex<double>, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAdd( float const * const, float const * const, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAdd( double const * const, double const * const, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAdd( std::complex<float> const * const, std::complex<float> const * const, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAdd( std::complex<double> const * const, std::complex<double> const * const, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t );

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAddInplace( float const * const, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAddInplace( double const * const, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAddInplace( std::complex<float> const * const, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyAddInplace( std::complex<double> const * const, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t );

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAdd( float, float const * const, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAdd( double, double const * const, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAdd( std::complex<float>, std::complex<float> const * const, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAdd( std::complex<double>, std::complex<double> const * const, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t);

/** Explicit instantiation for types float and double */
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAddInplace( float, float const * const, float * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAddInplace( double, double const * const, double * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAddInplace( std::complex<float>, std::complex<float> const * const, std::complex<float> * const, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorMultiplyConstantAddInplace( std::complex<double>, std::complex<double> const * const, std::complex<double> * const, std::size_t, std::size_t);

template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopyStrided( float const *, float *, std::size_t, std::size_t, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopyStrided( double const *, double *, std::size_t, std::size_t, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopyStrided( std::complex<float> const *, std::complex<float> *, std::size_t, std::size_t, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorCopyStrided( std::complex<double> const *, std::complex<double> *, std::size_t, std::size_t, std::size_t, std::size_t);

template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFillStrided( float, float *, std::size_t, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFillStrided( double, double *, std::size_t, std::size_t, std::size_t );
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFillStrided( std::complex<float>, std::complex<float> *, std::size_t, std::size_t, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorFillStrided( std::complex<double>, std::complex<double> *, std::size_t, std::size_t, std::size_t);

template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRampScaling(float const *, float const *, float *, float, float, std::size_t, bool, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRampScaling(double const *, double const *, double *, double, double, std::size_t, bool, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRampScaling(std::complex<float> const *, std::complex<float> const *, std::complex<float> *, std::complex<float>, std::complex<float>, std::size_t, bool, std::size_t);
template VISR_EFL_LIBRARY_SYMBOL ErrorCode vectorRampScaling(std::complex<double> const *, std::complex<double> const *, std::complex<double> *, std::complex<double>, std::complex<double>, std::size_t, bool, std::size_t);

} // namespace reference
} // namespace efl
} // namespace visr
