cmake_minimum_required( VERSION 3.3.0 )

project( kissFFT LANGUAGES C )


option( KISSFFT_BUILD_STATIC_LIBRARY "Build KissFFT as static library." ON )
option( KISSFFT_BUILD_SHARED_LIBRARY "Build KissFFT as shared library." ON )

set( SOURCES
  kiss_fft_double.c
  kiss_fft_float.c
  kiss_fftr_double.c
  kiss_fftr_float.c
)

set( PUBLIC_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fft.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fft_double.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fft_double_redefines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fft_float.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fft_float_redefines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fftr_double.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fftr_double_redefines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fftr_float.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kiss_fftr_float_redefines.h
)

if( KISSFFT_BUILD_STATIC_LIBRARY )
  add_library( kissfft_static STATIC ${SOURCES} )
  # Override global visibility preset to avoid warnings.
  # This is necessary because the global setting for policy CMP0063 is not heeded
  # in add_subdirectory() subprojects.
  set_property( TARGET kissfft_static PROPERTY C_VISIBILITY_PRESET )

  # This is necessary because files in tools/ include headers in the main
  # directory using #include "" without a relative path.
  target_include_directories( kissfft_static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  # Set position-independent code for the static library to make it usable in shared modules.
  if( WIN32 )
    # Avoid warnings about potentially lossy conversions at 'pedantic'
    # warning level.
    target_compile_options( kissfft_static PRIVATE /wd4244 /wd4267 )
  endif( WIN32 )
  set_property( TARGET kissfft_static PROPERTY POSITION_INDEPENDENT_CODE ON)
  set_property( TARGET kissfft_static PROPERTY PUBLIC_HEADER "${PUBLIC_HEADERS}" )
  set_property( TARGET kissfft_static PROPERTY ARCHIVE_OUTPUT_NAME kissfft )
endif( KISSFFT_BUILD_STATIC_LIBRARY )

if( KISSFFT_BUILD_SHARED_LIBRARY )
  add_library( kissfft_shared SHARED ${SOURCES} )
  # This is necessary because files in tools/ include headers in the main
  # directory using #include "" without a relative path.
  target_include_directories( kissfft_shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  if( WIN32 )
    # Avoid warnings about potentially lossy conversions at 'pedantic'
    # warning level.
    target_compile_options( kissfft_shared PRIVATE /wd4244 /wd4267 )
  endif( WIN32 )
  set_property( TARGET kissfft_shared PROPERTY PUBLIC_HEADER "${PUBLIC_HEADERS}" )
  # Export all shared library (because we don't have sophisticated per-symbol
  # declarations)
  set_property( TARGET kissfft_shared PROPERTY C_VISIBILITY_PRESET default )
  set_property( TARGET kissfft_shared PROPERTY LIBRARY_OUTPUT_NAME kissfft )
  set_property( TARGET kissfft_shared PROPERTY ARCHIVE_OUTPUT_NAME kissfft_shared )
endif( KISSFFT_BUILD_SHARED_LIBRARY )

# =============================================================================
# Installing / exporting kissfft targets as part of VISR.
# This part of the file is solely for including kissfft as a git subtree of the
# VISR repository.
# It is necessary to do this in the kissfft CMakeLists.txt file, because before
# CMake 3.13, install() commands can only be used for targets created in the same
# directory.
# DO NOT MERGE THIS PART INTO THE UPSTREAM KISSFFT REPOSITORY!

if( KISSFFT_BUILD_STATIC_LIBRARY )
  set_target_properties( kissfft_static PROPERTIES FOLDER 3rd)
  # Overwrite the include_directories
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    DIRECTORY )
  target_include_directories(kissfft_static PUBLIC
      $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}/kissfft>
      $<INSTALL_INTERFACE:include/visr/3rd/kissfft>
  )
  set_target_properties( kissfft_static PROPERTIES
    ARCHIVE_OUTPUT_NAME kissfft
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/3rd
  )
  install(TARGETS kissfft_static
          EXPORT VISR_3rd_party_static_exports
          PUBLIC_HEADER DESTINATION include/visr/3rd/kissfft COMPONENT development_files
          ARCHIVE DESTINATION ${THIRDPARTY_ARCHIVE_INSTALL_DIRECTORY} COMPONENT development_files
          )
endif( KISSFFT_BUILD_STATIC_LIBRARY )

if( KISSFFT_BUILD_SHARED_LIBRARY )
  set_target_properties( kissfft_shared PROPERTIES FOLDER 3rd)
  target_include_directories(kissfft_shared PUBLIC
      $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}/kissfft>
      $<INSTALL_INTERFACE:include/visr/3rd/kissfft>
  )
  set_target_properties( kissfft_shared PROPERTIES
    ARCHIVE_OUTPUT_NAME kissfft_shared # Import library on Windows
    LIBRARY_OUTPUT_NAME kissfft # Shared library on Unix platforms
    RUNTIME_OUTPUT_NAME kissfft # DLL on Windows
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/3rd
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/3rd
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/3rd
  )
  install(TARGETS kissfft_shared
          EXPORT VISR_3rd_party_shared_exports
          PUBLIC_HEADER DESTINATION include/visr/3rd/kissfft COMPONENT development_files
          ARCHIVE DESTINATION ${THIRDPARTY_ARCHIVE_INSTALL_DIRECTORY} COMPONENT development_files
          LIBRARY DESTINATION ${THIRDPARTY_LIBRARY_INSTALL_DIRECTORY} COMPONENT shared_libraries
          RUNTIME DESTINATION ${THIRDPARTY_RUNTIME_INSTALL_DIRECTORY} COMPONENT shared_libraries
          )
endif( KISSFFT_BUILD_SHARED_LIBRARY )
