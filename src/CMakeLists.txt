# Copyright Institute of Sound and Vibration Research - All rights reserved

# Set the installation directory for the export and version files
if( VISR_SYSTEM_NAME MATCHES "Linux" )
  # On  Linux, there's a predefined directory structure under /usr/lib
  set( VISR_INSTALL_EXPORT_CONFIG_DIRECTORY lib/cmake/visr )
else( VISR_SYSTEM_NAME MATCHES "Linux" )
  # On Windows and MacOS, install it in a cmake/ directory within the application's install directory,
  # e.g. c:\Program Files\VISR-X.X.X/cmake or /Applications/VISR-X.X.X/cmake
  set( VISR_INSTALL_EXPORT_CONFIG_DIRECTORY cmake )
endif( VISR_SYSTEM_NAME MATCHES "Linux" )

# Location of export files in build tree
set( VISR_BUILD_EXPORT_CONFIG_DIRECTORY ${CMAKE_BINARY_DIR}/cmake )

# Build configuration for the top-level src/ directory

# Define a list of all built libraries (to be used, e.g., in the export config)
set( VISR_LIBRARIES )

# Common functionalties for realtime applications
add_subdirectory( libapputilities )
list( APPEND VISR_LIBRARIES apputilities )

# Audio interfaces library
add_subdirectory( libaudiointerfaces )
list( APPEND VISR_LIBRARIES audiointerfaces )

# Elementary functions library
add_subdirectory( libefl )
list( APPEND VISR_LIBRARIES efl )

# Object model library.
add_subdirectory( libobjectmodel )
list( APPEND VISR_LIBRARIES objectmodel )

# Panning library, compute a panning gains for a loudspeaker configuration
add_subdirectory( libpanning )
list( APPEND VISR_LIBRARIES panning )

# Signal processing components and data types related to panning.
add_subdirectory( libpanningdsp )
list( APPEND VISR_LIBRARIES panningdsp )

# Parameter message library
add_subdirectory( libpml )
list( APPEND VISR_LIBRARIES pml )

# Python support for calling Python functionality from C++
if( BUILD_PYTHON_BINDINGS )
  add_subdirectory( libpythonsupport )
  list( APPEND VISR_LIBRARIES pythonsupport )
endif( BUILD_PYTHON_BINDINGS )

# Library for signal flows.
add_subdirectory( libsignalflows )
list( APPEND VISR_LIBRARIES signalflows )

# Library for signal flows that use Python.
# Temporary solution to avoid a link dependency on Python
# for the majority of signal flows.
# Note that this library is also build if BUILD_PYTHON_BINDINGS is unselected.
# In this case, it contains a reduced functionality that does not use Python.
add_subdirectory( libsignalflowspython )
list( APPEND VISR_LIBRARIES signalflowspython )


# Rendering building block library
add_subdirectory( librbbl )
list( APPEND VISR_LIBRARIES rbbl )

# Rendering component library
add_subdirectory( librcl )
list( APPEND VISR_LIBRARIES rcl )

# Rendering runtime library
add_subdirectory( librrl )
list( APPEND VISR_LIBRARIES rrl )

# Data types and components related to the reverb audio object.
add_subdirectory( libreverbobject )
list( APPEND VISR_LIBRARIES reverbobject )

# VISR main library
add_subdirectory( libvisr )
list( APPEND VISR_LIBRARIES visr )

if( BUILD_STANDALONE_APPLICATIONS )
  # Hold applications in a separate subdirectory
  add_subdirectory( apps )
endif( BUILD_STANDALONE_APPLICATIONS )

# Subdirectory for Matlab Mex externals
IF( BUILD_MATLAB_EXTERNALS )
  # Library containing general support classes and functions for Matlab Mex externals
  add_subdirectory( libmexsupport )
  list( APPEND VISR_LIBRARIES mexsupport )

  # Directory containing the Mex externals itself
  add_subdirectory( mex )
ENDIF(BUILD_MATLAB_EXTERNALS)

IF( BUILD_MAX_MSP_EXTERNALS )
  add_subdirectory( maxmspexternals )
ENDIF( BUILD_MAX_MSP_EXTERNALS )

# Python bindings for the rendering framework
if( BUILD_PYTHON_BINDINGS )
  add_subdirectory( python )
  add_subdirectory( libpythoncomponents )
  list( APPEND VISR_LIBRARIES pythoncomponents )
endif( BUILD_PYTHON_BINDINGS )

##### CMake packaging

# Define a list for the exported components (will be populated below).
# This will be written into the visr-config.cmake file.
set( VISR_EXPORTED_COMPONENTS )

if( BUILD_INSTALL_STATIC_LIBRARIES OR BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  ## Component for 3rd-static components provided by VISR
  # These are needed to link dependent projects against static VISR libraries.
  export(EXPORT VISR_3rd_party_static_exports
        NAMESPACE VISR::
        FILE ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr_3rd_party_static_targets.cmake
  )
  install(
      EXPORT VISR_3rd_party_static_exports
      FILE visr_3rd_party_static_targets.cmake
      NAMESPACE VISR::
      DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY}
      COMPONENT development_files
  )
  list( APPEND VISR_EXPORTED_COMPONENTS 3rd_party_static )

  file( COPY
    ${PROJECT_SOURCE_DIR}/cmake_modules/FindSndFile.cmake
    ${PROJECT_SOURCE_DIR}/cmake_modules/FindPortaudio.cmake
    DESTINATION ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}
  )

  install(
    FILES ${PROJECT_SOURCE_DIR}/cmake_modules/FindSndFile.cmake
    ${PROJECT_SOURCE_DIR}/cmake_modules/FindPortaudio.cmake
    DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY}
    COMPONENT development_files
  )
endif( BUILD_INSTALL_STATIC_LIBRARIES OR BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  ## Component for shared 3rd-party components provided by VISR
  export(EXPORT VISR_3rd_party_shared_exports
        NAMESPACE VISR::
        FILE ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr_3rd_party_shared_targets.cmake
  )
  install(
      EXPORT VISR_3rd_party_shared_exports
      FILE visr_3rd_party_shared_targets.cmake
      NAMESPACE VISR::
      DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY}
      COMPONENT development_files
  )
  list( APPEND VISR_EXPORTED_COMPONENTS 3rd_party_shared )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  # Create component export config in build tree.
  export(EXPORT VISR_shared_library_exports
         NAMESPACE VISR::
         FILE ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr_shared_libraries_targets.cmake
  )
  install(
    EXPORT VISR_shared_library_exports
    FILE visr_shared_libraries_targets.cmake
    NAMESPACE VISR::
    DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY} COMPONENT development_files
  )
  list( APPEND VISR_EXPORTED_COMPONENTS shared_libraries )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_LIBRARIES )
  # Create component export config in build tree.
  export(EXPORT VISR_static_library_exports
         NAMESPACE VISR::
         FILE ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr_static_libraries_targets.cmake
  )
  install(
    EXPORT VISR_static_library_exports
    FILE visr_static_libraries_targets.cmake
    NAMESPACE VISR::
    DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY} COMPONENT development_files
  )
  list( APPEND VISR_EXPORTED_COMPONENTS static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  # Create component export config in build tree.
  export(EXPORT VISR_static_pic_library_exports
         NAMESPACE VISR::
         FILE ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr_static_pic_libraries_targets.cmake
  )
  install(
    EXPORT VISR_static_pic_library_exports
    FILE visr_static_pic_libraries_targets.cmake
    NAMESPACE VISR::
    DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY} COMPONENT development_files
  )
  list( APPEND VISR_EXPORTED_COMPONENTS static_pic_libraries )
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_PYTHON_BINDINGS )
  # Create component export config in build tree.
  export(EXPORT VISR_python_externals_exports
         NAMESPACE VISR::
         FILE ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr_python_externals_targets.cmake
  )
  install(
    EXPORT VISR_python_externals_exports
    FILE visr_python_externals_targets.cmake
    NAMESPACE VISR::
    DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY} COMPONENT development_files
  )
  list( APPEND VISR_EXPORTED_COMPONENTS python_externals )
endif( BUILD_PYTHON_BINDINGS )

# Install package config file
# for use from install tree
install(FILES ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr-config.cmake
  DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY} COMPONENT development_files )

set( VISR_3RD_PARTY_DIR ${CMAKE_BINARY_DIR}/3rd )
 
# Configure and write package config file
include(CMakePackageConfigHelpers)
configure_package_config_file(visr-config.cmake.in
  ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr-config.cmake
  INSTALL_DESTINATION cmake
  PATH_VARS VISR_3RD_PARTY_DIR
  INSTALL_DESTINATION lib
  INSTALL_PREFIX ${CMAKE_BINARY_DIR}
#  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Create package version file
# Assuming major version bump = breaking API changes
write_basic_package_version_file(
  ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr-config-version.cmake
  VERSION ${VISR_MAJOR_VERSION}.${VISR_MINOR_VERSION}.${VISR_PATCH_VERSION}
  COMPATIBILITY SameMajorVersion)

# Install package version file
install(FILES ${VISR_BUILD_EXPORT_CONFIG_DIRECTORY}/visr-config-version.cmake
  DESTINATION ${VISR_INSTALL_EXPORT_CONFIG_DIRECTORY} COMPONENT development_files )

# Register visr package in user registry (if this behaviour activated).
export(PACKAGE visr)
