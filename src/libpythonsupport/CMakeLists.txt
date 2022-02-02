# Copyright Institute of Sound and Vibration Research - All rights reserved

SET( SOURCES
gil_ensure_guard.cpp
initialisation_guard.cpp
load_module.cpp
)

# Basically, this makes the files show up in the Visual studio project.
SET( HEADERS
export_symbols.hpp
gil_ensure_guard.hpp
initialisation_guard.hpp
load_module.hpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pythonsupport_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( pythonsupport_static PROPERTIES OUTPUT_NAME pythonsupport )
  target_compile_definitions( pythonsupport_static PUBLIC VISR_PYTHONSUPPORT_STATIC_LIBRARY=1 )
  set_target_properties( pythonsupport_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pythonsupport_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( pythonsupport_shared PROPERTIES OUTPUT_NAME pythonsupport )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( pythonsupport_shared PROPERTIES ARCHIVE_OUTPUT_NAME pythonsupport_shared )
  set_target_properties( pythonsupport_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( pythonsupport_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pythonsupport_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( pythonsupport_static_pic PROPERTIES OUTPUT_NAME pythonsupport_pic )
  set_target_properties( pythonsupport_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( pythonsupport_static_pic PUBLIC VISR_PYTHONSUPPORT_STATIC_LIBRARY=1 )
  set_target_properties( pythonsupport_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common for all library types.
foreach( LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( pythonsupport_${LIB_TYPE} PRIVATE VISR_BUILD_PYTHONSUPPORT_LIBRARY=1)

  # Transitive dependencies
  target_link_libraries( pythonsupport_${LIB_TYPE} PRIVATE pybind11::embed )
  target_link_libraries( pythonsupport_${LIB_TYPE} PRIVATE Boost::filesystem )
  if( NOT Boost_USE_STATIC_LIBS )
    target_compile_definitions( pythonsupport_${LIB_TYPE} PRIVATE -DBOOST_ALL_DYN_LINK )
  endif( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions( pythonsupport_${LIB_TYPE} PRIVATE -DBOOST_ALL_NO_LIB )

  # Export the Python version as a compile definition.
  # This can be extracted by other projects importing this target.
  target_compile_definitions( pythonsupport_${LIB_TYPE} INTERFACE
    VISR_PYTHON_VERSION=${VISR_PYTHON_VERSION_MAJOR_MINOR} )

  # Encode the pybind11 version as a target property.
  # In this way dependent projects can check whether they build against
  # a compatible version.
  target_compile_definitions( pythonsupport_${LIB_TYPE} INTERFACE
    VISR_PYBIND11_VERSION=${VISR_PYBIND11_VERSION} )

  # Export the location of the pybind11 directory tree.
  # Dependent projects can retrieve this value to build against the
  # same pybind11 version.
  # For installed versions of VISR, this points to $INSTALL_DIRECTORY/3rd/pybind11,
  # where the relevant parts of pybind11 are shipped (see PackageConfig.cmake).
  target_compile_definitions( pythonsupport_${LIB_TYPE} INTERFACE
    PYBIND11_DIR=$<BUILD_INTERFACE:${PYBIND11_DIR}>$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${VISR_TOPLEVEL_INSTALL_DIRECTORY}/3rd/include/pybind11> )

  # Set public headers to be installed.
  set_target_properties( pythonsupport_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(pythonsupport_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS pythonsupport_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpythonsupport COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS pythonsupport_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libpythonsupport COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS pythonsupport_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpythonsupport COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
