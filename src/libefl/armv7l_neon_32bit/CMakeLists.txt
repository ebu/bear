# Copyright Institute of Sound and Vibration Research - All rights reserved

set( HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/initialise_library.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_functions.hpp
)

set( SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/initialise_library.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_add.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_multiply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_multiply_add.cpp
)

# Add the architecture-specific files to the efl libraries.
foreach(TARGET_TYPE ${VISR_BUILD_LIBRARY_TYPES} )
  target_sources(efl_${TARGET_TYPE} PRIVATE ${HEADERS} ${SOURCES} )
endforeach()
