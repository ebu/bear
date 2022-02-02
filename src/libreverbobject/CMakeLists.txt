# Copyright Institute of Sound and Vibration Research - All rights reserved

SET( SOURCES
late_reverb_filter_calculator.cpp
late_reverb_parameter.cpp
reverb_object_renderer.cpp
reverb_parameter_calculator.cpp
)

# Basically, this makes the files show up in the Visual studio project.
SET( HEADERS
export_symbols.hpp
late_reverb_filter_calculator.hpp
late_reverb_parameter.hpp
reverb_object_renderer.hpp
reverb_parameter_calculator.hpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( reverbobject_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( reverbobject_static PROPERTIES OUTPUT_NAME reverbobject )
  target_compile_definitions( reverbobject_static PUBLIC VISR_REVERBOBJECT_STATIC_LIBRARY=1)
  set_target_properties( reverbobject_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( reverbobject_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( reverbobject_shared PROPERTIES OUTPUT_NAME reverbobject )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( reverbobject_shared PROPERTIES ARCHIVE_OUTPUT_NAME reverbobject_shared )
  set_target_properties( reverbobject_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( reverbobject_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( reverbobject_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( reverbobject_static_pic PROPERTIES OUTPUT_NAME reverbobject_pic )
  set_target_properties( reverbobject_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( reverbobject_static_pic PUBLIC VISR_REVERBOBJECT_STATIC_LIBRARY=1)
  set_target_properties( reverbobject_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( reverbobject_${LIB_TYPE} PRIVATE VISR_BUILD_REVERBOBJECT_LIBRARY=1)
  # Transitive link dependencies
  target_link_libraries( reverbobject_${LIB_TYPE} PUBLIC pml_${LIB_TYPE} )
  target_link_libraries( reverbobject_${LIB_TYPE} PUBLIC rcl_${LIB_TYPE} )
  target_link_libraries( reverbobject_${LIB_TYPE} PUBLIC objectmodel_${LIB_TYPE} )
  target_link_libraries( reverbobject_${LIB_TYPE} PUBLIC efl_${LIB_TYPE} )
  target_link_libraries( reverbobject_${LIB_TYPE} PRIVATE Boost::filesystem )
  target_link_libraries( reverbobject_${LIB_TYPE} PRIVATE Boost::boost ) # Header-only libraries (e.g., boost::random)
  if( NOT Boost_USE_STATIC_LIBS )
    target_compile_definitions( reverbobject_${LIB_TYPE} PRIVATE BOOST_ALL_DYN_LINK )
  endif( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions( reverbobject_${LIB_TYPE} PRIVATE BOOST_ALL_NO_LIB )
  # Set public headers to be installed.
  set_target_properties( reverbobject_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(reverbobject_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS reverbobject_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libreverbobject COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS reverbobject_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libreverbobject COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS reverbobject_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libreverbobject COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
