add_library(isb-common
    src/version.cpp
    src/capability_state.cpp
)
target_include_directories(isb-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(isb-common PUBLIC cxx_std_17)

if(BUILD_TESTING)
    add_executable(isb-common-tests tests/version_test.cpp)
    target_link_libraries(isb-common-tests PRIVATE isb-common)
    add_test(NAME isb-common-tests COMMAND isb-common-tests)

    add_executable(isb-common-error-tests tests/error_test.cpp)
    target_link_libraries(isb-common-error-tests PRIVATE isb-common)
    add_test(NAME isb-common-error-tests COMMAND isb-common-error-tests)

    add_executable(isb-common-error-release-tests tests/error_test.cpp)
    target_compile_definitions(isb-common-error-release-tests PRIVATE NDEBUG)
    target_link_libraries(isb-common-error-release-tests PRIVATE isb-common)
    add_test(NAME isb-common-error-release-tests COMMAND isb-common-error-release-tests)

    add_executable(isb-common-observed-tests tests/observed_test.cpp)
    target_link_libraries(isb-common-observed-tests PRIVATE isb-common)
    add_test(NAME isb-common-observed-tests COMMAND isb-common-observed-tests)

    add_executable(isb-common-capability-state-tests tests/capability_state_test.cpp)
    target_link_libraries(isb-common-capability-state-tests PRIVATE isb-common)
    add_test(NAME isb-common-capability-state-tests COMMAND isb-common-capability-state-tests)
endif()
