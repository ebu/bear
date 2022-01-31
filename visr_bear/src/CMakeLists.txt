# internal libraries which we need for building the library and for accessing
# internals (test,
add_library(bear-internals INTERFACE)
target_link_libraries(bear-internals INTERFACE mio)
target_link_libraries(bear-internals INTERFACE RapidJSON)
target_link_libraries(bear-internals INTERFACE Eigen3::Eigen)
target_include_directories(bear-internals INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(
  bear
  api.cpp
  brir_interpolation_controller.cpp
  brir_interpolation_controller.hpp
  config_impl.hpp
  constructor_thread.cpp
  control.cpp
  control.hpp
  direct_delay_calc.cpp
  direct_delay_calc.hpp
  direct_diffuse_split.cpp
  direct_diffuse_split.hpp
  direct_speakers_delay_calc.cpp
  direct_speakers_delay_calc.hpp
  direct_speakers_gain_calc.cpp
  direct_speakers_gain_calc.hpp
  direct_speakers_gain_norm.cpp
  direct_speakers_gain_norm.hpp
  dsp.cpp
  dsp.hpp
  dynamic_renderer.cpp
  dynamic_renderer.hpp
  gain_calc_hoa.cpp
  gain_calc_hoa.hpp
  gain_calc_objects.cpp
  gain_calc_objects.hpp
  gain_norm.cpp
  gain_norm.hpp
  geom.cpp
  geom.hpp
  listener_adaptation.cpp
  listener_adaptation.hpp
  listener_impl.hpp
  listener_impl.cpp
  panner.cpp
  panner.hpp
  per_ear_delay.cpp
  per_ear_delay.hpp
  parameters.cpp
  parameters.hpp
  select_brir.cpp
  select_brir.hpp
  sh_rotation.cpp
  sh_rotation.hpp
  static_delay_calc.cpp
  static_delay_calc.hpp
  tensorfile.cpp
  tensorfile.hpp
  top.cpp
  top.hpp
  variable_block_size.cpp)
add_library(bear::bear ALIAS bear)

target_link_libraries(bear PUBLIC VISR::pml_${BEAR_VISR_LIB_TYPE})
target_link_libraries(bear PUBLIC VISR::rcl_${BEAR_VISR_LIB_TYPE})
target_link_libraries(bear PUBLIC VISR::visr_${BEAR_VISR_LIB_TYPE})
target_link_libraries(bear PUBLIC VISR::efl_${BEAR_VISR_LIB_TYPE})
target_link_libraries(bear PUBLIC VISR::rbbl_${BEAR_VISR_LIB_TYPE})
target_link_libraries(bear PUBLIC VISR::rrl_${BEAR_VISR_LIB_TYPE})
target_link_libraries(bear PUBLIC ear)
target_link_libraries(bear PRIVATE Threads::Threads)

# avoids a dependency between bear and the libraries in bear-internals (which
# are just headers which don't need to be installed) when exporting bear in
# static mode
target_include_directories(
  bear PRIVATE $<TARGET_PROPERTY:bear-internals,INTERFACE_INCLUDE_DIRECTORIES>)

target_include_directories(
  bear
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
         $<INSTALL_INTERFACE:include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_property(
  TARGET bear
  PROPERTY PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/bear/api.hpp"
           "${CMAKE_SOURCE_DIR}/include/bear/variable_block_size.hpp")

# In order to create Python wrappers, we set the code to be position-independent
# even if we build a static library.
set_property(TARGET bear PROPERTY POSITION_INDEPENDENT_CODE ON)
# Avoid warnings about inlines with different visibility settings when building
# the Python bindings.
set_property(TARGET bear PROPERTY VISIBILITY_INLINES_HIDDEN ON)

if(BEAR_PACKAGE_AND_INSTALL)
  include(GNUInstallDirs)

  install(
    TARGETS bear
    EXPORT bear-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bear/)

  install(
    EXPORT bear-targets
    FILE bear-targets.cmake
    NAMESPACE bear::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bear)

  include(CMakePackageConfigHelpers)

  configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/bear-config.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/bear-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bear)

  write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/bear-config-version.cmake
    VERSION ${bear_VERSION}
    COMPATIBILITY AnyNewerVersion)

  install(FILES ${CMAKE_BINARY_DIR}/cmake/bear-config.cmake
                ${CMAKE_BINARY_DIR}/cmake/bear-config-version.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bear)
endif()
