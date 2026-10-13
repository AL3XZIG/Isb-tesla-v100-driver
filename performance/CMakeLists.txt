add_library(isb-performance
    src/telemetry.cpp
    src/throttling.cpp
)
target_include_directories(isb-performance PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(isb-performance PUBLIC isb-hub)
target_compile_features(isb-performance PUBLIC cxx_std_17)
