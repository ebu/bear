# Copyright Institute of Sound and Vibration Research - All rights reserved

SET( SOURCES
 AllRAD.cpp
 CAP.cpp
 CAP_VBAP.cpp
 LoudspeakerArray.cpp
 VBAP.cpp
 XYZ.cpp
)

# Basically, this makes the files show up in the Visual studio project.
SET( HEADERS
 defs.h
 export_symbols.hpp
 AllRAD.h
 CAP.h
 CAP_VBAP.h
 LoudspeakerArray.h
 VBAP.h
 XYZ.h
)

# Workaround for GCC <= 4.8:
# Because the regex implementation for this 's standard library is completely broken, we switch to GCC.
# Otherwise C++11's regex is preferred because it avoids the external shared library dependency.
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9 )
  set_source_files_properties( LoudspeakerArray.cpp PROPERTIES COMPILE_DEFINITIONS VISR_USE_BOOST_REGEX=1 )
endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9 )

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( panning_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( panning_static PROPERTIES OUTPUT_NAME panning )
  target_compile_definitions( panning_static PUBLIC VISR_PANNING_STATIC_LIBRARY=1)
  set_target_properties( panning_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( panning_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( panning_shared PROPERTIES OUTPUT_NAME panning )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( panning_shared PROPERTIES ARCHIVE_OUTPUT_NAME panning_shared )
  set_target_properties( panning_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( panning_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( panning_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( panning_static_pic PROPERTIES OUTPUT_NAME panning_pic )
  set_target_properties( panning_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( panning_static_pic PUBLIC VISR_PANNING_STATIC_LIBRARY=1)
  set_target_properties( panning_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( panning_${LIB_TYPE} PRIVATE VISR_BUILD_PANNING_LIBRARY=1)
  target_link_libraries( panning_${LIB_TYPE} PUBLIC rbbl_${LIB_TYPE} )
  if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9 )
    target_link_libraries( panning_${LIB_TYPE} PRIVATE Boost::regex )
  endif (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9 )
  target_link_libraries( panning_${LIB_TYPE} PRIVATE Boost::filesystem )
  if( NOT Boost_USE_STATIC_LIBS )
    target_compile_definitions( panning_${LIB_TYPE} PRIVATE -DBOOST_ALL_DYN_LINK )
  endif( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions( panning_${LIB_TYPE} PRIVATE -DBOOST_ALL_NO_LIB )
  # Set public headers to be installed.
  set_target_properties( panning_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
    target_include_directories(panning_${LIB_TYPE}
      PUBLIC
      $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/visr>
    )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS panning_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpanning COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS panning_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libpanning COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS panning_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpanning COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
