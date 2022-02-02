/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "cpu_features.hpp"

#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#else
#error "efl::intel_x86_64::getCpuId(): Unknown compiler"
#endif //  defined(__GNUC__) || defined(__clang__)

#include <ciso646>
#include <cstddef>

namespace visr
{
namespace efl
{
namespace intel_x86_64
{

namespace // unnamed
{

using CpuInfoType = int[4];
  
/**
 */
void getCpuId( int functionId, int count, CpuInfoType & result )
{
#if defined(__GNUC__) || defined(__clang__)
  __cpuid_count(functionId, count, result[0], result[1], result[2], result[3] );
#elif defined(_MSC_VER)
  __cpuidex( result, functionId, count );
#else
#error "efl::intel_x86_64::getCpuId(): Unknown compiler"
#endif //  defined(__GNUC__) || defined(__clang__)
}

bool bitIsSet( CpuInfoType const & vec, std::size_t regIdx, std::size_t bitIdx )  
{
  return (vec[regIdx] bitand static_cast<int>(1) << bitIdx ) != 0;
}
  
} // namespace unnamed

CpuFeatures::CpuFeatures()
 : mMMX{ false }
 , mSSE{ false }
 , mSSE2{ false }
 , mSSE3{ false }
 , mSSE41{ false }
 , mSSE42{ false }
 , mAVX{ false }
 , mAVX2{ false }
 , mFMA3{ false }
 , mAVX512F{ false }
{
  // Get basic information
  CpuInfoType baseInfo;
  getCpuId( 0x0, 0x0, baseInfo );
  int const maxInfoId = baseInfo[ 0 ];
  if( maxInfoId >= 1 )
  {
    CpuInfoType basicFeatureInfo;
    getCpuId( 0x01, 0x0, basicFeatureInfo );
    mMMX = bitIsSet( basicFeatureInfo, 3, 23 );
    mSSE = bitIsSet( basicFeatureInfo, 3, 25 );
    mSSE2 = bitIsSet( basicFeatureInfo, 3, 26 );
    mSSE3 = bitIsSet( basicFeatureInfo, 2, 9 );
    mSSE41 = bitIsSet( basicFeatureInfo, 2, 19 );
    mSSE42 = bitIsSet( basicFeatureInfo, 2, 20 );
    mAVX = bitIsSet( basicFeatureInfo, 2, 28 );
    mFMA3 =  bitIsSet( basicFeatureInfo, 2, 12 );
  }
  if( maxInfoId >= 0x07 )
  {
    CpuInfoType extendedFeatureInfo;
    getCpuId( 0x07, 0x0, extendedFeatureInfo );
    mAVX2 = bitIsSet( extendedFeatureInfo, 1, 5 );
    mAVX512F = bitIsSet( extendedFeatureInfo, 1, 16 );
  }
}

bool CpuFeatures::hasMMX() const
{
  return mMMX;
}

bool CpuFeatures::hasSSE() const
{
  return mSSE;
}

bool CpuFeatures::hasSSE2() const
{
  return mSSE2;
}

bool CpuFeatures::hasSSE3() const
{
  return mSSE3;
}

bool CpuFeatures::hasSSE41() const
{
  return mSSE41;
}

bool CpuFeatures::hasSSE42() const
{
  return mSSE42;
}

bool CpuFeatures::hasAVX() const
{
  return mAVX;
}

bool CpuFeatures::hasAVX2() const
{
  return mAVX2;
}

bool CpuFeatures::hasFMA3() const
{
  return mFMA3;
}

bool CpuFeatures::hasAVX512F() const
{
  return mAVX512F;
}
  
} // namespace intel_x86_64
} // namespace efl
} // namespace visr
