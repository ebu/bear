# Python bindings for the main VISR interface classes

# Define this variable to include the binding for the metadapter-enabled VisrRenderer.
# At the moment, the visr_renderer binding is excluded to break the dependency to the Python libraries.
# This dependency is likely to cause problems on Mac OS.
# TODO: Check whether pythoncomponent::Wrapper can be made working at all from within a Python interpreter!
# set( VISR_RENDERER_PYTHON_BINDING 1 )

set( SOURCES
baseline_renderer.cpp
core_renderer.cpp
delay_vector.cpp
signalflows.cpp
)

set( HEADERS
baseline_renderer.hpp
core_renderer.hpp
delay_vector.hpp
)

if( ${VISR_RENDERER_PYTHON_BINDING} )
  message( STATUS "Entering VISR_RENDERER_PYTHON_BINDING" )
  list( APPEND SOURCES visr_renderer.cpp )
endif( ${VISR_RENDERER_PYTHON_BINDING} )

set(PROJECT_NAME signalflowspython)

set(MODULE_NAME signalflows)

pybind11_add_module(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES} )

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME} )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/python" )

# In case of Windows, we need to make sure that the import library part does not conflict
# with a static library of the same name.
# So we leave it in the build directory and add an _import suffix.
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_NAME "${PROJECT_NAME}_import" )

target_link_libraries( ${PROJECT_NAME} PRIVATE signalflows_shared )
if( ${VISR_RENDERER_PYTHON_BINDING} )
  target_link_libraries( ${PROJECT_NAME} PRIVATE signalflowspython_shared ) 
  target_compile_definitions( ${PROJECT_NAME} PRIVATE VISR_RENDERER_PYTHON_BINDING=1 )
endif( ${VISR_RENDERER_PYTHON_BINDING} )

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER python_externals )

# Note: We export the target to enable dependent projects to query target properties
# such as the library location.
install(TARGETS ${PROJECT_NAME}
        EXPORT VISR_python_externals_exports
        DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
        COMPONENT python_externals )
