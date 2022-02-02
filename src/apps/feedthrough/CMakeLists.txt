# Copyright Institute of Sound and Vibration Research - All rights reserved

add_executable( feedthrough_app
  main.cpp
  options.hpp
  options.cpp
  signal_flow.cpp )

target_link_libraries(feedthrough_app PRIVATE visr_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(feedthrough_app PRIVATE rrl_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(feedthrough_app PRIVATE apputilities_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(feedthrough_app PRIVATE audiointerfaces_${BUILD_LIBRARY_TYPE_FOR_APPS} )
target_link_libraries(feedthrough_app PRIVATE Boost::filesystem )

set_target_properties( feedthrough_app PROPERTIES OUTPUT_NAME feedthrough )
set_target_properties( feedthrough_app PROPERTIES FOLDER applications )

install( TARGETS feedthrough_app  DESTINATION bin COMPONENT standalone_applications )