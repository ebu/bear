# Python bindings for the main VISR interface classes

set( SOURCES
late_reverb_filter_calculator.cpp
late_reverb_parameter.cpp
reverb_object_renderer.cpp
reverb_parameter_calculator.cpp
reverbobject.cpp
)

set( HEADERS
# No headers yet.
)

set(PROJECT_NAME reverbobjectpython)

set(MODULE_NAME reverbobject)

pybind11_add_module(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES} )

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME} )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/python" )

# In case of Windows, we need to make sure that the import library part does not conflict
# with a static library of the same name.
# So we leave it in the build directory and add an _import suffix.
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_NAME "${PROJECT_NAME}_import" )

target_link_libraries( ${PROJECT_NAME} PRIVATE reverbobject_shared )

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER python_externals )

install(TARGETS ${PROJECT_NAME}
        EXPORT VISR_python_externals_exports
        DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
        COMPONENT python_externals )
