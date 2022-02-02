# Python bindings for the main VISR interface classes

set( SOURCES
atomic_component.cpp
audio_connection.cpp
audio_port.cpp
audio_sample_type.cpp
channel_list.cpp
communication_protocol.cpp
communication_protocol_factory.cpp
component.cpp
composite_component.cpp
parameter_base.cpp
parameter_config_base.cpp
parameter_connection.cpp
parameter_factory.cpp
parameter_port.cpp
port.cpp
signal_flow_context.cpp
time.cpp
visr.cpp
)

set( HEADERS
)

set(PROJECT_NAME visrpython)

set(MODULE_NAME visr )

pybind11_add_module(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES} )

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME} )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/python" )

# In case of Windows, we need to make sure that the import library part does not conflict
# with a static library of the same name.
# So we leave it in the build directory and add an _import suffix.
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_NAME "${PROJECT_NAME}_import" )

target_link_libraries( ${PROJECT_NAME} PRIVATE visr_shared )

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER python_externals )

# Note: We export the target to enable dependent projects to query target properties
# such as the library location.
install(TARGETS ${PROJECT_NAME}
        EXPORT VISR_python_externals_exports
        DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
        COMPONENT python_externals )
