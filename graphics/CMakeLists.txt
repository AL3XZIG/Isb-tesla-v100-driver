cmake_minimum_required(VERSION 3.16)

# Graphics upscaling backend - interface and adapters
add_library(isb-graphics-upscaler
    src/upscaler_types.cpp
    src/upscaler_manager.cpp
    src/render_path_types.cpp
    src/render_path_manager.cpp
    src/render_path_platform.cpp
)

target_include_directories(isb-graphics-upscaler
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(isb-graphics-upscaler
    PUBLIC isb-common isb-cal isb-hub
    PRIVATE isb-nvml isb-vulkan
)

target_compile_features(isb-graphics-upscaler PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(isb-graphics-upscaler PRIVATE /W4 /permissive-)
else()
    target_compile_options(isb-graphics-upscaler PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: link against Vulkan if available
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    target_compile_definitions(isb-graphics-upscaler PUBLIC ISB_VULKAN_AVAILABLE)
    target_link_libraries(isb-graphics-upscaler PRIVATE Vulkan::Vulkan)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(upscaler-tests tests/upscaler_test.cpp)
    target_link_libraries(upscaler-tests PRIVATE isb-graphics-upscaler)
    add_test(NAME upscaler-tests COMMAND upscaler-tests)
    
    add_executable(render-path-tests tests/render_path_test.cpp)
    target_link_libraries(render-path-tests PRIVATE isb-graphics-upscaler)
    add_test(NAME render-path-tests COMMAND render-path-tests)
endif()

if(WIN32)
    target_link_libraries(isb-graphics-upscaler PRIVATE dxgi)
endif()
