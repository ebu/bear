# Copyright Institute of Sound and Vibration Research - All rights reserved

set( APPLICATION_NAME rrl_test )

set( SOURCES
audio_signal_flow_checking.cpp
parameter_connection.cpp
test_main.cpp
)

add_executable( ${APPLICATION_NAME} ${SOURCES} )

# Note: Consider moving delay_vector somewhere else (if dependency on real component libraries is not desired here.)
target_link_libraries( ${APPLICATION_NAME} PRIVATE signalflows_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( ${APPLICATION_NAME} PRIVATE rcl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( ${APPLICATION_NAME} PRIVATE rrl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( ${APPLICATION_NAME} PRIVATE pml_${BUILD_LIBRARY_TYPE_FOR_APPS} )
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
