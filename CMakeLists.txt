cmake_minimum_required(VERSION 3.16)

project(argparse.cpp)

set(CMAKE_CXX_STANDARD 17)
if(LINUX OR APPLE)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
   OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
   OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -O0 -g")
endif()

include_directories(include)

add_library(argparse INTERFACE)
add_executable(example src/example.cpp)
target_link_libraries(example PRIVATE argparse)

add_subdirectory(third_party/googletest)

file(GLOB all_unittest_sources src/*_test.cpp tests/*_test.cpp)

enable_testing()
foreach(unittest_file ${all_unittest_sources})
  get_filename_component(unittest_name ${unittest_file} NAME_WE)
  add_executable(${unittest_name} ${unittest_file})

  if(LINUX OR APPLE)
    target_compile_options(${unittest_name} PRIVATE -fsanitize=address
                                                    -fsanitize=undefined)
    target_link_options(${unittest_name} PRIVATE -fsanitize=address
                        -fsanitize=undefined)
  endif()
  target_link_libraries(${unittest_name} argparse gtest gtest_main)
  add_test(NAME ${unittest_name} COMMAND ${unittest_name})
endforeach()
