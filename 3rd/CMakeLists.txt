

# Put binaries built within 3rd/ into 3rd/bin.
# This avoids cluttering the main $VISR-build/bin directory with ffts unit tests.
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin )

# Build the FFTS library.
# First, provide hard-coded settings for the FFTS'configuration options
set( GENERATE_POSITION_INDEPENDENT_CODE ON CACHE BOOL
"Force position-independent code for 3rd-party library ffts." FORCE )
#  According to FFTS' CmakeLists.txt, ENABLE_AVX is for debugging only.
set( ENABLE_AVX OFF CACHE BOOL "Disable AVX code generation for ffts." )
if( VISR_SYSTEM_PROCESSOR STREQUAL 'armv7l' )
  set( ENABLE_NEON ON CACHE BOOL "FFTS: Enable double-precision arithmethic." FORCE )
endif( VISR_SYSTEM_PROCESSOR STREQUAL 'armv7l' )
set( ENABLE_DOUBLE ON CACHE BOOL "FFTS: Enable double-precision arithmethic." FORCE )
# It is not clear whether and where the shared variant will be used,
# but certainly only if VISR shared libraries are also built.
if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  set( ENABLE_SHARED ON CACHE BOOL "FFTS: Enable building a shared library." FORCE )
else( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  set( ENABLE_SHARED OFF CACHE BOOL "FFTS: Enable building a shared library." FORCE )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
set( DISABLE_DYNAMIC_CODE ON CACHE BOOL "FFTS: Disable dynamic code generation." FORCE )
set( ENABLE_STATIC ON CACHE BOOL "FFTS: Enable building a static library." FORCE )
# Make these options less visible
mark_as_advanced( FORCE GENERATE_POSITION_INDEPENDENT_CODE ENABLE_AVX ENABLE_NEON ENABLE_DOUBLE ENABLE_SHARED ENABLE_STATIC ENABLE_VFP DISABLE_DYNAMIC_CODE )
add_subdirectory( ffts ${CMAKE_BINARY_DIR}/3rd/ffts )

# KissFFT
# Always build the static variant, because it's used by shared VISR libraries, too.
set( KISSFFT_BUILD_STATIC_LIBRARY ON CACHE BOOL "KissFFT: Build static library" FORCE )
# It is not clear whether and where the shared variant will be used,
# but certainly only if VISR shared libraries are also built.
if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  set( KISSFFT_BUILD_SHARED_LIBRARY ON CACHE BOOL "KissFFT: Build shared library" FORCE )
else( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  set( KISSFFT_BUILD_SHARED_LIBRARY OFF CACHE BOOL "KissFFT: Build shared library" FORCE )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
add_subdirectory( kissfft ${CMAKE_BINARY_DIR}/3rd/kissfft )
