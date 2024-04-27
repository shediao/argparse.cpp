cmake_minimum_required(VERSION 3.21)

project(argparse.cpp)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_library(argparse src/argparse.cpp)

add_subdirectory(third_party/googletest)

file(GLOB all_unittest_sources src/*_test.cpp tests/*_test.cpp)

foreach(unittest_file ${all_unittest_sources})
    get_filename_component(unittest_name ${unittest_file} NAME_WE)
    add_executable(${unittest_name} ${unittest_file})

    target_compile_options(${unittest_name} PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_options(${unittest_name} PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_libraries(${unittest_name} argparse gtest gtest_main)
endforeach()


