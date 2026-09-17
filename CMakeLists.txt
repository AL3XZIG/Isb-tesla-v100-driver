cmake_minimum_required(VERSION 3.16)

project(isb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" ON)

add_subdirectory(cal)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/verification/CMakeLists.txt")
    add_subdirectory(diagnostics/verification)
endif()

add_subdirectory(experiments/benchmarks)

enable_testing()
