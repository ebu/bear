# compile mex

# Code adapted from
# http://www.mathworks.com/matlabcentral/fileexchange/45522-mex-cmake

set(CPP_FILE biquad_iir_filter)

set(PROJECT_NAME biquad_iir_filter_mex)

set( ADDITIONAL_CPP_FILES signal_flow.cpp )

# set up matlab libraries
set(CPP_FILE_NAME ${CPP_FILE}.cpp)
INCLUDE_DIRECTORIES(${MATLAB_INCLUDE_DIR})

# <AF> Don't use export files, put export the mexFunction explicitly.
add_library(${PROJECT_NAME} MODULE ${CPP_FILE_NAME} ${ADDITIONAL_CPP_FILES}  ${PROJECT_SOURCE_DIR}/cmake_modules/Matlabdef.def )

# Add any extras dependencies
target_link_libraries(${PROJECT_NAME} rcl_static visr_static mexsupport ${MATLAB_LIBRARIES})

IF( BUILD_USE_IPP )
  TARGET_LINK_LIBRARIES(${APPLICATION_NAME} ${IPP_LIB_S} ${IPP_LIB_CORE} )
ENDIF( BUILD_USE_IPP )

# Set the name of the mex object (and thus the Matlab function to be called) independently
# of the project name.
# Also set the file extension according to the operating system.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${CPP_FILE} SUFFIX ${MATLAB_MEX_SUFFIX} PREFIX "")

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER matlab_externals )

# Install the .mexXXX file to mex/
INSTALL(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION mex)
