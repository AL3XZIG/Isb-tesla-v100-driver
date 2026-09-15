cmake_minimum_required(VERSION 3.16)

project(isb_cal VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(isb-cal
    src/capabilities.cpp
)

target_include_directories(isb-cal
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(isb-cal PUBLIC cxx_std_17)

include(CTest)
if(BUILD_TESTING)
    add_executable(isb-cal-tests
        tests/capabilities_test.cpp
    )
    target_link_libraries(isb-cal-tests PRIVATE isb-cal)
    add_test(NAME isb-cal-tests COMMAND isb-cal-tests)
endif()
