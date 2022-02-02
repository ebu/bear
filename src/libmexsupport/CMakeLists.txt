# Copyright Institute of Sound and Vibration Research - All rights reserved

INCLUDE_DIRECTORIES(${MATLAB_INCLUDE_DIR})

SET( SOURCES mex_wrapper.cpp )

# Basically, this makes the files show up in the Visual studio project.
SET( HEADERS export_symbol.hpp mex_wrapper.hpp )

add_library( mexsupport STATIC ${SOURCES} ${HEADERS} )
# Transitive link dependencies
target_link_libraries( mexsupport PRIVATE visr_shared )
target_link_libraries( mexsupport PRIVATE ${MATLAB_MEX_LIBRARY} )
target_link_libraries( mexsupport PRIVATE ${MATLAB_MX_LIBRARY} )
set_target_properties( mexsupport PROPERTIES FOLDER static_libraries )

set_target_properties( mexsupport PROPERTIES FOLDER matlab_externals )
