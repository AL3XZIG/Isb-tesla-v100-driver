# CLI presentation library
add_library(isb-cli-presentation STATIC
    presentation/formatter.cpp
)
target_include_directories(isb-cli-presentation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(isb-cli-presentation PUBLIC isb-hub)

# Main CLI executable. Runtime aggregation is the default; mock mode is explicit.
add_executable(isb main.cpp)
target_link_libraries(isb PRIVATE isb-hub-runtime isb-graphics-upscaler isb-cli-presentation)

if(BUILD_TESTING)
    add_executable(isb-cli-provider-selection-tests tests/provider_selection_test.cpp)
    target_link_libraries(isb-cli-provider-selection-tests PRIVATE isb-hub-runtime)
    isb_configure_test_target(isb-cli-provider-selection-tests)
    add_test(NAME isb-cli-provider-selection-tests COMMAND isb-cli-provider-selection-tests)
endif()
