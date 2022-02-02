# Copyright Institute of Sound and Vibration Research - All rights reserved

# Top-level build configuration for project VISceRal

# 3.0.0+ needed for export(EXPORT) command in src/CMakeLists.txt
# >= 3.1 for CMAKE_CXX_STANDARD
# 3.3 for IN_LIST operator in if() conditions
CMAKE_MINIMUM_REQUIRED (VERSION 3.3.0)

# Avoid warnings about the CXX_VISIBILITY_PRESET set for all target types.
if( POLICY CMP0063 ) # (only if the CMake version supports this policy
  CMAKE_POLICY(SET CMP0063 NEW)
endif( POLICY CMP0063 )
# Policy for supporting <package>_ROOT CMake or environment variables in find_package() (CMake >= 3.12 only)
if( POLICY CMP0074 )
  CMAKE_POLICY(SET CMP0074 NEW)
endif( POLICY CMP0074 )
if( POLICY CMP0057 )
  CMAKE_POLICY(SET CMP0057 NEW)
else( POLICY CMP0057 )
  message( FATAL_ERROR "Policy CMP0057 not supported (IN_LIST operator), although CMake version requirement is met.")
endif( POLICY CMP0057 )
if( POLICY CMP0090 ) # Disable writing into CMake registry.
  CMAKE_POLICY(SET CMP0090 NEW) # Do not write into the package registry.
else( POLICY CMP0090 )
  set( CMAKE_EXPORT_NO_PACKAGE_REGISTRY ON ) # Emulate the NEW behaviour)
endif( POLICY CMP0090 )
# Enable setting the runtime library type on MSVC platforms.
# This is done by setting CMAKE_MSVC_RUNTIME_LIBRARY, for example
# -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>
if( POLICY CMP0091 )
  CMAKE_POLICY(SET CMP0091 NEW)
endif( POLICY CMP0091 )


# The name of our top-level project.
PROJECT( VISR CXX C )

# Define central version numbers for the project
# TODO: Decide whether to move that to an external CMake file .
set( VISR_MAJOR_VERSION 0 )
set( VISR_MINOR_VERSION 13 )
set( VISR_PATCH_VERSION 0 )

add_definitions( -DVISR_MAJOR_VERSION=${VISR_MAJOR_VERSION} )
add_definitions( -DVISR_MINOR_VERSION=${VISR_MINOR_VERSION} )
add_definitions( -DVISR_PATCH_VERSION=${VISR_PATCH_VERSION} )

# Add support for unit tests
INCLUDE( CTest )

## Set C++ standard used.

# Enable C++ 11 support
# This requires CMake >= 3.1
# TODO: Check when we can increase the requirement to C++14
set( CMAKE_CXX_STANDARD 11 )

# Treat the above standard as a hard requirement rather than 'decaying' to a
# previous standard if the requirments aren' met
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# Disable compiler-specific extensions (e.g., GNU extensions)
set( CMAKE_CXX_EXTENSIONS OFF )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake_modules")

# Default path for static libraries
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/staticlib )

# Copy the executables to the binary directory
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )

# Default destination for shared libraries (not DLLs)
SET( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

# Default path for DLLs on windows
SET( RUNTIME_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

set( VISR_VERSIONED_NAME VISR-${VISR_MAJOR_VERSION}.${VISR_MINOR_VERSION}.${VISR_PATCH_VERSION})

#############################################################
# Create a system string.
SET( VISR_SYSTEM_NAME ${CMAKE_SYSTEM_NAME} )

# Mac OS specific settings (mainly rpath handling)
if( VISR_SYSTEM_NAME MATCHES "Darwin" )
  set( VISR_SYSTEM_NAME MacOS ) # Replace CMake's name for MacOS X by a more descriptive one.

  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX /Applications/${VISR_VERSIONED_NAME} CACHE PATH "install prefix" FORCE)
  ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif( VISR_SYSTEM_NAME MATCHES "Darwin" )

# Allow disabling of setting rpath - parent projects may want to customise it, based on installation
OPTION( AUTOSET_RPATH "Automatically set rpath" ON )
if(AUTOSET_RPATH)
  
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/3rd;${CMAKE_INSTALL_PREFIX}/lib")

  # Activate rpath support
  set( CMAKE_MACOSX_RPATH ON)
  # Use, i.e., don't skip rpaths in the build tree
  set( CMAKE_SKIP_BUILD_RPATH FALSE)
  set( CMAKE_BUILD_WITH_INSTALL_NAME_DIR TRUE)
  # When building, set the rpath to the libraries in the build tree, not to the install tree
  set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

  # Add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif(AUTOSET_RPATH)

SET( VISR_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} )
IF( VISR_SYSTEM_PROCESSOR MATCHES "AMD64" )
  SET( VISR_SYSTEM_PROCESSOR x86_64 )
ENDIF( VISR_SYSTEM_PROCESSOR MATCHES "AMD64" )
STRING( TOLOWER ${VISR_SYSTEM_NAME}-${VISR_SYSTEM_PROCESSOR} VISR_PLATFORM_NAME )
# For use as a C/C++ preprocessor definition, we must not use hyphens.
STRING( TOLOWER ${VISR_SYSTEM_NAME}_${VISR_SYSTEM_PROCESSOR} VISR_PLATFORM_NAME_MACRO )


# MESSAGE( STATUS "VISR system processor: " ${VISR_SYSTEM_PROCESSOR} )
# MESSAGE( STATUS "VISR system name:      " ${VISR_SYSTEM_NAME} )
# MESSAGE( STATUS "VISR platform name:    " ${VISR_PLATFORM_NAME} )

# Make the platform strings available to the source code.
ADD_DEFINITIONS("-DVISR_SYSTEM_PROCESSOR_${VISR_SYSTEM_PROCESSOR}")
ADD_DEFINITIONS("-DVISR_SYSTEM_NAME_${VISR_SYSTEM_NAME}")
ADD_DEFINITIONS("-DVISR_PLATFORM_NAME_${VISR_PLATFORM_NAME_MACRO}")

############################################################
# Set platform-specific compiler flags

# This covers Linux and MacOS X
if(UNIX)
  # Support for the Raspberry Pi: Enable hardware floating-point arithmetic
  if( ${VISR_SYSTEM_PROCESSOR} MATCHES "armv7l" )
    add_compile_options( -mfpu=neon -march=armv7-a -mfloat-abi=hard )
  endif( ${VISR_SYSTEM_PROCESSOR} MATCHES "armv7l" )
  # TODO: Add more elaborate configuration for -march and -mtune
  # At the moment, the code is invariably built for the architecture
  # features of the machine it is compiled on.
  add_compile_options( -Wall --pedantic )
  # Avoid super-picky 'nested anon types' warning
  if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang") # Matches regular Clang and AppleClang (XCode)
    # message( STATUS "Detected LLVM C++ compiler" )
    # Remove nonsensical warnings due to unknown pragmas introduced by -pedantic
    # (as pragmas are compiler-specific, unknown ones should be ignored silently anyway)
    add_compile_options( -Wno-nested-anon-types -Wno-unknown-pragmas )
    # Avoid warnings if (static) member variables of templates are not explicitly defined in the same translation unit.
    # This happens, for example, with the function pointer dispatch mechanism in libefl.
    # A missing definition of a variable in question results in an undefined reference at the linking stage anyway.
    add_compile_options( -Wno-undefined-var-template )
  endif (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
endif(UNIX)

if( WIN32 ) # Covers 64-bit Windows as well
  # Macro to determine the Windows version, stolen from
  # http://stackoverflow.com/questions/9742003/platform-detection-in-cmake
  macro(get_win32_winnt_version version)
    if (WIN32 AND CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" ver ${ver})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})
        set(${version} "0x${ver}")
    endif()
  endmacro()
  get_win32_winnt_version( WIN_VERSION_ID )
  message( STATUS "Window version: " ${WIN_VERSION_ID} )
  add_definitions( -D_WINN32_WINNT=${WIN_VERSION_ID} )
  # Avoid specific compiler warnings of the Visual Studio  compiler.
  add_definitions( -D_SCL_SECURE_NO_WARNINGS ) # STL algorithms with pointers instead of 'real' iterators are perfectly legal.
  # Avoid complaints about STL members in classes with VISR_CORE_LIBRARY_SYMBOL not having DLL interfaces.
  add_definitions( "/wd4251" )

  # TODO: Fine to remove commented-out next line?
  # add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /wd4251 /nologo" )
  add_definitions( "-D_WIN32_WINNT=0x0601" )

  add_compile_options( /W3 ) # Increase warning level (/W4 and /Wall generate lots of output not related to coding flaws.
endif( WIN32 )

############################################################
# Set architecture options.
set( BUILD_PROCESSOR_GENERATION "generic" CACHE STRING "CPU Architecture for which the binaries are built." )
if( ${VISR_SYSTEM_PROCESSOR} MATCHES "x86_64" )
  # These values work both for GCC and CLANG
  set( PROCESSOR_GENERATION_LIST "generic;native;skylake-avx512;skylake;broadwell;haswell;ivybridge;sandybridge;westmere;nehalem"  )
endif( ${VISR_SYSTEM_PROCESSOR} MATCHES "x86_64" )
set_property( CACHE BUILD_PROCESSOR_GENERATION PROPERTY STRINGS ${PROCESSOR_GENERATION_LIST} )

if( ${VISR_SYSTEM_PROCESSOR} MATCHES "x86_64" )
  if( WIN32 )
    # TODO: Translate into Windows flags (AFAIK only
    # /arch:AVX and /arch:AVX2 are supported)
  else( WIN32 )
    # '-march=generic' is not supported by GCC and Clang
    # because it is the default anyway.
    if( NOT ${BUILD_PROCESSOR_GENERATION} MATCHES "generic" )
        add_compile_options( -march=${BUILD_PROCESSOR_GENERATION} )
    endif( NOT ${BUILD_PROCESSOR_GENERATION} MATCHES "generic" )
    # Tune for the same architecture.
    # This might make a difference especially if '-march=native' is chosen.
    # https://lemire.me/blog/2018/07/25/it-is-more-complicated-than-i-thought-mtune-march-in-gcc/
    add_compile_options( -mtune=${BUILD_PROCESSOR_GENERATION} )
  endif( WIN32 )
endif( ${VISR_SYSTEM_PROCESSOR} MATCHES "x86_64" )

# TODO: Generate architecture flags for other platforms (e.g., ARM)

############################################################
# Logic for building, using, and installing libraries

option( BUILD_INSTALL_SHARED_LIBRARIES "Whether to provide shared libraries in the installation packages." ON )
option( BUILD_INSTALL_STATIC_LIBRARIES "Whether to provide static libraries in the installation packages." OFF )
option( BUILD_INSTALL_STATIC_PIC_LIBRARIES "Whether to provide static position-independent libraries in the installation packages." OFF )

set( BUILD_LIBRARY_TYPE_FOR_APPS "shared" CACHE STRING "The library type used for standalone applications." )
set( VISR_LIBRARY_TYPES "shared;static" )
set_property( CACHE BUILD_LIBRARY_TYPE_FOR_APPS PROPERTY STRINGS ${VISR_LIBRARY_TYPES} )

# We need to know whether the standalone applications are built, because this influences which
# library types are built.
option( BUILD_STANDALONE_APPLICATIONS "Whether to build the standalone VISR applications." ON)

# List to contain the library variants (static, shared, static_pic) to be built.
# Start with an empty list which is populated below
set( VISR_BUILD_LIBRARY_TYPES )
# Decide which library types are built and store them in a list that is used in
# subdirectories.
# Static libraries are built if they are used in the standalone applications or
# if they are to be installed.
if( (BUILD_STANDALONE_APPLICATIONS AND (BUILD_LIBRARY_TYPE_FOR_APPS STREQUAL "static"))
  OR BUILD_INSTALL_STATIC_LIBRARIES )
  list( APPEND VISR_BUILD_LIBRARY_TYPES "static")
endif( (BUILD_STANDALONE_APPLICATIONS AND (BUILD_LIBRARY_TYPE_FOR_APPS STREQUAL "static"))
  OR BUILD_INSTALL_STATIC_LIBRARIES )
if( BUILD_INSTALL_SHARED_LIBRARIES )
  list( APPEND VISR_BUILD_LIBRARY_TYPES "shared")
endif( BUILD_INSTALL_SHARED_LIBRARIES )
if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  list( APPEND VISR_BUILD_LIBRARY_TYPES "static_pic" )
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )


############################################################
# Create universal binaries on MacOS
IF( VISR_SYSTEM_NAME MATCHES "MacOS" )
  OPTION( BUILD_MACOS_UNIVERSAL_BINARIES "Universal (32/64-bit) binaries on MacOS X" OFF)
  IF( BUILD_MACOS_UNIVERSAL_BINARIES )
    SET( OSX_UNIVERSAL ON )
    SET( CMAKE_OSX_ARCHITECTURES "i386;x86_64" )
  ENDIF( BUILD_MACOS_UNIVERSAL_BINARIES )
ENDIF( VISR_SYSTEM_NAME MATCHES "MacOS" )

############################################################
# Set the minimum Mac OS version.
if( VISR_SYSTEM_NAME MATCHES "MacOS" )
#  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
 set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")
endif( VISR_SYSTEM_NAME MATCHES "MacOS" )

############################################################
# Set default visibility of shared library symbols to hidden (as on Windows platforms)
SET( CMAKE_C_VISIBILITY_PRESET hidden )
SET( CMAKE_CXX_VISIBILITY_PRESET hidden )

############################################################
# Support for Building Matlab externals

OPTION( BUILD_MATLAB_EXTERNALS "Build Matlab externals" OFF )

IF( BUILD_MATLAB_EXTERNALS )
  FIND_PACKAGE(Matlab REQUIRED)
  IF(MATLAB_FOUND)
    MESSAGE( STATUS "Matlab found." )
  ELSE( MATLAB_FOUND )
    MESSAGE( FATAL_ERROR, "Matlab not found, cancelling compilation." )
  ENDIF( MATLAB_FOUND )
ENDIF( BUILD_MATLAB_EXTERNALS )

################################################################################
# Support for Max/MSP externals
IF( WIN32 OR APPLE )
  OPTION( BUILD_MAX_MSP_EXTERNALS "Build externals for MAX/MSP" OFF )
  IF( BUILD_MAX_MSP_EXTERNALS )
    SET( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{MAX_SDK_ROOT} )
    FIND_PACKAGE( MaxSDK REQUIRED)
    MESSAGE( STATUS "MAX SDK found: " ${MAXSDK_MAINPATH} )
  ENDIF(BUILD_MAX_MSP_EXTERNALS)

  SET( MAXSDK_EXTERNAL_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/max )
ENDIF(  WIN32 OR APPLE )

################################################################################
# Support for Python bindings of selected VISR functionality
option( BUILD_PYTHON_BINDINGS "Python bindings of selected VISR functionality" ON )

if( BUILD_PYTHON_BINDINGS )
  # Finding Python and the pybind11 directory. This is done using the pybind11
  # mechanisms.
  # To build VISR for a non-default Python version, set the CMake cache variable PYTHON_EXECUTABLE to the respective interpreter binary.
  set( PYBIND11_DIR ${PROJECT_SOURCE_DIR}/3rd/pybind11/ CACHE PATH "Location of the Pybind11 library. Defaults to the internally provided version in 3rd/pybind11." )

  message( STATUS "Using PYBIND11_DIR: " ${PYBIND11_DIR} )

  # Ensure that pybind11's targets are exported within the VISR export set.
  # This is necessary to export static versions of libpythonsupport
  # and libpythoncomponents, which have a private link dependency to pybind11::embed.
  # A drawback of the current approach is that the targets are exported as,
  # e.g., VISR::embed, but we cannot set the namespace because this is done in
  # the pybind11 tree.
  set( PYBIND11_EXPORT_NAME VISR-3rd_party_static-exports )
  # Force setting of pybind11 option (necessary in order to get the targets exported)
  set( PYBIND11_INSTALL ON CACHE BOOL "Force exporting of pybind11 targets." FORCE )

  # Enable the pybind11 cmake support via add_subdirectory
  # This takes, amongst other things, care of providing proper
  # compile and link options.
  # The second argument specifies the binary directory and is needed if
  # ${PYBIND11_DIR} could be outside the project source directory.
  # EXCLUDE_FROM_ALL means that the target will not be built
  # (which isn't relevant for a header-only library anyway), and that it
  # is not shown in IDE project files.
  # TODO: consider to use the binary dir only if an outside PYBIND11_DIR
  # is used. This would avoid a 3rd/ folder in the build directory unless
  # necessary.
  add_subdirectory( ${PYBIND11_DIR}
  ${CMAKE_BINARY_DIR}/3rd/pybind11 EXCLUDE_FROM_ALL )

  # Define a Python version string. Note that the variables returned from
  # the pybind11 may not contain the Python patch version.
  # However, this is not problematic because only the major.minor
  # version is needed to set the VISR package name.
  set( VISR_PYTHON_VERSION_MAJOR_MINOR ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} )

  # Define the pybind11 version as a version string.
  # This string is imported as a target property of the
  # VISR::pythonsupport libraries and can be used by dependent
  # project to check whether they are using a compatible
  # pybind11 version.
  get_directory_property(PYBIND11_VERSION_MAJOR DIRECTORY ${PYBIND11_DIR}
  DEFINITION PYBIND11_VERSION_MAJOR )
  get_directory_property(PYBIND11_VERSION_MINOR DIRECTORY ${PYBIND11_DIR}
  DEFINITION PYBIND11_VERSION_MINOR )
  get_directory_property(PYBIND11_VERSION_PATCH DIRECTORY ${PYBIND11_DIR}
  DEFINITION PYBIND11_VERSION_PATCH )
  set( VISR_PYBIND11_VERSION
    ${PYBIND11_VERSION_MAJOR}.${PYBIND11_VERSION_MINOR}.${PYBIND11_VERSION_PATCH} )

  # get PYTHON_SITE_PACKAGES to figure out where to install; this looks at
  # PYTHON_EXECUTABLE to figure out which python to use, which makes sure that
  # it's always compatible with the python found by the pybind11 add_subdirectory
  # above
  list(APPEND CMAKE_MODULE_PATH "${PYBIND11_DIR}/tools")
  find_package(PythonLibsNew REQUIRED)
endif( BUILD_PYTHON_BINDINGS )

option( BUILD_DISABLE_THREADS "Whether to disable the use of threads. This is mainly useful mainly specialised purposes, e.g., Web assembly" OFF )

################################################################################
# Set the install directories of special output types.
# For Linux this adjusts the path to put the items under /usr/share/visr

if( VISR_SYSTEM_NAME MATCHES "Linux" )
  set( VISR_TOPLEVEL_INSTALL_DIRECTORY "share/visr" ) # Directory under CMAKE_INSTALL_PREFIX where visr-specific content is placed.
else( VISR_SYSTEM_NAME MATCHES "Linux" )
  set( VISR_TOPLEVEL_INSTALL_DIRECTORY "./" ) # CMAKE_INSTALL_PREFIX is already OK
endif( VISR_SYSTEM_NAME MATCHES "Linux" )

# Installation destination for third-party libraries: shared libs (incl. the DLL part and
# import lib on Windows) and static libs.
# This will be appended to the installation prefix.
if( VISR_SYSTEM_NAME MATCHES Linux )
  # Installed files go into the /usr tree, so we must use the standard location for the libs.
  set( THIRDPARTY_LIBRARY_INSTALL_DIRECTORY lib )
  set( THIRDPARTY_RUNTIME_INSTALL_DIRECTORY lib)
  set( THIRDPARTY_ARCHIVE_INSTALL_DIRECTORY lib )
else()
  # TODO: Decide whether this should go into lib/ on Windows to avoid an
  # additional entry to be added to %PATH%
  set( THIRDPARTY_LIBRARY_INSTALL_DIRECTORY 3rd )
  set( THIRDPARTY_RUNTIME_INSTALL_DIRECTORY 3rd )
  set( THIRDPARTY_ARCHIVE_INSTALL_DIRECTORY 3rd )
endif( VISR_SYSTEM_NAME MATCHES Linux )

###########################################################
# Support for 3rd-party libraries.

# Add 3rd-party libraries that are built locally.
add_subdirectory( 3rd )

# Threads might be used through boost::filesystem, the C++11 threads functionality,
# or through direct OS calls.
if( BUILD_DISABLE_THREADS )
  add_definitions( -DVISR_DISABLE_THREADS=1 )
else()
  find_package(Threads REQUIRED)
endif( BUILD_DISABLE_THREADS )

# if( NOT WIN32 )
## Portaudio
option( BUILD_AUDIOINTERFACES_PORTAUDIO "Support PortAudio audio interface" ON )
if( BUILD_AUDIOINTERFACES_PORTAUDIO )
  # For windows, we ship the includes and libraries.
  # TODO: Support system-specific library directories
  # TODO: Provide separate libraries for debug and release builds.
  if( WIN32 ) # This includes 64-bit platforms!
    # NOTE: It would be nicer to pass the search path to the FIND_PACKAGE
    # call instead of setting it globally.
    list( APPEND CMAKE_INCLUDE_PATH TO_CMAKE_PATH(${PROJECT_SOURCE_DIR}/3rd/include/portaudio) )
    list( APPEND CMAKE_LIBRARY_PATH TO_CMAKE_PATH(${PROJECT_SOURCE_DIR}/3rd/lib/${VISR_PLATFORM_NAME}/portaudio) )
  endif( WIN32 )
  find_package( Portaudio REQUIRED )
endif( BUILD_AUDIOINTERFACES_PORTAUDIO )
# endif( NOT WIN32 )

## Jack Audio Connection Kit (JACK)
option( BUILD_AUDIOINTERFACES_JACK "Support the Jack (Jack Audio Connection Kit) audio interface" OFF )
if( BUILD_AUDIOINTERFACES_JACK )
  # Preliminary support for testing the Jack library on Windows platforms.
  if( WIN32 )
    set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{JACK_ROOT} )
  endif( WIN32 )
  find_package( Jack REQUIRED)
endif( BUILD_AUDIOINTERFACES_JACK )

## Libsndfile
option( BUILD_USE_SNDFILE_LIBRARY "Use libsndfile functionality (loading/storing of audio files)" ON )
if( BUILD_USE_SNDFILE_LIBRARY )
  # On Linux and MacOS, we use the versions provided by the OS (or brew on MacOS)
  if( WIN32 ) # This includes 64-bit platforms!
    # NOTE: It would be nicer to pass the search path to the FIND_PACKAGE
    # call instead of setting it globally.
    list( APPEND CMAKE_INCLUDE_PATH TO_CMAKE_PATH(${PROJECT_SOURCE_DIR}/3rd/include/sndfile) )
    list( APPEND CMAKE_LIBRARY_PATH TO_CMAKE_PATH(${PROJECT_SOURCE_DIR}/3rd/lib/${VISR_PLATFORM_NAME}/sndfile) )
  endif( WIN32 )
  find_package( SndFile REQUIRED )
endif( BUILD_USE_SNDFILE_LIBRARY )

# IPP
OPTION( BUILD_USE_IPP "Use the Intel IPP for fast arithmetic functions and trasnforms. Set the environment variable IPP_ROOT accordingly." OFF )
IF( BUILD_USE_IPP )
  # Use the variable IPP_ROOT to point to the base directory of the library.
  FIND_PACKAGE( IPP REQUIRED )
ENDIF( BUILD_USE_IPP )

## Boost support
# On Linux and Windows, the boost libraries provided by the OS image (or brew on MacOS)
# should be found automatically.
# Otherwise (in particular for Windows platforms), an environment variable
# BOOST_ROOT should hold the path to the boost root directory.

# Define a minimum boost version
# Note: This requirement is chosen heuristically at the moment.
# Todo: Replace by a substantiated version requirement.
set( VISR_BOOST_MINIMUM_VERSION 1.50 )

option( Boost_USE_STATIC_LIBS "Whether to use static or shared boost libraries." OFF)

# Define the boost libraries that will be needed in deployed installations.
# In boost 1.59 on Windows, chrono is an implicit dependency of boost_thread. So in order to locate the runtime DLL for packaging, we mus add this library here.
# Note that unit_test_framework is excluded here because it's only used in local builds.
set( VISR_BOOST_INSTALL_LIBRARIES chrono date_time filesystem program_options system regex timer )
if( NOT BUILD_DISABLE_THREADS )
  list( APPEND VISR_BOOST_INSTALL_LIBRARIES thread atomic)
endif( NOT BUILD_DISABLE_THREADS )
# The set of all boost libraries used by the build and test stages.
set( VISR_BOOST_LIBRARIES ${VISR_BOOST_INSTALL_LIBRARIES} unit_test_framework )
# Note that boost regex is used only with GCC <= 4.8 on Linux because of the faulty std::regex implementation in these GCC versions.
# However, we do not exclude boost::regex here because of the number of places it is handled in the build structure (especially packaging)

# Switch to the "-mt" set of libraries. Also prevents warnings due to already
# defined import locations.
set(Boost_USE_MULTITHREADED TRUE)

# Store/restore value of Boost_USE_STATIC_LIBS - vcpkg on Windows will reset it during find_package
set(_Boost_USE_STATIC_LIBS ${Boost_USE_STATIC_LIBS})
find_package( Boost ${VISR_BOOST_MINIMUM_VERSION} REQUIRED COMPONENTS ${VISR_BOOST_LIBRARIES} )
set(Boost_USE_STATIC_LIBS ${_Boost_USE_STATIC_LIBS})

# Avoid a pragma message caused by an internal inconsistency between boost::bind and
# boost::property_tree.
message( STATUS "Boost_VERSION_STRING " ${Boost_VERSION_STRING} )
if( (${Boost_VERSION_STRING} VERSION_GREATER 1.72.0)
 AND (${Boost_VERSION_STRING} VERSION_LESS 1.76.0) )
  add_definitions( -DBOOST_BIND_GLOBAL_PLACEHOLDERS=1 )
endif( (${Boost_VERSION_STRING} VERSION_GREATER 1.72.0)
 AND (${Boost_VERSION_STRING} VERSION_LESS 1.76.0) )


# Static Boost libraries might not be usable in shared libraries or loadable
# modules, depending on how they are compiled
if( Boost_USE_STATIC_LIBS AND NOT WIN32 )
  if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
    message( WARNING
        "Building shared libraries if Boost_USE_STATIC_LIBS is on. "
        "This might break if Boost was compiled without position-independent code."
        )
  endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
endif( Boost_USE_STATIC_LIBS AND NOT WIN32 )

############################################################
# Copy 3rd party libraries and set RPATH for MacOs
if( BUILD_USE_SNDFILE_LIBRARY AND (VISR_SYSTEM_NAME MATCHES "MacOS") )
  find_package(Flac REQUIRED)
  find_package(OggVorbis REQUIRED)
endif( BUILD_USE_SNDFILE_LIBRARY AND (VISR_SYSTEM_NAME MATCHES "MacOS") )

if( BUILD_PYTHON_BINDINGS )
  set( PYTHON_MODULE_INSTALL_DIRECTORY "${PYTHON_SITE_PACKAGES}" )
else( BUILD_PYTHON_BINDINGS )
  # this sometimes gets used if BUILD_PYTHON_BINDINGS is not set
  set( PYTHON_MODULE_INSTALL_DIRECTORY "${VISR_TOPLEVEL_INSTALL_DIRECTORY}/python" )
endif( BUILD_PYTHON_BINDINGS )
set( DOCUMENTATION_INSTALL_DIRECTORY "${VISR_TOPLEVEL_INSTALL_DIRECTORY}/doc" )

################################################################################
# Adjust the location of 3rd-party libraries on Windows and Mac OS:
# - On Windows, we need to add the locations of all non-standard 3rd-party shared
#   libraries to the %PATH% variable in order to execute built binaries (e.g.,
#   Python externals or unit tests).
# - On Mac OS this is also necessary (depending on the RPATH setting used while
#   building). In addition, we need to adjust the rpath settings of the 3rd-party
#   libraries itself, which implies that they need to be copied.

# Define the location of the 3rd-party runtime libraries (Shared libraries or DLLs)
set( VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR ${CMAKE_BINARY_DIR}/3rd/ )

# NOTE: This must happen before the add_subdirectory( src ) directive.
if( (VISR_SYSTEM_NAME MATCHES "MacOS") OR (VISR_SYSTEM_NAME MATCHES "Windows") )
  include( Adjust3rdPartyLibraries )
endif( (VISR_SYSTEM_NAME MATCHES "MacOS") OR (VISR_SYSTEM_NAME MATCHES "Windows") )

################################################################################
# Actual software components (subprojects) to be built are
# contained in the src/ directory
add_subdirectory( src )

option(BUILD_DOCUMENTATION "Build the API and user documentation (requires Sphinx, breathe, and Doxygen)" ON)
if(BUILD_DOCUMENTATION)
  add_subdirectory(doc)
endif()

# Instructions for packaging are held in a separate file.
include( PackageConfig )
