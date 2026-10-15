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
