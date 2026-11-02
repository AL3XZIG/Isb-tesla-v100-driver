# CLI presentation layer
add_library(isb-cli-presentation
    presentation/formatter.cpp
)

target_include_directories(isb-cli-presentation
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(isb-cli-presentation
    PUBLIC isb-common
)

target_compile_features(isb-cli-presentation PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(isb-cli-presentation PRIVATE /W4 /permissive-)
else()
    target_compile_options(isb-cli-presentation PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Main CLI executable. Runtime aggregation is the default; mock is explicit.
add_executable(isb main.cpp)
target_link_libraries(isb PRIVATE isb-hub-runtime isb-graphics-upscaler isb-cli-presentation)
target_compile_features(isb PRIVATE cxx_std_17)

if(BUILD_TESTING)
    add_executable(cli-tests tests/cli_test.cpp)
    target_link_libraries(cli-tests PRIVATE isb-cli-presentation isb-hub)
    isb_configure_test_target(cli-tests)
    add_test(NAME cli-tests COMMAND cli-tests)

    add_executable(isb-cli-provider-selection-tests tests/provider_selection_test.cpp)
    target_link_libraries(isb-cli-provider-selection-tests PRIVATE isb-hub-runtime)
    isb_configure_test_target(isb-cli-provider-selection-tests)
    add_test(NAME isb-cli-provider-selection-tests COMMAND isb-cli-provider-selection-tests)
endif()
