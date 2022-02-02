# Copyright Institute of Sound and Vibration Research - All rights reserved

SET( SOURCES
core_renderer.cpp
baseline_renderer.cpp
bunch_renderer.cpp
delay_vector.cpp
gain_matrix.cpp
time_frequency_feedthrough.cpp
)

# Basically, this makes the headers show up in the Visual studio project.
SET( HEADERS
baseline_renderer.hpp
bunch_renderer.hpp
core_renderer.hpp
delay_vector.hpp
export_symbols.hpp
gain_matrix.hpp
time_frequency_feedthrough.hpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( signalflows_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( signalflows_static PROPERTIES OUTPUT_NAME signalflows )
  target_compile_definitions( signalflows_static PUBLIC VISR_SIGNALFLOWS_STATIC_LIBRARY=1)
  set_target_properties( signalflows_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( signalflows_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( signalflows_shared PROPERTIES OUTPUT_NAME signalflows )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( signalflows_shared PROPERTIES ARCHIVE_OUTPUT_NAME signalflows_shared )
  set_target_properties( signalflows_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( signalflows_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( signalflows_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( signalflows_static_pic PROPERTIES OUTPUT_NAME signalflows_pic )
  set_target_properties( signalflows_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( signalflows_static_pic PUBLIC VISR_SIGNALFLOWS_STATIC_LIBRARY=1)
  set_target_properties( signalflows_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES})
  target_compile_definitions( signalflows_${LIB_TYPE} PRIVATE VISR_BUILD_SIGNALFLOWS_LIBRARY=1)
  target_link_libraries( signalflows_${LIB_TYPE} PUBLIC rcl_${LIB_TYPE} )
  target_link_libraries( signalflows_${LIB_TYPE} PUBLIC reverbobject_${LIB_TYPE} )
  target_link_libraries( signalflows_${LIB_TYPE} PUBLIC pml_${LIB_TYPE} )
  target_link_libraries( signalflows_${LIB_TYPE} PRIVATE Boost::system )
  if( NOT Boost_USE_STATIC_LIBS )
    target_compile_definitions( signalflows_${LIB_TYPE} PUBLIC -DBOOST_ALL_DYN_LINK )
  endif( NOT Boost_USE_STATIC_LIBS )
  target_link_libraries( signalflows_${LIB_TYPE} PUBLIC Boost::disable_autolinking )
  # Set public headers to be installed.
  set_target_properties( signalflows_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(signalflows_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS signalflows_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libsignalflows COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS signalflows_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libsignalflows COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS signalflows_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libsignalflows COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
