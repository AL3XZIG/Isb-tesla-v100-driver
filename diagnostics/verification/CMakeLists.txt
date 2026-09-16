cmake_minimum_required(VERSION 3.16)

project(isb_verification VERSION 1.0.0 LANGUAGES CXX)

add_library(isb-verification
    src/assertions.cpp
    src/json.cpp
    src/runner.cpp
    src/snapshot.cpp
    src/types.cpp
)

target_include_directories(isb-verification
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(isb-verification PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(isb-verification PRIVATE /W4 /permissive-)
else()
    target_compile_options(isb-verification PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(isb-verification-tests
        tests/verification_test.cpp
    )
    target_link_libraries(isb-verification-tests PRIVATE isb-verification)

    if(MSVC)
        target_compile_options(isb-verification-tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(isb-verification-tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME isb-verification-tests COMMAND isb-verification-tests)
    set_tests_properties(isb-verification-tests PROPERTIES TIMEOUT 10)
endif()
