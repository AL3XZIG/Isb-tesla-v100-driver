add_library(isb-optiscaler-integration
    src/manager.cpp
)

target_include_directories(isb-optiscaler-integration
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(isb-optiscaler-integration PUBLIC cxx_std_17)

if(BUILD_TESTING)
    add_executable(isb-optiscaler-integration-tests
        tests/manager_test.cpp
    )
    target_link_libraries(isb-optiscaler-integration-tests
        PRIVATE isb-optiscaler-integration
    )
    add_test(NAME isb-optiscaler-integration-tests
        COMMAND isb-optiscaler-integration-tests)
endif()
