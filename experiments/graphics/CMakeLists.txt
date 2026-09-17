add_library(isb-graphics-capabilities
    src/adapter.cpp
    src/report.cpp
)

target_include_directories(isb-graphics-capabilities
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(isb-graphics-capabilities PUBLIC isb-cal)
target_compile_features(isb-graphics-capabilities PUBLIC cxx_std_17)

if(UNIX AND NOT APPLE)
    target_link_libraries(isb-graphics-capabilities PRIVATE dl)
endif()

if(MSVC)
    target_compile_options(isb-graphics-capabilities PRIVATE /W4 /permissive-)
else()
    target_compile_options(isb-graphics-capabilities PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(graphics-capability-tests tests/capability_test.cpp)
    target_link_libraries(graphics-capability-tests PRIVATE isb-graphics-capabilities)
    if(MSVC)
        target_compile_options(graphics-capability-tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(graphics-capability-tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME graphics-capability-tests COMMAND graphics-capability-tests)
endif()
