# Copyright Institute of Sound and Vibration Research - All rights reserved

add_executable( visr_renderer main.cpp options.hpp options.cpp )

target_compile_definitions( visr_renderer PRIVATE
  CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

if( BUILD_PYTHON_BINDINGS )
  if( NOT BUILD_INSTALL_SHARED_LIBRARIES )
    message( SEND_ERROR "Internal logic error: BUILD_PYTHON_BINDINGS active although shared libraries are not built." )
  endif( NOT BUILD_INSTALL_SHARED_LIBRARIES )
  set( VISR_RENDERER_LIBRARY_TYPE shared )
  # Note: pythonsupport must be linked as a shared library, because there must be
  # exactly one instance of pythonsupport::InitialisationGuard
  target_link_libraries(visr_renderer PRIVATE pythonsupport_${VISR_RENDERER_LIBRARY_TYPE} )
  # Make sure that the Python interpreter initialisation is performed.
  target_compile_definitions( visr_renderer PRIVATE VISR_PYTHON_SUPPORT=1 )
else()
  set( VISR_RENDERER_LIBRARY_TYPE ${BUILD_LIBRARY_TYPE_FOR_APPS} )
endif( BUILD_PYTHON_BINDINGS )

target_link_libraries(visr_renderer PRIVATE apputilities_${VISR_RENDERER_LIBRARY_TYPE} )
target_link_libraries(visr_renderer PRIVATE signalflowspython_${VISR_RENDERER_LIBRARY_TYPE} )
target_link_libraries(visr_renderer PRIVATE audiointerfaces_${VISR_RENDERER_LIBRARY_TYPE} )
target_link_libraries(visr_renderer PRIVATE panning_${VISR_RENDERER_LIBRARY_TYPE} )
target_link_libraries(visr_renderer PRIVATE rrl_${VISR_RENDERER_LIBRARY_TYPE} )
target_link_libraries(visr_renderer PRIVATE visr_${VISR_RENDERER_LIBRARY_TYPE} )
target_link_libraries(visr_renderer PRIVATE Boost::filesystem )

set_target_properties( visr_renderer PROPERTIES FOLDER applications )

install( TARGETS visr_renderer
         RUNTIME DESTINATION bin
         COMPONENT standalone_applications )
