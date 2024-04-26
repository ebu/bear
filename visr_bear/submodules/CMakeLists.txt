add_library(mio INTERFACE)
target_include_directories(mio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mio)

if(BEAR_USE_INTERNAL_LIBEAR)
  set(EAR_PACKAGE_AND_INSTALL
      ${BEAR_PACKAGE_AND_INSTALL}
      CACHE BOOL "Package and install libear")
  add_subdirectory(libear)
endif()
