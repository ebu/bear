# Copyright Institute of Sound and Vibration Research - All rights reserved

set( SOURCES
audio_interface.cpp
audio_interface_factory.cpp
)

set( HEADERS
audio_interface.hpp
audio_interface_factory.hpp
export_symbols.hpp
)

if( BUILD_AUDIOINTERFACES_PORTAUDIO )
  list( APPEND HEADERS portaudio_interface.hpp )
  list( APPEND SOURCES portaudio_interface.cpp )
endif( BUILD_AUDIOINTERFACES_PORTAUDIO )

if( BUILD_AUDIOINTERFACES_JACK )
  list( APPEND HEADERS jack_interface.hpp )
  list( APPEND SOURCES jack_interface.cpp )
endif( BUILD_AUDIOINTERFACES_JACK )

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( audiointerfaces_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( audiointerfaces_static PROPERTIES OUTPUT_NAME audiointerfaces )
  target_compile_definitions( audiointerfaces_static PUBLIC VISR_AUDIOINTERFACES_STATIC_LIBRARY=1 )
  set_target_properties( audiointerfaces_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( audiointerfaces_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( audiointerfaces_shared PROPERTIES OUTPUT_NAME audiointerfaces )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( audiointerfaces_shared PROPERTIES ARCHIVE_OUTPUT_NAME audiointerfaces_shared )
  set_target_properties( audiointerfaces_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( audiointerfaces_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( audiointerfaces_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( audiointerfaces_static_pic PROPERTIES OUTPUT_NAME audiointerfaces_pic )
  set_target_properties( audiointerfaces_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( audiointerfaces_static_pic PUBLIC VISR_AUDIOINTERFACES_STATIC_LIBRARY=1 )
  set_target_properties( audiointerfaces_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( audiointerfaces_${LIB_TYPE} PRIVATE VISR_BUILD_AUDIOINTERFACES_LIBRARY=1)
  target_link_libraries( audiointerfaces_${LIB_TYPE} PRIVATE rrl_${LIB_TYPE} )
  target_link_libraries( audiointerfaces_${LIB_TYPE} PRIVATE Boost::boost ) # Set the boost include directory.
  if( BUILD_AUDIOINTERFACES_PORTAUDIO )
    target_link_libraries( audiointerfaces_${LIB_TYPE} PRIVATE Portaudio::portaudio )
    target_compile_definitions( audiointerfaces_${LIB_TYPE} PUBLIC -DVISR_AUDIOINTERFACES_PORTAUDIO_SUPPORT )
  endif( BUILD_AUDIOINTERFACES_PORTAUDIO )
  if( BUILD_AUDIOINTERFACES_JACK )
    target_include_directories( audiointerfaces_${LIB_TYPE} PRIVATE ${JACK_INCLUDE_DIR} )
    target_link_libraries( audiointerfaces_${LIB_TYPE} PRIVATE ${JACK_LIBRARY} )
    target_link_libraries( audiointerfaces_${LIB_TYPE} PRIVATE rbbl_${LIB_TYPE} )
    target_compile_definitions( audiointerfaces_${LIB_TYPE} PUBLIC -DVISR_AUDIOINTERFACES_JACK_SUPPORT )
  endif( BUILD_AUDIOINTERFACES_JACK )
  # Set public headers to be installed.
  set_target_properties( audiointerfaces_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories( audiointerfaces_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS audiointerfaces_static 
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libaudiointerfaces COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS audiointerfaces_shared
          COMPONENT shared_libraries
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/libaudiointerfaces COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS audiointerfaces_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libaudiointerfaces COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
