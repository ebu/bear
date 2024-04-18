# Copyright Institute of Sound and Vibration Research - All rights reserved

set( APPLICATION_NAME pythoncomponents_test )

set( SOURCES
pml_initialisation.cpp
test_main.cpp
wrapper.cpp
)

add_executable( ${APPLICATION_NAME} ${SOURCES} )

target_link_libraries( ${APPLICATION_NAME} PRIVATE pythoncomponents_shared )
target_link_libraries( ${APPLICATION_NAME} PRIVATE pythonsupport_shared )
target_link_libraries( ${APPLICATION_NAME} PRIVATE rrl_shared )
target_link_libraries( ${APPLICATION_NAME} PRIVATE pml_shared )
target_link_libraries( ${APPLICATION_NAME} PRIVATE Boost::filesystem )
target_link_libraries( ${APPLICATION_NAME} PRIVATE Boost::unit_test_framework )

if( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions(${APPLICATION_NAME} PRIVATE -DBOOST_ALL_DYN_LINK )
endif( NOT Boost_USE_STATIC_LIBS )
target_compile_definitions( ${APPLICATION_NAME} PRIVATE -DBOOST_ALL_NO_LIB )

# Since the 'wrapper' unit tests require the visr Python external, ensure that it is compiled
add_dependencies( ${APPLICATION_NAME} visrpython eflpython pmlpython rbblpython objectmodelpython )

# Encode the current source location to give access to the Python code contained in the test/python directory.
target_compile_definitions( ${APPLICATION_NAME} PRIVATE CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

# Encode the path to the visr Python external into a compile definition
target_compile_definitions( ${APPLICATION_NAME} PRIVATE PYTHON_MODULE_INSTALL_DIRECTORY="$<TARGET_FILE_DIR:visrpython>" )

set_target_properties( ${APPLICATION_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/test_binaries)

set_target_properties( ${APPLICATION_NAME} PROPERTIES FOLDER unit_tests )

add_test(NAME ${APPLICATION_NAME} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_binaries/${APPLICATION_NAME} )

include( adjust_test_environment )
adjustTestEnvironment( ${APPLICATION_NAME} )

# The PYTHONHOME environment variable must be set in order to let the Python
# interpreter embedded in the unit test binary find the right standard
# libraries, i.e., the one corresponding to the Python environment used for
# building the externals.
# Retrieve the correct setting for PYTHONHOME from the Python executable.
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} -c "import sys;print(sys.prefix+':'+sys.exec_prefix)"
  OUTPUT_VARIABLE PYTHON_HOME OUTPUT_STRIP_TRAILING_WHITESPACE )
set_property(TEST ${APPLICATION_NAME} APPEND PROPERTY ENVIRONMENT
  "PYTHONHOME=${PYTHON_HOME}" )
# Disable leak checking because the memory handling of Python interpreter
# causes leak reports (false positives?) that are outside our responsibility.
set_property(TEST ${APPLICATION_NAME} APPEND PROPERTY ENVIRONMENT
  "BOOST_TEST_DETECT_MEMORY_LEAK=0" )
