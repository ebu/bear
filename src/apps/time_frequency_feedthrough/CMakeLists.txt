# Copyright Institute of Sound and Vibration Research - All rights reserved

add_executable( time_frequency_feedthrough main.cpp )

target_link_libraries(time_frequency_feedthrough PRIVATE signalflows_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(time_frequency_feedthrough PRIVATE rrl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(time_frequency_feedthrough PRIVATE audiointerfaces_${BUILD_LIBRARY_TYPE_FOR_APPS} )

set_target_properties( time_frequency_feedthrough PROPERTIES FOLDER applications )

install( TARGETS time_frequency_feedthrough DESTINATION bin COMPONENT standalone_applications )

