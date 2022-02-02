# Copyright Institute of Sound and Vibration Research - All rights reserved

IF( APPLE )
  FIND_LIBRARY( CORE_SERVICE_FRAMEWORK NAMES CoreServices )
  INCLUDE_DIRECTORIES( ${CORE_SERVICE_FRAMEWORK}/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers )
ENDIF( APPLE )

SET( SOURCES
argument_list.cpp
external_base.cpp
signal_flow_wrapper.cpp
)

SET( HEADERS
argument_list.hpp
class_registrar.hpp
export_symbols.hpp
external_base.hpp
external_wrapper.hpp
signal_flow_wrapper.hpp)

#INCLUDE_DIRECTORIES( ${MAXSDK_MSP_INCLUDE_DIR} )
#INCLUDE_DIRECTORIES( ${MAXSDK_MAX_INCLUDE_DIR} )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( maxsupport_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( maxsupport_static_pic PROPERTIES FOLDER static_pic_libraries )
  set_target_properties( maxsupport_static_pic PROPERTIES OUTPUT_NAME maxsupport )

  set_target_properties( maxsupport_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)

  install( TARGETS maxsupport_static_pic
           ARCHIVE DESTINATION staticlib COMPONENT development_files
         )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( maxsupport_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( maxsupport_shared PROPERTIES FOLDER shared_libraries )

  set_target_properties( maxsupport_shared PROPERTIES OUTPUT_NAME maxsupport )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( maxsupport_shared PROPERTIES ARCHIVE_OUTPUT_NAME maxsupport_shared)
  target_compile_definitions( maxsupport_shared PRIVATE -DVISR_BUILD_MAXSUPPORT_SHARED_LIBRARY=1 -DVISR_BUILD_MAXSUPPORT_LIBRARY=1)
  target_compile_definitions( maxsupport_shared PUBLIC -DVISR_BUILD_MAXSUPPORT_SHARED_LIBRARY=1)
  set_target_properties( maxsupport_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )

  install( TARGETS maxsupport_shared
           ARCHIVE DESTINATION lib COMPONENT max_externals
           RUNTIME DESTINATION lib COMPONENT max_externals
           LIBRARY DESTINATION lib COMPONENT max_externals
           PUBLIC_HEADER DESTINATION include/visr/libmaxsupport COMPONENT max_externals )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Don't build the static librrary variant, because PIC (position
# independent code) support is invariably required for MAX/MSP
# externals (which are shared modules).
set( MAXSUPPORT_BUILD_LIBRARY_TYPES ${VISR_BUILD_LIBRARY_TYPES} )
list( REMOVE_ITEM MAXSUPPORT_BUILD_LIBRARY_TYPES "static" )
foreach( LIB_TYPE ${MAXSUPPORT_BUILD_LIBRARY_TYPES} )
  target_include_directories( maxsupport_${LIB_TYPE} PUBLIC ${MAXSDK_MAX_INCLUDE_DIR} )
  target_include_directories( maxsupport_${LIB_TYPE} PUBLIC ${MAXSDK_MSP_INCLUDE_DIR} )
  # Transitive dependencies
  target_link_libraries( maxsupport_${LIB_TYPE} PUBLIC efl_${LIB_TYPE} )
  target_link_libraries( maxsupport_${LIB_TYPE} PUBLIC rrl_${LIB_TYPE} )
  target_link_libraries( maxsupport_${LIB_TYPE} PUBLIC ${MAXSDK_AUDIO_LIBRARY} )
  target_link_libraries( maxsupport_${LIB_TYPE} PUBLIC ${MAXSDK_API_LIBRARY} )
  target_link_libraries( maxsupport_${LIB_TYPE} PRIVATE Boost::boost )
  set_target_properties( maxsupport_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
endforeach()
