# CMake generated Testfile for 
# Source directory: /workspace/hub
# Build directory: /workspace/build-ci/hub
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(isb-hub-tests "/workspace/build-ci/hub/isb-hub-tests")
set_tests_properties(isb-hub-tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/hub/CMakeLists.txt;8;add_test;/workspace/hub/CMakeLists.txt;0;")
add_test(isb-hub-runtime-tests "/workspace/build-ci/hub/isb-hub-runtime-tests")
set_tests_properties(isb-hub-runtime-tests PROPERTIES  _BACKTRACE_TRIPLES "/workspace/hub/CMakeLists.txt;21;add_test;/workspace/hub/CMakeLists.txt;0;")
