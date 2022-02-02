# Copyright Institute of Sound and Vibration Research - All rights reserved

set( SOURCES
initialise_parameter_library.cpp
panning_gain_matrix.cpp
panning_matrix_parameter.cpp
)

set( HEADERS
export_symbols.hpp
initialise_parameter_library.hpp
panning_gain_matrix.hpp
panning_matrix_parameter.hpp
types.hpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( panningdsp_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( panningdsp_static PROPERTIES OUTPUT_NAME panningdsp )
  target_compile_definitions( panningdsp_static PUBLIC VISR_PANNINGDSP_STATIC_LIBRARY=1)
  set_target_properties( panningdsp_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( panningdsp_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( panningdsp_shared PROPERTIES OUTPUT_NAME panningdsp )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( panningdsp_shared PROPERTIES ARCHIVE_OUTPUT_NAME panningdsp_shared )
  set_target_properties( panningdsp_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( panningdsp_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( panningdsp_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( panningdsp_static_pic PROPERTIES OUTPUT_NAME panningdsp_pic )
  set_target_properties( panningdsp_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( panningdsp_static_pic PUBLIC VISR_PANNINGDSP_STATIC_LIBRARY=1)
  set_target_properties( panningdsp_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES})
  target_compile_definitions( panningdsp_${LIB_TYPE} PRIVATE VISR_BUILD_PANNINGDSP_LIBRARY=1)
  set_target_properties( panningdsp_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  target_link_libraries( panningdsp_${LIB_TYPE} PUBLIC pml_${LIB_TYPE} )
  target_link_libraries( panningdsp_${LIB_TYPE} PUBLIC visr_${LIB_TYPE} )
  target_link_libraries( panningdsp_${LIB_TYPE} PUBLIC efl_${LIB_TYPE} )
endforeach()


# Set include paths for dependent projects
get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
foreach(TARGET_TYPE ${VISR_BUILD_LIBRARY_TYPES})
  target_include_directories(panningdsp_${TARGET_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS panningdsp_static
          EXPORT VISR_static_library_exports
          ARCHIVE DESTINATION lib COMPONENT static_libraries
          PUBLIC_HEADER DESTINATION include/visr/libpanningdsp COMPONENT development_files
  )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS panningdsp_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libpanningdsp COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS panningdsp_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpanningdsp COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
