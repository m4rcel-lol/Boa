cmake_minimum_required(VERSION 3.16)
project(Boa VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main interpreter executable
add_executable(boa boa-lang/src/main.cpp)
target_include_directories(boa PRIVATE boa-lang/include)

# Tests
enable_testing()

add_executable(boa_tests tests/unit/test_main.cpp)
target_include_directories(boa_tests PRIVATE boa-lang/include)
add_test(NAME boa_unit_tests COMMAND boa_tests)

# Integration tests (run example scripts)
add_test(
    NAME integration_hello
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/hello.boa
)
add_test(
    NAME integration_fibonacci
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/fibonacci.boa
)
add_test(
    NAME integration_loops
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/loops.boa
)
add_test(
    NAME integration_lists
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/lists.boa
)
add_test(
    NAME integration_calculator
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/calculator.boa
)
add_test(
    NAME integration_string_ops
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/string_ops.boa
)
add_test(
    NAME integration_error_handling
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/error_handling.boa
)
add_test(
    NAME integration_dict
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/dict.boa
)
add_test(
    NAME integration_recursion
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/recursion.boa
)
add_test(
    NAME integration_scope
    COMMAND boa ${CMAKE_SOURCE_DIR}/examples/scope.boa
)

# Install rules
install(TARGETS boa DESTINATION bin)
