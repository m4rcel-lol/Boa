# CMake generated Testfile for 
# Source directory: /home/runner/work/Boa/Boa
# Build directory: /home/runner/work/Boa/Boa/_codeql_build_dir
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(boa_unit_tests "/home/runner/work/Boa/Boa/_codeql_build_dir/boa_tests")
set_tests_properties(boa_unit_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;16;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_hello "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/hello.boa")
set_tests_properties(integration_hello PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;19;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_fibonacci "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/fibonacci.boa")
set_tests_properties(integration_fibonacci PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;23;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_loops "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/loops.boa")
set_tests_properties(integration_loops PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;27;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_lists "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/lists.boa")
set_tests_properties(integration_lists PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;31;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_calculator "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/calculator.boa")
set_tests_properties(integration_calculator PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;35;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_string_ops "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/string_ops.boa")
set_tests_properties(integration_string_ops PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;39;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_error_handling "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/error_handling.boa")
set_tests_properties(integration_error_handling PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;43;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_dict "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/dict.boa")
set_tests_properties(integration_dict PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;47;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_recursion "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/recursion.boa")
set_tests_properties(integration_recursion PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;51;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
add_test(integration_scope "/home/runner/work/Boa/Boa/_codeql_build_dir/boa" "/home/runner/work/Boa/Boa/examples/scope.boa")
set_tests_properties(integration_scope PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/Boa/Boa/CMakeLists.txt;55;add_test;/home/runner/work/Boa/Boa/CMakeLists.txt;0;")
