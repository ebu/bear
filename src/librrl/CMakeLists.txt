# Copyright Institute of Sound and Vibration Research - All rights reserved

SET( SOURCES
audio_connection_map.cpp
audio_signal_flow.cpp
communication_area.cpp
integrity_checking.cpp
flexible_buffer_wrapper.cpp
parameter_connection_graph.cpp
parameter_connection_map.cpp
port_utilities.cpp
scheduling_graph.cpp
signal_routing_internal.cpp
)

SET( PUBLIC_HEADERS
audio_signal_flow.hpp
export_symbols.hpp
flexible_buffer_wrapper.hpp
integrity_checking.hpp
)

SET( INTERNAL_HEADERS
audio_connection_map.hpp
communication_area.hpp
parameter_connection_graph.hpp
parameter_connection_map.hpp
port_utilities.hpp
scheduling_graph.hpp
signal_routing_internal.hpp
)

option( BUILD_RUNTIME_SYSTEM_PROFILING "Enable optional measurement of runtime statistics." OFF )

if( BUILD_RUNTIME_SYSTEM_PROFILING )
  list( APPEND SOURCES runtime_profiler.cpp )
  list( APPEND PUBLIC_HEADERS runtime_profiler.hpp )
endif( BUILD_RUNTIME_SYSTEM_PROFILING )

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( rrl_static STATIC ${SOURCES} ${PUBLIC_HEADERS} ${INTERNAL_HEADERS} )
  set_target_properties( rrl_static PROPERTIES OUTPUT_NAME rrl )
  target_compile_definitions( rrl_static PUBLIC VISR_RRL_STATIC_LIBRARY=1)
  set_target_properties( rrl_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( rrl_shared SHARED ${SOURCES} ${PUBLIC_HEADERS} ${INTERNAL_HEADERS} )
  set_target_properties( rrl_shared PROPERTIES OUTPUT_NAME rrl )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( rrl_shared PROPERTIES ARCHIVE_OUTPUT_NAME rrl_shared )
  set_target_properties( rrl_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( rrl_static_pic STATIC ${SOURCES} ${PUBLIC_HEADERS} ${INTERNAL_HEADERS} )
  set_target_properties( rrl_static_pic PROPERTIES OUTPUT_NAME rrl_pic )
  set_target_properties( rrl_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( rrl_static_pic PUBLIC VISR_RRL_STATIC_LIBRARY=1)
  set_target_properties( rrl_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( rrl_${LIB_TYPE} PRIVATE VISR_BUILD_RRL_LIBRARY=1)
  if( BUILD_RUNTIME_SYSTEM_PROFILING )
    target_compile_definitions( rrl_${LIB_TYPE} PUBLIC
      VISR_RRL_RUNTIME_SYSTEM_PROFILING=1)
  endif( BUILD_RUNTIME_SYSTEM_PROFILING )
  # Transitive link dependencies
  target_link_libraries( rrl_${LIB_TYPE} PUBLIC visr_${LIB_TYPE} )
  target_link_libraries( rrl_${LIB_TYPE} PUBLIC efl_${LIB_TYPE} )
  target_link_libraries( rrl_${LIB_TYPE} PRIVATE rbbl_${LIB_TYPE} )
  target_link_libraries( rrl_${LIB_TYPE} PRIVATE Boost::boost ) # Adds the boost include directory
  # Set public headers to be installed.
  set_target_properties(rrl_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(rrl_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS rrl_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/librrl COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS rrl_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/librrl COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS rrl_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/librrl COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
