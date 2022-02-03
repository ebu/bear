cmake_minimum_required(VERSION 3.12.0)

project(
  bear
  VERSION 0.1.0
  LANGUAGES CXX)

if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(IS_ROOT_PROJECT TRUE)
else()
  set(IS_ROOT_PROJECT FALSE)
endif()

set(CMAKE_CXX_STANDARD 14)

# Define build configuration options
include(CMakeDependentOption)
option(
  BUILD_PYTHON_BINDINGS
  "Whether to build a Python module to use the defined components from Python"
  ON)
option(BEAR_PACKAGE_AND_INSTALL "Package and install bear" ${IS_ROOT_PROJECT})
option(BEAR_UNIT_TESTS "Build units tests" ${IS_ROOT_PROJECT})

option(BEAR_DOWNLOAD_DATA_DEFAULT "Download the default data file" ON)
option(BEAR_DOWNLOAD_DATA_DEFAULT_SMALL "Download the default small data file"
       OFF)

set(BEAR_DATA_PATH_DEFAULT
    CACHE PATH "path to the default data file if externally provided")
set(BEAR_DATA_PATH_DEFAULT_SMALL
    CACHE PATH "path to the default small data file if externally provided")

cmake_dependent_option(BEAR_DEMO "build demo module" ${IS_ROOT_PROJECT}
                       "BUILD_PYTHON_BINDINGS" OFF)
set(BEAR_VISR_LIB_TYPE
    "shared"
    CACHE STRING "VISR library type to link against")

# Additional CMake functionality in separate files.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

# Create a system string.
set(BEAR_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})

# Mac OS specific settings (mainly rpath handling)
if(BEAR_SYSTEM_NAME MATCHES "Darwin")
  set(BEAR_SYSTEM_NAME MacOS) # Replace CMake's name for MacOS X by a more
                              # descriptive one.

  # Activate rpath support
  set(CMAKE_MACOSX_RPATH ON)
  # Use, i.e. don't skip, rpaths in the build tree
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR TRUE)
  # When building, set the rpath to the libraries in the build tree, not to the
  # install tree
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH
      "${CMAKE_INSTALL_PREFIX}/3rd;${CMAKE_INSTALL_PREFIX}/lib")
  # Add the automatically determined parts of the RPATH which point to
  # directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif(BEAR_SYSTEM_NAME MATCHES "Darwin")

if(WIN32) # Covers 64-bit Windows as well
  # Avoid complaints about STL members in classes with VISR_CORE_LIBRARY_SYMBOL
  # not having DLL interfaces.
  add_definitions("/wd4251")
  # Disable warnings about unsafe STL calls (while calling them with raw
  # pointers is perfectly legal).
  add_definitions("/wd4996")
endif(WIN32)

# Import VISR targets (libraries and include files) - perhaps a flag like
# USE_INTERNAL_LIBEAR would be more consistent?
if(NOT TARGET VISR::visr_${BEAR_VISR_LIB_TYPE})
  message(STATUS "VISR Target not found. Using find_package...")
  find_package(VISR 0.11.0 REQUIRED)
endif()

option(USE_INTERNAL_LIBEAR
       "should we use our own version of libear, or find one with find_package?"
       ON)
if(NOT USE_INTERNAL_LIBEAR)
  find_package(ear REQUIRED)
endif()

find_package(Boost REQUIRED)

find_package(Threads REQUIRED)

add_subdirectory(submodules)
add_subdirectory(src)

add_subdirectory(data)

if(BUILD_PYTHON_BINDINGS)
  add_subdirectory(pythonwrappers)
endif(BUILD_PYTHON_BINDINGS)

if(BEAR_DEMO)
  add_subdirectory(demo)
endif()

if(BEAR_UNIT_TESTS)
  include(CTest)
  add_subdirectory(test)
endif()
