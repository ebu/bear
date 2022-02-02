# Copyright Institute of Sound and Vibration Research - All rights reserved

set( SOURCES
 array_configuration.cpp
 biquad_parameter.cpp
 double_buffering_protocol.cpp
 empty_parameter_config.cpp
 filter_routing_parameter.cpp
 indexed_value_parameter.cpp
 initialise_parameter_library.cpp
 interpolation_parameter.cpp
 listener_position.cpp
 matrix_parameter.cpp
 matrix_parameter_config.cpp
 message_queue_protocol.cpp
 object_vector.cpp
 scalar_parameter.cpp
 shared_data_protocol.cpp
 signal_routing_parameter.cpp
 sparse_gain_routing_parameter.cpp
 string_parameter.cpp
 time_frequency_parameter.cpp
 time_frequency_parameter_config.cpp
 vector_parameter.cpp
 vector_parameter_config.cpp
)

# Basically, this makes the headers show up in the Visual studio project.
set( HEADERS
 array_configuration.hpp
 biquad_parameter.hpp
 double_buffering_protocol.hpp
 empty_parameter_config.hpp
 export_symbols.hpp
 filter_routing_parameter.hpp
 indexed_value_parameter.hpp
 initialise_parameter_library.hpp
 interpolation_parameter.hpp
 listener_position.hpp
 matrix_parameter.hpp
 matrix_parameter_config.hpp
 message_queue_protocol.hpp
 object_vector.hpp
 scalar_parameter.hpp
 shared_data_protocol.hpp
 signal_routing_parameter.hpp
 sparse_gain_routing_parameter.hpp
 string_parameter.hpp
 time_frequency_parameter.hpp
 time_frequency_parameter_config.hpp
 vector_parameter.hpp
 vector_parameter_config.hpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pml_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( pml_static PROPERTIES OUTPUT_NAME pml )
  target_compile_definitions( pml_static PUBLIC VISR_PML_STATIC_LIBRARY=1)
  set_target_properties( pml_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pml_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( pml_shared PROPERTIES OUTPUT_NAME pml )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( pml_shared PROPERTIES ARCHIVE_OUTPUT_NAME pml_shared )
  set_target_properties( pml_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( pml_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( pml_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( pml_static_pic PROPERTIES OUTPUT_NAME pml_pic )
  set_target_properties( pml_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( pml_static_pic PUBLIC VISR_PML_STATIC_LIBRARY=1)
  set_target_properties( pml_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( pml_${LIB_TYPE} PRIVATE VISR_BUILD_PML_LIBRARY=1 )
  target_link_libraries( pml_${LIB_TYPE} PUBLIC objectmodel_${LIB_TYPE} )
  target_link_libraries( pml_${LIB_TYPE} PUBLIC visr_${LIB_TYPE} )
  if( BUILD_USE_SNDFILE_LIBRARY )
    target_compile_definitions( pml_${LIB_TYPE} PUBLIC VISR_PML_USE_SNDFILE_LIBRARY=1 )
    target_link_libraries( pml_${LIB_TYPE} PRIVATE SndFile::sndfile )
  endif( BUILD_USE_SNDFILE_LIBRARY )
  target_link_libraries( pml_${LIB_TYPE} PUBLIC Boost::boost )
  target_link_libraries( pml_${LIB_TYPE} PRIVATE Boost::filesystem )
  if( NOT Boost_USE_STATIC_LIBS )
    target_compile_definitions( pml_${LIB_TYPE} PRIVATE -DBOOST_ALL_DYN_LINK )
  endif( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions( pml_${LIB_TYPE} PRIVATE -DBOOST_ALL_NO_LIB )
  # Set public headers to be installed.
  set_target_properties(pml_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(pml_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS pml_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpml COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS pml_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib  COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libpml COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS pml_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libpml COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )


if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
