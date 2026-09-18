add_executable(isb main.cpp)
target_link_libraries(isb PRIVATE isb-hub isb-graphics-upscaler)
target_compile_features(isb PRIVATE cxx_std_17)
