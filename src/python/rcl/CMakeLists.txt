# Python bindings for the standard VISR interface component

set( SOURCES
add.cpp
biquad_iir_filter.cpp
cap_gain_calculator.cpp
channel_object_routing_calculator.cpp
crossfading_fir_filter_matrix.cpp
delay_matrix.cpp
delay_vector.cpp
diffusion_gain_calculator.cpp
fir_filter_matrix.cpp
gain_matrix.cpp
gain_vector.cpp
hoa_allrad_gain_calculator.cpp
interpolating_fir_filter_matrix.cpp
listener_compensation.cpp
null_source.cpp
object_gain_eq_calculator.cpp
panning_calculator.cpp
parameter_input_terminator.cpp
parameter_output_terminator.cpp
position_decoder.cpp
rcl.cpp
scene_decoder.cpp
scene_encoder.cpp
scalar_osc_decoder.cpp
signal_routing.cpp
sparse_gain_matrix.cpp
time_frequency_inverse_transform.cpp
time_frequency_transform.cpp
udp_receiver.cpp
udp_sender.cpp
)

# We don't need header files

set(PROJECT_NAME rclpython)

set(MODULE_NAME rcl)

pybind11_add_module(${PROJECT_NAME} MODULE ${HEADERS} ${SOURCES} )

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME} )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/python" )

# In case of Windows, we need to make sure that the import library part does not conflict
# with a static library of the same name.
# So we leave it in the build directory and add an _import suffix.
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
set_target_properties( ${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_NAME "${PROJECT_NAME}_import" )

target_link_libraries( ${PROJECT_NAME} PRIVATE rcl_shared )
target_link_libraries( ${PROJECT_NAME} PRIVATE pythonbindinghelpers )

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER python_externals )

# Note: We export the target to enable dependent projects to query target properties
# such as the library location.
install(TARGETS ${PROJECT_NAME}
        EXPORT VISR_python_externals_exports
        DESTINATION ${PYTHON_MODULE_INSTALL_DIRECTORY}
        COMPONENT python_externals )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
