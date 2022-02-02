# Copyright Institute of Sound and Vibration Research - All rights reserved

set( APPLICATION_NAME rbbl_test )

add_definitions( -DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

set( SOURCES
 biquad_coefficient.cpp
 circular_buffer.cpp
 ffts_wrapper.cpp
 float_sequence.cpp index_sequence.cpp
 interpolating_convolver.cpp
 kiss_fft_wrapper.cpp
 parametric_iir_coefficient.cpp
 test_main.cpp
 test_FIR.cpp
 multichannel_convolver.cpp
 object_channel_allocator.cpp
 sparse_gain_routing.cpp
 test_main.cpp
)

add_executable( ${APPLICATION_NAME} ${SOURCES} )

target_link_libraries( ${APPLICATION_NAME} PRIVATE rbbl_${BUILD_LIBRARY_TYPE_FOR_APPS})
target_link_libraries( ${APPLICATION_NAME} PRIVATE pml_${BUILD_LIBRARY_TYPE_FOR_APPS})
target_link_libraries( ${APPLICATION_NAME} PRIVATE visr_${BUILD_LIBRARY_TYPE_FOR_APPS})
target_link_libraries( ${APPLICATION_NAME} PRIVATE Boost::filesystem )
target_link_libraries( ${APPLICATION_NAME} PRIVATE Boost::unit_test_framework )
if( NOT Boost_USE_STATIC_LIBS )
  target_compile_definitions( ${APPLICATION_NAME} PRIVATE -DBOOST_ALL_DYN_LINK )
endif( NOT Boost_USE_STATIC_LIBS )
target_compile_definitions( ${APPLICATION_NAME} PRIVATE -DBOOST_ALL_NO_LIB )
if( BUILD_USE_IPP )
  list( APPEND SOURCES ipp_fft_wrapper.cpp)
  target_compile_definitions(${APPLICATION_NAME} PRIVATE BUILD_USE_IPP )
endif( BUILD_USE_IPP )

set_target_properties( ${APPLICATION_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/test_binaries)

set_target_properties( ${APPLICATION_NAME} PROPERTIES FOLDER unit_tests )

add_test(NAME ${APPLICATION_NAME} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_binaries/${APPLICATION_NAME} )

include( adjust_test_environment )
adjustTestEnvironment( ${APPLICATION_NAME} )
