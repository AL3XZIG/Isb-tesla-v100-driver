# Shared helpers for test targets.
#
# The repository's tests assert with <cassert>. In Release builds CMake adds
# NDEBUG, which compiles every assert() into a no-op — tests would "pass"
# without checking anything. Every test executable must therefore be built
# with assertions explicitly re-enabled via isb_configure_test_target().

if(NOT COMMAND isb_configure_test_target)
    function(isb_configure_test_target target)
        if(MSVC)
            target_compile_options(${target} PRIVATE /UNDEBUG)
        else()
            target_compile_options(${target} PRIVATE -UNDEBUG)
        endif()
    endfunction()
endif()

# isb_add_test(NAME <name> SOURCES <src...> [LIBS <lib...>])
#
# Registers a CTest executable in one line: creates the target, links it,
# re-enables assertions via isb_configure_test_target() and adds it to CTest.
# Use this instead of hand-rolling the four boilerplate lines per test so no
# target can forget isb_configure_test_target().
if(NOT COMMAND isb_add_test)
    function(isb_add_test)
        cmake_parse_arguments(ISB_TEST "" "NAME" "SOURCES;LIBS" ${ARGN})
        if(NOT ISB_TEST_NAME OR NOT ISB_TEST_SOURCES)
            message(FATAL_ERROR "isb_add_test requires NAME and SOURCES")
        endif()
        add_executable(${ISB_TEST_NAME} ${ISB_TEST_SOURCES})
        if(ISB_TEST_LIBS)
            target_link_libraries(${ISB_TEST_NAME} PRIVATE ${ISB_TEST_LIBS})
        endif()
        isb_configure_test_target(${ISB_TEST_NAME})
        add_test(NAME ${ISB_TEST_NAME} COMMAND ${ISB_TEST_NAME})
    endfunction()
endif()
