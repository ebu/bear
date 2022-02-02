# Copyright Institute of Sound and Vibration Research - All rights reserved


set( SOURCES
biquad_coefficient.cpp
circular_buffer.cpp
crossfading_convolver_uniform.cpp
core_convolver_uniform.cpp
fft_wrapper_factory.cpp
ffts_wrapper.cpp
fir.cpp
filter_routing.cpp
fractional_delay_base.cpp
fractional_delay_factory.cpp
float_sequence.cpp
gain_fader.cpp
gain_matrix.cpp
index_sequence.cpp
interpolating_convolver_uniform.cpp
interpolation_parameter.cpp
kiss_fft_wrapper_double.cpp
kiss_fft_wrapper_float.cpp
lagrange_interpolator.cpp
multichannel_convolver_uniform.cpp
multichannel_delay_line.cpp
object_channel_allocator.cpp
parametric_iir_coefficient.cpp
parametric_iir_coefficient_calculator.cpp
position_3d.cpp
quaternion.cpp
sparse_gain_routing.cpp
)

# Basically, this makes the headers show up in the Visual studio project.
set( HEADERS
biquad_coefficient.hpp
circular_buffer.hpp
core_convolver_uniform.hpp
crossfading_convolver_uniform.hpp
export_symbols.hpp
filter_routing.hpp
fir.hpp
fft_wrapper_base.hpp
fft_wrapper_factory.hpp
ffts_wrapper.hpp
float_sequence.hpp
fractional_delay_base.hpp
fractional_delay_factory.hpp
gain_fader.hpp
gain_matrix.hpp
index_sequence.hpp
interpolating_convolver_uniform.hpp
interpolation_parameter.hpp
kiss_fft_wrapper.hpp
lagrange_interpolator.hpp
multichannel_convolver_uniform.hpp
multichannel_delay_line.hpp
object_channel_allocator.hpp
parametric_iir_coefficient.hpp
parametric_iir_coefficient_calculator.hpp
position_3d.hpp
quaternion.hpp
sparse_gain_routing.hpp
)

if( BUILD_USE_IPP )
  list( APPEND SOURCES ipp_fft_wrapper.cpp )
  list( APPEND HEADERS ipp_fft_wrapper.hpp )
endif( BUILD_USE_IPP )

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( rbbl_static STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( rbbl_static PROPERTIES OUTPUT_NAME rbbl )
  target_compile_definitions( rbbl_static PUBLIC VISR_RBBL_STATIC_LIBRARY=1)
  set_target_properties( rbbl_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( rbbl_shared SHARED ${SOURCES} ${HEADERS} )
  set_target_properties( rbbl_shared PROPERTIES OUTPUT_NAME rbbl )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( rbbl_shared PROPERTIES ARCHIVE_OUTPUT_NAME rbbl_shared )
  set_target_properties( rbbl_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( rbbl_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( rbbl_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( rbbl_static_pic PROPERTIES OUTPUT_NAME rbbl_pic )
  set_target_properties( rbbl_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( rbbl_static_pic PUBLIC VISR_RBBL_STATIC_LIBRARY=1)
  set_target_properties( rbbl_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

# Set target properties common to both library types.
foreach(LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_compile_definitions( rbbl_${LIB_TYPE} PRIVATE VISR_BUILD_RBBL_LIBRARY=1)
  target_link_libraries( rbbl_${LIB_TYPE} PUBLIC efl_${LIB_TYPE} )
  target_link_libraries( rbbl_${LIB_TYPE} PRIVATE ffts_static )
  target_link_libraries( rbbl_${LIB_TYPE} PRIVATE kissfft_static )
  target_link_libraries( rbbl_${LIB_TYPE} PUBLIC Boost::boost ) # Adds the boost include directory
  if( BUILD_USE_IPP )
    target_compile_definitions( rbbl_${LIB_TYPE} PRIVATE -DBUILD_USE_IPP )
    target_include_directories( rbbl_${LIB_TYPE} PRIVATE ${IPP_INCLUDE_DIR} )
    target_link_libraries( rbbl_${LIB_TYPE} PRIVATE ${IPP_LIB_S} ${IPP_LIB_VM} ${IPP_LIB_CORE} )
  endif( BUILD_USE_IPP )
  # Set public headers to be installed.
  set_target_properties( rbbl_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${HEADERS}" )
  # Set include paths for dependent projects
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(rbbl_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/visr>
  )
endforeach()

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS rbbl_static
          EXPORT VISR_static_library_exports
          PUBLIC_HEADER DESTINATION include/visr/librbbl COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS rbbl_shared
          EXPORT VISR_shared_library_exports
          ARCHIVE DESTINATION lib COMPONENT development_files
          RUNTIME DESTINATION lib COMPONENT shared_libraries
          LIBRARY DESTINATION lib COMPONENT shared_libraries
          PUBLIC_HEADER DESTINATION include/visr/librbbl COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS rbbl_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/librbbl COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )


if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
