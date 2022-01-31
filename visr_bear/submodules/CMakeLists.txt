add_library(mio INTERFACE)
target_include_directories(mio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mio)

add_library(RapidJSON INTERFACE)
target_include_directories(
  RapidJSON INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

if(USE_INTERNAL_LIBEAR)
  set(EAR_PACKAGE_AND_INSTALL
      ${BEAR_PACKAGE_AND_INSTALL}
      CACHE BOOL "Package and install libear")
  add_subdirectory(libear)
endif()
