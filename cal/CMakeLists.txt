cmake_minimum_required(VERSION 3.16)

project(isb_cal VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ISB_CAL_ENABLE_WARNINGS "Enable strict compiler warnings for CAL" ON)

add_library(isb-cal
    src/capabilities.cpp
)

target_include_directories(isb-cal
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(isb-cal PUBLIC cxx_std_17)

if(ISB_CAL_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(isb-cal PRIVATE /W4 /permissive-)
    else()
        target_compile_options(isb-cal PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(isb-cal-tests
        tests/capabilities_test.cpp
    )
    target_link_libraries(isb-cal-tests PRIVATE isb-cal)
    if(ISB_CAL_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(isb-cal-tests PRIVATE /W4 /permissive-)
        else()
            target_compile_options(isb-cal-tests PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
    add_test(NAME isb-cal-tests COMMAND isb-cal-tests)
endif()
