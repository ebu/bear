# Python bindings for the main VISR interface classes

set( SOURCES
audiointerfaces.cpp
audio_interface.cpp
audio_interface_factory.cpp
)

set( HEADERS
)
# We don't need header files


if( BUILD_AUDIOINTERFACES_PORTAUDIO )
  list( APPEND SOURCES portaudio_interface.cpp )
endif( BUILD_AUDIOINTERFACES_PORTAUDIO )

if( BUILD_AUDIOINTERFACES_JACK )
  list( APPEND SOURCES jack_interface.cpp )
endif( BUILD_AUDIOINTERFACES_JACK )

set(PROJECT_NAME audiointerfacespython)

set(MODULE_NAME audiointerfaces)

pybind11_add_module(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES} )

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME} )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/python" )

# In case of Windows, we need to make sure that the import library part does not conflict
# with a static library of the same name.
# So we leave it in the build directory and add an _import suffix.
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_NAME "${PROJECT_NAME}_import" )

target_link_libraries( ${PROJECT_NAME} PRIVATE audiointerfaces_shared )
target_link_libraries( ${PROJECT_NAME} PRIVATE rrl_shared )
target_link_libraries( ${PROJECT_NAME} PRIVATE Boost::boost ) # Adds the boost include directory

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER python_externals )

# Note: We export the target to enable dependent projects to query target properties
# such as the library location.
install(TARGETS ${PROJECT_NAME}
        EXPORT VISR_python_externals_exports
        DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
        COMPONENT python_externals )
