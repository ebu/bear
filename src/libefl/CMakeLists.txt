# Copyright Institute of Sound and Vibration Research - All rights reserved

IF( BUILD_USE_IPP )
 INCLUDE_DIRECTORIES( ${IPP_INCLUDE_DIR} )
 LIST( APPEND SOURCES vector_functions_ipp.cpp)
ELSE( BUILD_USE_IPP )
 LIST( APPEND SOURCES vector_functions.cpp)
ENDIF( BUILD_USE_IPP )

SET( SOURCES
basic_matrix.cpp
basic_vector.cpp
denormalised_number_handling.cpp
error_codes.cpp
filter_functions.cpp
initialise_library.cpp
lagrange_coefficient_calculator.cpp
matrix_functions.cpp
vector_conversions.cpp
vector_functions.cpp
reference/filter_functions.cpp
reference/vector_conversions.cpp
reference/vector_functions.cpp
)

SET( PUBLIC_HEADERS
aligned_array.hpp
alignment.hpp
basic_matrix.hpp
basic_vector.hpp
cartesian_spherical_conversion.hpp
db_linear_conversion.hpp
degree_radian_conversion.hpp
denormalised_number_handling.hpp
error_codes.hpp
export_symbols.hpp
filter_functions.hpp
function_wrapper.hpp
initialise_library.hpp
lagrange_coefficient_calculator.hpp
matrix_functions.hpp
vector_conversions.hpp
vector_functions.hpp
)

SET( PRIVATE_HEADERS
reference/filter_functions.hpp
reference/filter_functions_impl.hpp
reference/vector_conversions.hpp
reference/vector_conversions_impl.hpp
reference/vector_functions.hpp
reference/vector_functions_impl.hpp
)

if( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( efl_static STATIC ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} )
  set_target_properties( efl_static PROPERTIES OUTPUT_NAME efl )
  target_compile_definitions( efl_static PUBLIC VISR_EFL_STATIC_LIBRARY=1)
  set_target_properties( efl_static PROPERTIES FOLDER static_libraries )
endif( "static" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( efl_shared SHARED ${SOURCES} ${PUBLIC_HEADERS} ${PRIVATE_HEADERS} )
  set_target_properties( efl_shared PROPERTIES OUTPUT_NAME efl )
  # Specify a specific name for the .lib stub of the DLL on Windows to avoid clashes with the static library.
  set_target_properties( efl_shared PROPERTIES ARCHIVE_OUTPUT_NAME efl_shared )
  set_target_properties( efl_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" )
  set_target_properties( efl_shared PROPERTIES FOLDER shared_libraries )
endif( "shared" IN_LIST VISR_BUILD_LIBRARY_TYPES )

if( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )
  add_library( efl_static_pic STATIC ${SOURCES} ${HEADERS} )
  set_target_properties( efl_static_pic PROPERTIES OUTPUT_NAME efl_pic )
  set_target_properties( efl_static_pic PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_compile_definitions( efl_static_pic PUBLIC VISR_EFL_STATIC_LIBRARY=1)
  set_target_properties( efl_static_pic PROPERTIES FOLDER static_pic_libraries )
endif( "static_pic" IN_LIST VISR_BUILD_LIBRARY_TYPES )

foreach( LIB_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_link_libraries( efl_${LIB_TYPE} PRIVATE Boost::boost )
  target_compile_definitions( efl_${LIB_TYPE} PRIVATE VISR_BUILD_EFL_LIBRARY=1 )
  set_target_properties( efl_${LIB_TYPE} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}" )
  target_link_libraries( efl_${LIB_TYPE} PRIVATE Boost::boost )
  get_filename_component(PARENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
  target_include_directories(efl_${LIB_TYPE}
    PUBLIC
    $<BUILD_INTERFACE:${PARENT_SOURCE_DIR}>
      $<INSTALL_INTERFACE:include/visr>
    )
endforeach()

if( VISR_SYSTEM_PROCESSOR MATCHES "x86_64" )
  add_subdirectory( intel_x86_64 )
endif( VISR_SYSTEM_PROCESSOR MATCHES "x86_64" )

if( VISR_SYSTEM_PROCESSOR MATCHES "armv7l" )
  add_subdirectory( armv7l_neon_32bit )
endif( VISR_SYSTEM_PROCESSOR MATCHES "armv7l" )

if( BUILD_INSTALL_STATIC_LIBRARIES )
  install(TARGETS efl_static
    EXPORT VISR_static_library_exports
    PUBLIC_HEADER DESTINATION include/visr/libefl COMPONENT development_files
    ARCHIVE DESTINATION lib COMPONENT static_libraries )
endif( BUILD_INSTALL_STATIC_LIBRARIES )

if( BUILD_INSTALL_SHARED_LIBRARIES )
  install(TARGETS efl_shared
          EXPORT VISR_shared_library_exports
    ARCHIVE DESTINATION lib COMPONENT development_files
    RUNTIME DESTINATION lib COMPONENT shared_libraries
    LIBRARY DESTINATION lib COMPONENT shared_libraries
    PUBLIC_HEADER DESTINATION include/visr/libefl COMPONENT development_files )
endif( BUILD_INSTALL_SHARED_LIBRARIES )

if( BUILD_INSTALL_STATIC_PIC_LIBRARIES )
  install(TARGETS efl_static_pic
          EXPORT VISR_static_pic_library_exports
          PUBLIC_HEADER DESTINATION include/visr/libefl COMPONENT development_files
          ARCHIVE DESTINATION lib COMPONENT static_pic_libraries)
endif( BUILD_INSTALL_STATIC_PIC_LIBRARIES )

if( BUILD_TESTING )
  add_subdirectory( test )
endif( BUILD_TESTING )
