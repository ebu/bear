# Copyright Institute of Sound and Vibration Research - All rights reserved

set( APPLICATION_NAME visr_library_test )

set( SOURCES
audio_connection_descriptor.cpp
audio_sample_type.cpp
component_names.cpp
parameter_factory.cpp
parameter_connection.cpp
polymorphic_parameter_port.cpp
test_main.cpp
)

add_executable( ${APPLICATION_NAME} ${SOURCES} )

target_link_libraries( ${APPLICATION_NAME} PRIVATE visr_${BUILD_LIBRARY_TYPE_FOR_APPS})
target_link_libraries( ${APPLICATION_NAME} PRIVATE pml_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( ${APPLICATION_NAME} PRIVATE rcl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( ${APPLICATION_NAME} PRIVATE Boost::filesystem )
target_link_libraries( ${APPLICATION_NAME} PRIVATE Boost::unit_test_framework )
if( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions(${APPLICATION_NAME} PRIVATE -DBOOST_ALL_DYN_LINK )
endif( NOT Boost_USE_STATIC_LIBS )
target_compile_definitions( ${APPLICATION_NAME} PRIVATE -DBOOST_ALL_NO_LIB )

set_target_properties( ${APPLICATION_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/test_binaries)

set_target_properties( ${APPLICATION_NAME} PROPERTIES FOLDER unit_tests )

add_test(NAME ${APPLICATION_NAME} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_binaries/${APPLICATION_NAME} )

include( adjust_test_environment )
adjustTestEnvironment( ${APPLICATION_NAME} )
