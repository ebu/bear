# Copyright Institute of Sound and Vibration Research - All rights reserved

set( SOURCES main.cpp init_filter_matrix.cpp options.cpp )
set( HEADERS init_filter_matrix.hpp options.hpp )

add_executable( matrix_convolver_app ${SOURCES} ${HEADERS} )

set_target_properties( matrix_convolver_app PROPERTIES OUTPUT_NAME matrix_convolver )

target_link_libraries(matrix_convolver_app PRIVATE apputilities_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(matrix_convolver_app PRIVATE rcl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(matrix_convolver_app PRIVATE rrl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(matrix_convolver_app PRIVATE audiointerfaces_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(matrix_convolver_app PRIVATE visr_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(matrix_convolver_app PRIVATE Boost::filesystem )

set_target_properties( matrix_convolver_app PROPERTIES OUTPUT_NAME matrix_convolver )
set_target_properties( matrix_convolver_app PROPERTIES FOLDER applications )

install( TARGETS matrix_convolver_app DESTINATION bin COMPONENT standalone_applications )
