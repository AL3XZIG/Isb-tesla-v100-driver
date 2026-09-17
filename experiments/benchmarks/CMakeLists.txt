add_library(isb-benchmarks
    src/benchmark.cpp
    src/report.cpp
)

target_include_directories(isb-benchmarks PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(isb-benchmarks PUBLIC cxx_std_17)

find_package(CUDAToolkit QUIET)
if(CUDAToolkit_FOUND)
    target_link_libraries(isb-benchmarks PRIVATE CUDA::cudart)
    target_compile_definitions(isb-benchmarks PRIVATE ISB_BENCHMARKS_HAS_CUDA=1)
endif()

if(MSVC)
    target_compile_options(isb-benchmarks PRIVATE /W4 /permissive-)
else()
    target_compile_options(isb-benchmarks PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(isb-benchmarks-tests tests/benchmark_test.cpp)
    target_link_libraries(isb-benchmarks-tests PRIVATE isb-benchmarks)
    add_test(NAME isb-benchmarks-tests COMMAND isb-benchmarks-tests)
endif()
