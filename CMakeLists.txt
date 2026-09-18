cmake_minimum_required(VERSION 3.16)

project(isb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" ON)
include(CTest)

enable_testing()

function(isb_add_component directory)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${directory}/CMakeLists.txt")
        add_subdirectory("${directory}")
    endif()
endfunction()

# Core and stable user-facing components.
isb_add_component(common)
isb_add_component(cal)
isb_add_component(providers/nvml)
isb_add_component(providers/vulkan)
isb_add_component(providers/cuda)

# Graphics upscaling/render-path layer.
isb_add_component(graphics)

isb_add_component(hub)
isb_add_component(cli)
isb_add_component(control-center)

# Experimental components are optional at the directory level: they are only
# added when their own CMakeLists.txt is present.
isb_add_component(experiments/graphics)
isb_add_component(experiments/compute)
isb_add_component(experiments/benchmarks)

# Verification is optional because some repository revisions do not contain a
# buildable verification CMake target yet.
isb_add_component(diagnostics/verification)
