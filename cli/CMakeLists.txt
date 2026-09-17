add_executable(isb main.cpp)
target_link_libraries(isb PRIVATE isb-hub)
target_compile_features(isb PRIVATE cxx_std_17)
