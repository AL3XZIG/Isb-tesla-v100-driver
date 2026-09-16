cmake_minimum_required(VERSION 3.16)

project(isb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" ON)

add_subdirectory(cal)
add_subdirectory(diagnostics/verification)

enable_testing()
