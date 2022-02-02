/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_INTEL_X86_64_CPU_FEATURES_HPP_INCLUDED
#define VISR_LIBEFL_INTEL_X86_64_CPU_FEATURES_HPP_INCLUDED

namespace visr
{
namespace efl
{
namespace intel_x86_64
{

struct CpuFeatures
{
public:
  CpuFeatures();
  
  bool hasMMX() const;
  bool hasSSE() const;
  bool hasSSE2() const;
  bool hasSSE3() const;
  bool hasSSE41() const;
  bool hasSSE42() const;
  bool hasAVX() const;
  bool hasAVX2() const;
  bool hasFMA3() const;
  bool hasAVX512F() const;
private:
  bool mMMX;
  bool mSSE;
  bool mSSE2;
  bool mSSE3;
  bool mSSE41;
  bool mSSE42;
  bool mAVX;
  bool mAVX2;
  bool mFMA3;
  bool mAVX512F;
};

} // namespace intel_x86_64
} // namespace efl
} // namespace visr

#endif // #define VISR_LIBEFL_INTEL_X86_64_CPU_FEATURES_HPP_INCLUDED
