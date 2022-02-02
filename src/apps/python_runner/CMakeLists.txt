# Copyright Institute of Sound and Vibration Research - All rights reserved

add_executable( python_runner main.cpp options.hpp options.cpp )

target_link_libraries( python_runner PRIVATE audiointerfaces_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( python_runner PRIVATE apputilities_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( python_runner PRIVATE pythoncomponents_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( python_runner PRIVATE pythonsupport_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( python_runner PRIVATE pml_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( python_runner PRIVATE rrl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( python_runner PRIVATE visr_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries( python_runner PRIVATE Boost::filesystem )

set_target_properties( python_runner PROPERTIES FOLDER applications )

install( TARGETS python_runner DESTINATION bin COMPONENT standalone_applications )
