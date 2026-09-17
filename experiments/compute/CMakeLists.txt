add_library(isb-experimental-compute
    src/device.cpp
    src/report.cpp
)

add_library(isb::experimental-compute ALIAS isb-experimental-compute)

target_include_directories(isb-experimental-compute
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(isb-experimental-compute PUBLIC isb-cal)
target_compile_features(isb-experimental-compute PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(isb-experimental-compute PRIVATE /W4 /permissive-)
else()
    target_compile_options(isb-experimental-compute PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(isb-experimental-compute-tests
        tests/compute_test.cpp
    )
    target_link_libraries(isb-experimental-compute-tests PRIVATE isb-experimental-compute)
    if(MSVC)
        target_compile_options(isb-experimental-compute-tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(isb-experimental-compute-tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME isb-experimental-compute-tests COMMAND isb-experimental-compute-tests)
endif()
