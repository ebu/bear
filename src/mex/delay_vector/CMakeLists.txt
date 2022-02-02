# compile mex

# Code adapted from
# http://www.mathworks.com/matlabcentral/fileexchange/45522-mex-cmake

set(CPP_FILE delay_vector)

set(HEADER_FILE signal_flow.hpp )

set(PROJECT_NAME delay_vector_mex)

# set up matlab libraries
set(CPP_FILE_NAME ${CPP_FILE}.cpp)
INCLUDE_DIRECTORIES(${MATLAB_INCLUDE_DIR})

# <AF> Don't use export files, put export the mexFunction explicitly.
add_library(${PROJECT_NAME} MODULE ${CPP_FILE_NAME} ${ADDITIONAL_CPP_FILES}  ${PROJECT_SOURCE_DIR}/cmake_modules/Matlabdef.def )

# Set the name of the mex object (and thus the Matlab function to be called) independently
# of the project name.
# Also set the file extension according to the operating system.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${CPP_FILE} SUFFIX ${MATLAB_MEX_SUFFIX} PREFIX "")

# Add any extras dependencies
target_link_libraries(${PROJECT_NAME} signalflows rbbl rcl visr_static efl mexsupport ${MATLAB_LIBRARIES})

# Put directly into the mex/ subdirectory
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/mex" )

set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER matlab_externals )

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_BINARY_DIR}/mex )
