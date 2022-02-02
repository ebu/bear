# Copyright Institute of Sound and Vibration Research - All rights reserved


set( HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/cpu_features.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/initialise_library.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vector_functions.hpp
)

set( SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/cpu_features.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/initialise_library.cpp
)

# sources which are compiled multiple times with different VISR_SIMD_FEATURE
# defines and corresponding instruction set flags. these should only contain
# public symbols which have VISR_SIMD_FEATURE in the name/type.
set( FEATURE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/vector_multiply_add.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vector_ramp_scaling.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vector_multiply.cpp
)

# add $FEATURE_SOURCES to $TARGET while setting VISR_SIMD_FEATURE=$FEATURE and
# setting compile options to $FLAGS
function(add_feature_sources TARGET_NAME TARGET_TYPE FEATURE FLAGS MSVC_FLAGS)
  set( TARGET ${TARGET_NAME}_${TARGET_TYPE} )
  add_library(${TARGET}_${FEATURE} OBJECT ${FEATURE_SOURCES})
  target_sources(${TARGET} PRIVATE $<TARGET_OBJECTS:${TARGET}_${FEATURE}>)

  # copy relevant target properties to feature-specific objects
  foreach(PROPERTY COMPILE_DEFINITIONS)
    get_target_property(VALUE ${TARGET} ${PROPERTY})
    set_property(TARGET ${TARGET}_${FEATURE} PROPERTY ${PROPERTY} ${VALUE})
  endforeach()

  target_compile_definitions(${TARGET}_${FEATURE} PRIVATE VISR_SIMD_FEATURE=${FEATURE})

  if(MSVC)
    target_compile_options(${TARGET}_${FEATURE} PRIVATE ${MSVC_FLAGS})
  else()
    target_compile_options(${TARGET}_${FEATURE} PRIVATE ${FLAGS})
  endif()

  set_target_properties( ${TARGET}_${FEATURE} PROPERTIES FOLDER ${TARGET_TYPE}_libraries/internal )
endfunction()

# Add the architecture-specific files to the efl libraries.
foreach(TARGET_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_sources( efl_${TARGET_TYPE} PRIVATE ${HEADERS} ${SOURCES} )
  add_feature_sources( efl ${TARGET_TYPE} FMA "-mfma;-mavx2;-mavx;-msse4.2" "/arch:AVX2" )
  add_feature_sources( efl ${TARGET_TYPE} AVX "-mavx;-msse4.2" "/arch:AVX" )
  add_feature_sources( efl ${TARGET_TYPE} SSE "-msse4.2" "")
endforeach()
