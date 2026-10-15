add_library(isb-common
    src/version.cpp
    src/capability_state.cpp
    src/json_writer.cpp
)
target_include_directories(isb-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(isb-common PUBLIC cxx_std_17)

if(BUILD_TESTING)
    isb_add_test(NAME isb-common-tests SOURCES tests/version_test.cpp LIBS isb-common)
    isb_add_test(NAME isb-common-error-tests SOURCES tests/error_test.cpp LIBS isb-common)
    isb_add_test(NAME isb-common-observed-tests SOURCES tests/observed_test.cpp LIBS isb-common)
    isb_add_test(NAME isb-common-capability-state-tests SOURCES tests/capability_state_test.cpp LIBS isb-common)
    isb_add_test(NAME isb-common-json-writer-tests SOURCES tests/json_writer_test.cpp LIBS isb-common)

    # Regression guard: this target deliberately keeps NDEBUG to document that
    # plain assert()-based tests become no-ops in Release builds. It must stay
    # outside isb_add_test() because that helper forces -UNDEBUG on purpose.
    add_executable(isb-common-error-release-tests tests/error_test.cpp)
    target_compile_definitions(isb-common-error-release-tests PRIVATE NDEBUG)
    target_link_libraries(isb-common-error-release-tests PRIVATE isb-common)
    add_test(NAME isb-common-error-release-tests COMMAND isb-common-error-release-tests)
endif()
