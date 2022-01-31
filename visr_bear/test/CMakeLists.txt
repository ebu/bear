include(${PROJECT_SOURCE_DIR}/submodules/catch2.cmake)

if(NOT BEAR_DATA_OUTPUT_PATH_DEFAULT)
  message(
    FATAL_ERROR "data file required for tests not found; "
                "set BEAR_DOWNLOAD_DATA_DEFAULT or BEAR_DATA_PATH_DEFAULT")
endif()

set(BUNDLED_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/files/")
set(GENERATED_TEST_FILES "${CMAKE_CURRENT_BINARY_DIR}/files/")
set(DEFAULT_TENSORFILE_NAME "${BEAR_DATA_OUTPUT_PATH_DEFAULT}")
configure_file(test_config.h.in test_config.h)

function(add_visr_bear_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE bear bear-internals catch2)
  # for including test_config.h
  target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
endfunction()

add_visr_bear_test(test_tensorfile)
add_visr_bear_test(test_panner)
add_visr_bear_test(test_renderer)
add_visr_bear_test(test_variable_block_size)
add_visr_bear_test(test_sh_rotation)
add_visr_bear_test(test_dynamic_renderer)

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE bear bear-internals)
# for including test_config.h
target_include_directories(benchmark PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
