# Copyright Institute of Sound and Vibration Research - All rights reserved

INCLUDE_DIRECTORIES( ${MAXSDK_MSP_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${MAXSDK_MAX_INCLUDE_DIR} )

IF( APPLE )
  FIND_LIBRARY( CORE_SERVICE_FRAMEWORK NAMES CoreServices )
  INCLUDE_DIRECTORIES( ${CORE_SERVICE_FRAMEWORK}/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers )
ELSE( APPLE )
  ADD_DEFINITIONS( -DWIN_VERSION )
ENDIF( APPLE )

SET( EXTERNAL_NAME matrix_convolver~ )

SET( PROJECT_NAME matrix_convolver_max_external )

ADD_LIBRARY( ${PROJECT_NAME} MODULE
init_filter_matrix.hpp
init_filter_matrix.cpp
matrix_convolver.hpp
matrix_convolver.cpp
options.hpp
options.cpp
)

SET_TARGET_PROPERTIES( ${PROJECT_NAME}  PROPERTIES
    PREFIX ""
    OUTPUT_NAME ${EXTERNAL_NAME}
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/max" )

# <AF> TODO: Move this conditional logic to a centralised location!
if(WIN32) # 32-bit and 64-bit Windows
  if (CMAKE_CL_64)
      SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES SUFFIX .mxe64)
  else(CMAKE_CL_64)
      SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES SUFFIX .mxe)
  endif(CMAKE_CL_64)
elseif(APPLE)
  # Force generation of MacOS bundles.
  set_target_properties( ${PROJECT_NAME}  PROPERTIES BUNDLE True )
  set_target_properties( ${PROJECT_NAME}  PROPERTIES BUNDLE_EXTENSION "mxo" )
  set_target_properties( ${PROJECT_NAME}  PROPERTIES XCODE_ATTRIBUTE_MACH_O_TYPE mh_bundle )
  # Workaround for CMake bug https://gitlab.kitware.com/cmake/cmake/issues/16147
  # "BUNDLE_EXTENSION doesn't apply for Xcode generator"
  set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "mxo")
endif(WIN32)

# Link either to shared libraries or position-independent static libraries.
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} apputilities_shared ) # slight misnomer, consider restructuring if the option handling should prove beneficial.
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} rcl_shared )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} maxsupport_shared )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} Boost::filesystem )

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER max_msp_externals )

install( TARGETS ${PROJECT_NAME}
         LIBRARY DESTINATION max COMPONENT max_externals
         RUNTIME DESTINATION max COMPONENT max_externals )
