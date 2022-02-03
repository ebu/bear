# Copyright Andreas Franck a.franck@soton.ac.uk - All rights reserved Copyright
# Institute of Sound and Vibration Research - All rights reserved

# Helper function to extract a value from a key-value pair within the
# INTERFACE_COMPILE_DEFINITIONS property of a CMake target.
function(getValueFromCompileDefinition target key result)
  get_target_property(DEFINITION_LIST ${target} INTERFACE_COMPILE_DEFINITIONS)
  list(FILTER DEFINITION_LIST INCLUDE REGEX ${key}=*)
  list(LENGTH DEFINITION_LIST NUM_MATCHES)
  if(NUM_MATCHES EQUAL 1)
    list(GET DEFINITION_LIST 0 KEYVAL)
    string(REGEX REPLACE ".*=(.*)" "\\1" VAL ${KEYVAL})
  else(NUM_MATCHES EQUAL 1)
    message(WARNING "Could not retrieve value for key " ${key} " from target "
                    ${target} "'s compile definitions.")
    set(${VAL} "")
  endif(NUM_MATCHES EQUAL 1)
  set(${result}
      ${VAL}
      PARENT_SCOPE)
endfunction()

# Determine value of VISR PYBIND11_DIR. This is encoded within the interface
# compile definitions of target pythonsupport_shared
function(getPybind11Dir target result)
  getvaluefromcompiledefinition(${target} PYBIND11_DIR RES)
  set(${result}
      ${RES}
      PARENT_SCOPE)
endfunction()

# Determine the Pybind11 version used to compile VISR.
function(getVisrPybind11Version target result)
  getvaluefromcompiledefinition(${target} VISR_PYBIND11_VERSION RES)
  set(${result}
      ${RES}
      PARENT_SCOPE)
endfunction()

# Initialise PYBIND11_DIR with the value sensed from the respective VISR
# directory.
getpybind11dir(VISR::pythonsupport_shared VISR_PYBIND_DIR)
set(PYBIND11_DIR
    ${VISR_PYBIND_DIR}
    CACHE
      PATH
      "Location of the Pybind11 library. Defaults to the one provided by the VISR installation."
)

# The second parameter (binary dir) is needed because PYBIND11_DIR is possibly
# outside the project directory.
add_subdirectory(${PYBIND11_DIR} ${CMAKE_BINARY_DIR}/3rd/pybind11
                 EXCLUDE_FROM_ALL)

# get PYTHON_SITE_PACKAGES to figure out where to install; this looks at
# PYTHON_EXECUTABLE to figure out which python to use, which makes sure that
# it's always compatible with the python found by the pybind11 add_subdirectory
# above
list(APPEND CMAKE_MODULE_PATH "${PYBIND11_DIR}/tools")
find_package(PythonLibsNew REQUIRED)
# allow users to override this for package building
set(BEAR_PYTHON_SITE_PACKAGES
    ${PYTHON_SITE_PACKAGES}
    CACHE PATH "path to install python modules")

# Check whether the pybind11 version used to build the VISR Python externals is
# compatible with the one used for building this library.
getvisrpybind11version(VISR::pythonsupport_shared VISR_PYBIND11_VERSION)
message(STATUS "VISR_PYBIND11_VERSION: " ${VISR_PYBIND11_VERSION})
if(NOT ${VISR_PYBIND11_VERSION} VERSION_EQUAL
   ${PYBIND11_VERSION_MAJOR}.${PYBIND11_VERSION_MINOR}.${PYBIND11_VERSION_PATCH}
)
  message(
    WARNING
      "Pybind11 version used in VISR differs from version used for library."
      " This might cause problems.")
endif()

set(SOURCES bearmodule.cpp api.cpp debug_sink.cpp internals.cpp
            dynamic_renderer.cpp)

set(PROJECT_NAME pythonwrappers)

set(MODULE_NAME visr_bear)

pybind11_add_module(${PROJECT_NAME} MODULE ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${MODULE_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                                 "${PROJECT_BINARY_DIR}/python")

# In case of Windows, we need to make sure that the import library part does not
# conflict with a static library of the same name. So we leave it in the build
# directory and add an _import suffix.
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
                                                 ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_NAME
                                                 "${PROJECT_NAME}_import")

target_link_libraries(${PROJECT_NAME} PRIVATE bear bear-internals)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BEAR_PYTHON_SITE_PACKAGES})

# install known data files as a separate python package
install(DIRECTORY visr_bear_data DESTINATION ${BEAR_PYTHON_SITE_PACKAGES})
function(install_data_file name path)
  if(BEAR_SYMLINK_DATA_FILES AND path)
    install(
      CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    ${path} \
    ${BEAR_PYTHON_SITE_PACKAGES}/visr_bear_data/${name} \
    )")
  else()
    install(
      FILES ${path}
      DESTINATION ${BEAR_PYTHON_SITE_PACKAGES}/visr_bear_data
      RENAME ${name}
      OPTIONAL)
  endif()
endfunction()
install_data_file("default.tf" "${BEAR_DATA_OUTPUT_PATH_DEFAULT}")
install_data_file("default_small.tf" "${BEAR_DATA_OUTPUT_PATH_DEFAULT_SMALL}")
