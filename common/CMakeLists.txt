add_library(isb-common src/version.cpp)
target_include_directories(isb-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(isb-common PUBLIC cxx_std_17)

if(BUILD_TESTING)
    add_executable(isb-common-tests tests/version_test.cpp)
    target_link_libraries(isb-common-tests PRIVATE isb-common)
    add_test(NAME isb-common-tests COMMAND isb-common-tests)

    add_executable(isb-common-observed-tests tests/observed_test.cpp)
    target_link_libraries(isb-common-observed-tests PRIVATE isb-common)
    add_test(NAME isb-common-observed-tests COMMAND isb-common-observed-tests)
endif()
