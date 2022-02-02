# Copyright Institute of Sound and Vibration Research - All rights reserved

add_definitions( -DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

add_executable( baseline_renderer main.cpp options.hpp options.cpp )

target_link_libraries(baseline_renderer PRIVATE apputilities_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(baseline_renderer PRIVATE signalflows_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(baseline_renderer PRIVATE audiointerfaces_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(baseline_renderer PRIVATE rrl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(baseline_renderer PRIVATE Boost::filesystem )

set_target_properties( baseline_renderer PROPERTIES FOLDER applications )

install( TARGETS baseline_renderer DESTINATION bin COMPONENT standalone_applications )
