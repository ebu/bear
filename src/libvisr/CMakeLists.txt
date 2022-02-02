# Copyright Institute of Sound and Vibration Research - All rights reserved

# Define the header files that should be installed to the directory include/visr/visr.
SET( PUBLIC_HEADERS
atomic_component.hpp
audio_input.hpp
audio_output.hpp
audio_port_base.hpp
audio_sample_type.hpp
channel_list.hpp
communication_protocol_base.hpp
communication_protocol_type.hpp
communication_protocol_factory.hpp
component.hpp
composite_component.hpp
constants.hpp
export_symbols.hpp
parameter_base.hpp
parameter_config_base.hpp
parameter_input.hpp
parameter_factory.hpp
parameter_output.hpp
parameter_port_base.hpp
parameter_type.hpp
polymorphic_parameter_input.hpp
polymorphic_parameter_output.hpp
port_base.hpp
signal_flow_context.hpp
status_message.hpp
time.hpp
typed_parameter_base.hpp
version.hpp
)

SET( PUBLIC_HEADERS_DETAIL
detail/compile_time_hash_fnv1.hpp
detail/compose_message_string.hpp
)

# Header files used internally, not to be installed.
set( INTERNAL_HEADERS
impl/audio_connection_descriptor.hpp
impl/component_implementation.hpp
impl/composite_component_implementation.hpp
impl/parameter_connection_descriptor.hpp
impl/port_base_implementation.hpp
impl/audio_port_base_implementation.hpp
impl/parameter_port_base_implementation.hpp
)

set( SOURCES
atomic_component.cpp
audio_input.cpp
audio_output.cpp
audio_port_base.cpp
audio_sample_type.cpp
channel_list.cpp
communication_protocol_base.cpp
communication_protocol_factory.cpp
component.cpp
composite_component.cpp
parameter_config_base.cpp
parameter_factory.cpp
parameter_input.cpp
parameter_output.cpp
parameter_base.cpp
parameter_port_base.cpp
polymorphic_parameter_input.cpp
polymorphic_parameter_output.cpp
port_base.cpp
signal_flow_context.cpp
time.cpp
version.cpp
impl/audio_connection_descriptor.cpp
impl/audio_port_base_implementation.cpp
impl/component_implementation.cpp
impl/composite_component_implementation.cpp
impl/parameter_connection_descriptor.cpp
impl/parameter_port_base_implementation.cpp
impl/port_base_implementation.cpp
impl/time_implementation.cpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( visr_static STATIC ${SOURCES} ${PUBLIC_HEADERS} ${PUBLIC_HEADERS_DETAIL} ${INTERNAL_HEADERS} )
  set_target_properties( visr_static PROPERTIES OUTPUT_NAME visr)
  set_target_properties(visr_static PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib)
  target_compile_definitions( visr_static PUBLIC -DVISR_CORE_STATIC_LIBRARY=1 )
  set_target_properties( visr_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( visr_shared SHARED ${SOURCES} ${PUBLIC_HEADERS} ${PUBLIC_HEADERS_DETAIL} ${INTERNAL_HEADERS} )
  set_target_properties( visr_shared PROPERTIES OUTPUT_NAME visr)
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( visr_shared PROPERTIES ARCHIVE_OUTPUT_NAME visr_shared )
  # Set this definition only when building the present target.
  set_target_properties(visr_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( visr_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( visr_static_pic STATIC ${SOURCES} ${PUBLIC_HEADERS} ${PUBLIC_HEADERS_DETAIL} ${INTERNAL_HEADERS} )
  set_target_properties( visr_static_pic PROPERTIES OUTPUT_NAME visr_pic)
  set_target_properties(visr_static_pic PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib)
  set_target_properties(visr_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( visr_static_pic PUBLIC -DVISR_CORE_STATIC_LIBRARY=1 )
  set_target_properties( visr_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

foreach( LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( visr_${LIB_TYPE} PRIVATE VISR_BUILD_CORE_LIBRARY=1 )
  # Set public headers to be installed.
  set_target_properties(visr_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}" )
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(
      visr_${LIB_TYPE}
      PUBLIC
      # this will make impl headers visible when linking to build tree
      # Alternative is to copy public headers to binary dir
      # It's nice to be able to edit headers from dependent projects
      # when doing dev (i.e. not working from an installation) so lesser of two evils?
      $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/visr>)
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS visr_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libvisr COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS visr_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libvisr COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS visr_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libvisr COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

# TODO: Try to install the detail/ headers as part of the ordinary PUBLIC_HEADER installation.
if( BUILD_INSTALL_SHARED_LIBRARIES OR BUILD_INSTALL_SHARED_LIBRARIES )
  install( FILES ${PUBLIC_HEADERS_DETAIL} DESTINATION include/visr/libvisr/detail COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES OR BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
