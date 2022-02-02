# Copyright Institute of Sound and Vibration Research - All rights reserved

set( SOURCES options.cpp )
set( HEADERS export_symbols.hpp options.hpp )

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( apputilities_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( apputilities_static PROPERTIES OUTPUT_NAME apputilities )
  target_compile_definitions( apputilities_static PUBLIC VISR_APPUTILITIES_STATIC_LIBRARY=1 )
  set_target_properties( apputilities_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( apputilities_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( apputilities_shared PROPERTIES OUTPUT_NAME apputilities )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( apputilities_shared PROPERTIES ARCHIVE_OUTPUT_NAME apputilities_shared )
  set_target_properties( apputilities_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( apputilities_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( apputilities_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( apputilities_static_pic PROPERTIES OUTPUT_NAME apputilities_pic )
  set_target_properties( apputilities_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( apputilities_static_pic PUBLIC VISR_APPUTILITIES_STATIC_LIBRARY=1 )
  set_target_properties( apputilities_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES})
  target_compile_definitions( apputilities_${LIB_TYPE} PRIVATE VISR_BUILD_APPUTILITIES_LIBRARY=1)
  target_link_libraries( apputilities_${LIB_TYPE} PUBLIC Boost::program_options )
  target_link_libraries( apputilities_${LIB_TYPE} PRIVATE Boost::filesystem )
  if( NOT Boost_USE_STATIC_LIBS )
    target_compile_definitions( apputilities_${LIB_TYPE} PUBLIC -DBOOST_ALL_DYN_LINK )
  endif( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions( apputilities_${LIB_TYPE} PUBLIC -DBOOST_ALL_NO_LIB )
  # Set public headers to be installed.
  set_target_properties(apputilities_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(apputilities_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS apputilities_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libapputilities COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries)
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS apputilities_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libapputilities COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS apputilities_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libapputilities COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
