# Copyright Institute of Sound and Vibration Research - All rights reserved

SET( SOURCES
visr_renderer.cpp
)

# Basically, this makes the headers show up in the Visual studio project.
SET( HEADERS
export_symbols.hpp
visr_renderer.hpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( signalflowspython_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( signalflowspython_static PROPERTIES OUTPUT_NAME signalflowspython )
  target_compile_definitions( signalflowspython_static PUBLIC VISR_SIGNALFLOWSPYTHON_STATIC_LIBRARY=1)
  set_target_properties( signalflowspython_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( signalflowspython_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( signalflowspython_shared PROPERTIES OUTPUT_NAME signalflowspython )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( signalflowspython_shared PROPERTIES ARCHIVE_OUTPUT_NAME signalflowspython_shared )
  set_target_properties( signalflowspython_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( signalflowspython_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( signalflowspython_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( signalflowspython_static_pic PROPERTIES OUTPUT_NAME signalflowspython_pic )
  set_target_properties( signalflowspython_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( signalflowspython_static_pic PUBLIC VISR_SIGNALFLOWSPYTHON_STATIC_LIBRARY=1)
  set_target_properties( signalflowspython_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( signalflowspython_${LIB_TYPE} PRIVATE VISR_BUILD_SIGNALFLOWSPYTHON_LIBRARY=1)
  target_link_libraries( signalflowspython_${LIB_TYPE} PUBLIC rcl_${LIB_TYPE} )
  target_link_libraries( signalflowspython_${LIB_TYPE} PUBLIC signalflows_${LIB_TYPE} )
  target_link_libraries( signalflowspython_${LIB_TYPE} PUBLIC pml_${LIB_TYPE} )

  # Note: Despite the name, the Python dependency is optional to provide all components (where sensible) with the
  # Python functionality disabled.
  # If Python is activated, enable the optional use of a Metadapter to receive and process object metadata.
  if( BUILD_PYTHON_BINDINGS )
    set_property( SOURCE visr_renderer.cpp APPEND PROPERTY COMPILE_DEFINITIONS VISR_PYTHON_SUPPORT=1 )
    target_link_libraries( signalflowspython_${LIB_TYPE} PRIVATE pythoncomponents_${LIB_TYPE} )
  endif( BUILD_PYTHON_BINDINGS )

  target_link_libraries( signalflowspython_${LIB_TYPE} PRIVATE Boost::system )
  if( NOT Boost_USE_STATIC_LIBS )
    target_compile_definitions( signalflowspython_${LIB_TYPE} PUBLIC -DBOOST_ALL_DYN_LINK )
  endif( NOT Boost_USE_STATIC_LIBS )
  target_link_libraries( signalflowspython_${LIB_TYPE} PUBLIC Boost::disable_autolinking )
  # Set public headers to be installed.
  set_target_properties( signalflowspython_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(signalflowspython_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS signalflowspython_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libsignalflows COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS signalflowspython_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libsignalflows COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS signalflowspython_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libsignalflowspython COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
