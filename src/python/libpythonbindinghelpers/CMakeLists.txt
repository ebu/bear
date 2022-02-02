# Copyright Institute of Sound and Vibration Research - All rights reserved

# This is a header-only (interface) library
add_library( pythonbindinghelpers INTERFACE )

# Define the header files to be installed.
SET( PUBLIC_HEADERS
  container_access.hpp
  ndarray_from_basic_vector.hpp
  ndarray_from_basic_matrix.hpp
  matrix_from_ndarray.hpp
  vector_from_ndarray.hpp
)

# Set the include directory for build and install interface
target_include_directories( pythonbindinghelpers
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include/visr>
)

install( TARGETS pythonbindinghelpers
         EXPORT VISR_python_externals_exports
         COMPONENT development_files )

# Install files (this need to be done separately for interface targets)
# (see https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#interface-libraries)
install( FILES ${PUBLIC_HEADERS}
         DESTINATION include/visr/python/libpythonbindinghelpers
         COMPONENT development_files )
