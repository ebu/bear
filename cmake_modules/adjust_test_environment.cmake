# Copyright (c) 2014-2020 Andreas Franck a.franck@soton.ac.uk --- All rights reserved.
# Copyright (c) 2014-2019 Institute of Sound and Vibration Research,
#   University of Southampton and VISR contributors --- All rights reserved.

# Adjust the system environment of a unit test target.
# This is mainly needed on Windows to put the paths containing the 3rd party
# libraries and the VISR DLLs on the system path.
function( adjustTestEnvironment target )
  if(VISR_SYSTEM_NAME MATCHES "Windows")
    # Prevent CMake from interpreting the semicolon (the separator in Windows path lists) as list separator.
    # Double quoting required because the variable is evaluated twice.
    string(REPLACE ";" "\\\;"  QUOTED_PATH "$ENV{PATH}" )
    if( BUILD_LIBRARY_TYPE_FOR_APPS STREQUAL "shared" )
      set_property(TEST ${target} APPEND PROPERTY ENVIRONMENT
        "PATH=${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR}\;$<TARGET_FILE_DIR:visr_shared>\;${QUOTED_PATH}" )
    else()
     set_property(TEST ${APPLICATION_NAME} APPEND PROPERTY ENVIRONMENT
        "PATH=${VISR_BUILD_3RD_PARTY_RUNTIME_LIBRARY_DIR}\;${QUOTED_PATH}" )
    endif( BUILD_LIBRARY_TYPE_FOR_APPS STREQUAL "shared" )
  endif(VISR_SYSTEM_NAME MATCHES "Windows")
endfunction()


# Adjust the system environment of a Python-based unit test target.
# In addition to the adjustments needed for all unit tests, the PYTHONPATH must contain
# the directory holding all used VISR Python externals.
# This is done by passing a CMake target pythonpackage.
function( adjustPythonTestEnvironment target pythonpackage )
  adjustTestEnvironment( ${target} )
  if( CMAKE_VERSION VERSION_LESS "3.22.0" )
    # ideally we would add to PYTHONPATH, but the path separator is
    # system-dependant, and the sensible way to do it would be to use
    # cmake_path or cmake -E env, but those don't help expand the support base
    # (much), so just ignore it with a warning
    message(WARNING "packages on PYTHONPATH will be ignored in tests; upgrade to cmake 3.22")
    set_property( TEST ${target} APPEND PROPERTY ENVIRONMENT
      "PYTHONPATH=$<TARGET_FILE_DIR:${pythonpackage}>" )
  else()
    set_property( TEST ${target} APPEND PROPERTY ENVIRONMENT_MODIFICATION
      "PYTHONPATH=path_list_prepend:$<TARGET_FILE_DIR:${pythonpackage}>" )
  endif( CMAKE_VERSION VERSION_LESS "3.22.0" )
endfunction()
