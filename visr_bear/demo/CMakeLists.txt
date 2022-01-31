pybind11_add_module(bear_demo MODULE runner.cpp python.cpp)

set_target_properties(bear_demo PROPERTIES OUTPUT_NAME bear_demo)

set_target_properties(bear_demo PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                                 "${PROJECT_BINARY_DIR}/python")

# In case of Windows, we need to make sure that the import library part does not
# conflict with a static library of the same name. So we leave it in the build
# directory and add an _import suffix.
set_target_properties(bear_demo PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
                                                 ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(bear_demo PROPERTIES ARCHIVE_OUTPUT_NAME
                                                 "bear_demo_import")

target_link_libraries(bear_demo PRIVATE bear bear-internals)
target_link_libraries(bear_demo PUBLIC VISR::audiointerfaces_${BEAR_VISR_LIB_TYPE})

install(TARGETS bear_demo DESTINATION ${BEAR_PYTHON_SITE_PACKAGES})
