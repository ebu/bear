# Copyright Institute of Sound and Vibration Research - All rights reserved

set( SOURCES
wrapper.cpp
)

# Basically, this makes the files show up in the Visual studio project.
set( HEADERS
export_symbols.hpp
wrapper.hpp)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pythoncomponents_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( pythoncomponents_static PROPERTIES OUTPUT_NAME pythoncomponents )
  target_compile_definitions( pythoncomponents_static PUBLIC VISR_PYTHONCOMPONENTS_STATIC_LIBRARY=1 )
  set_target_properties( pythoncomponents_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pythoncomponents_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( pythoncomponents_shared PROPERTIES OUTPUT_NAME pythoncomponents )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( pythoncomponents_shared PROPERTIES ARCHIVE_OUTPUT_NAME pythoncomponents_shared )
  set_target_properties( pythoncomponents_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
    set_target_properties( pythoncomponents_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pythoncomponents_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( pythoncomponents_static_pic PROPERTIES OUTPUT_NAME pythoncomponents_pic )
  set_target_properties( pythoncomponents_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( pythoncomponents_static_pic PUBLIC VISR_PYTHONCOMPONENTS_STATIC_LIBRARY=1 )
  set_target_properties( pythoncomponents_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( pythoncomponents_${LIB_TYPE} PRIVATE VISR_BUILD_PYTHONCOMPONENTS_LIBRARY=1 )
  # Transitive dependencies
  # Note: We always need to link the Pythonsupport shared library as a shared library, because there must be
  # exactly one instance of pythonsupport::InitialisationGuard that is shared by the
  # containing application and the VISR externals instantiated within Python
  target_link_libraries( pythoncomponents_${LIB_TYPE} 
    PRIVATE pythonsupport_${LIB_TYPE} )
  target_link_libraries( pythoncomponents_${LIB_TYPE} PUBLIC visr_${LIB_TYPE} )
  target_link_libraries( pythoncomponents_${LIB_TYPE} PRIVATE pybind11::embed )
  target_link_libraries( pythoncomponents_${LIB_TYPE} PRIVATE Boost::boost )
  # Set public headers to be installed.
  set_target_properties( pythoncomponents_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(pythoncomponents_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS pythoncomponents_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpythoncomponents COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS pythoncomponents_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libpythoncomponents COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS pythoncomponents_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpythoncomponents COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
