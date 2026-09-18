add_library(isb-hub src/hub.cpp)
target_include_directories(isb-hub PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(isb-hub PUBLIC isb-cal)
target_compile_features(isb-hub PUBLIC cxx_std_17)
if(BUILD_TESTING)
 add_executable(isb-hub-tests tests/hub_test.cpp)
 target_link_libraries(isb-hub-tests PRIVATE isb-hub)
 add_test(NAME isb-hub-tests COMMAND isb-hub-tests)
endif()


# Real runtime aggregation keeps provider-specific APIs out of frontends.
add_library(isb-hub-runtime src/runtime_provider.cpp)
target_include_directories(isb-hub-runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(isb-hub-runtime PUBLIC isb-hub isb-nvml isb-cuda isb-vulkan)
target_compile_features(isb-hub-runtime PUBLIC cxx_std_17)

if(BUILD_TESTING)
 add_executable(isb-hub-runtime-tests tests/runtime_provider_test.cpp)
 target_link_libraries(isb-hub-runtime-tests PRIVATE isb-hub-runtime)
 add_test(NAME isb-hub-runtime-tests COMMAND isb-hub-runtime-tests)
endif()
