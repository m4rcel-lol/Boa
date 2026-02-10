// Boa Language - Unit Tests
// Simple test framework without external dependencies

#include "boa/interpreter.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    static void test_##name(); \
    struct TestReg_##name { \
        TestReg_##name() { \
            tests_run++; \
            std::cout << "  " << #name << " ... "; \
            try { test_##name(); tests_passed++; std::cout << "PASSED\n"; } \
            catch (const std::exception& e) { tests_failed++; std::cout << "FAILED: " << e.what() << "\n"; } \
            catch (...) { tests_failed++; std::cout << "FAILED: unknown exception\n"; } \
        } \
    } test_reg_##name; \
    static void test_##name()

#define EXPECT_EQ(a, b) \
    do { auto _a = (a); auto _b = (b); \
    if (_a != _b) { \
        throw std::runtime_error(std::string("Assertion failed at line ") + std::to_string(__LINE__)); \
    }} while(0)

#define EXPECT_THROW(expr) \
    do { bool caught = false; \
    try { expr; } catch (...) { caught = true; } \
    if (!caught) throw std::runtime_error("Expected exception at line " + std::to_string(__LINE__)); \
    } while(0)

// ===========================================================================
// Tokenizer tests
// ===========================================================================
TEST(tokenizer_basic) {
    boa::Lexer lexer("x = 42\n");
    auto tokens = lexer.tokenize();
    // Should have: Identifier("x"), Eq, Int("42"), Newline, Eof
    assert(tokens.size() >= 4);
    EXPECT_EQ(tokens[0].type, boa::TokenType::Identifier);
    EXPECT_EQ(tokens[0].value, "x");
    EXPECT_EQ(tokens[1].type, boa::TokenType::Eq);
    EXPECT_EQ(tokens[2].type, boa::TokenType::Int);
    EXPECT_EQ(tokens[2].value, "42");
}

TEST(tokenizer_keywords) {
    boa::Lexer lexer("fn if elif else for in while imp ret\n");
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens[0].type, boa::TokenType::Fn);
    EXPECT_EQ(tokens[1].type, boa::TokenType::If);
    EXPECT_EQ(tokens[2].type, boa::TokenType::Elif);
    EXPECT_EQ(tokens[3].type, boa::TokenType::Else);
    EXPECT_EQ(tokens[4].type, boa::TokenType::For);
    EXPECT_EQ(tokens[5].type, boa::TokenType::In);
    EXPECT_EQ(tokens[6].type, boa::TokenType::While);
    EXPECT_EQ(tokens[7].type, boa::TokenType::Imp);
    EXPECT_EQ(tokens[8].type, boa::TokenType::Ret);
}

TEST(tokenizer_string) {
    boa::Lexer lexer("\"hello world\"\n");
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens[0].type, boa::TokenType::String);
    EXPECT_EQ(tokens[0].value, "hello world");
}

TEST(tokenizer_operators) {
    boa::Lexer lexer("+ - * / % ** == != < <= > >= = += -= *= /=\n");
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens[0].type, boa::TokenType::Plus);
    EXPECT_EQ(tokens[1].type, boa::TokenType::Minus);
    EXPECT_EQ(tokens[2].type, boa::TokenType::Star);
    EXPECT_EQ(tokens[3].type, boa::TokenType::Slash);
    EXPECT_EQ(tokens[4].type, boa::TokenType::Percent);
    EXPECT_EQ(tokens[5].type, boa::TokenType::DoubleStar);
    EXPECT_EQ(tokens[6].type, boa::TokenType::EqEq);
    EXPECT_EQ(tokens[7].type, boa::TokenType::BangEq);
}

TEST(tokenizer_indent) {
    boa::Lexer lexer("if true:\n    x = 1\n");
    auto tokens = lexer.tokenize();
    // Check that INDENT is produced after the colon + newline
    bool found_indent = false;
    for (auto& t : tokens) {
        if (t.type == boa::TokenType::Indent) found_indent = true;
    }
    assert(found_indent);
}

TEST(tokenizer_comment) {
    boa::Lexer lexer("x = 1 # this is a comment\n");
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens[0].type, boa::TokenType::Identifier);
    EXPECT_EQ(tokens[1].type, boa::TokenType::Eq);
    EXPECT_EQ(tokens[2].type, boa::TokenType::Int);
}

// ===========================================================================
// Parser tests
// ===========================================================================
TEST(parser_simple_assignment) {
    boa::Lexer lexer("x = 42\n");
    auto tokens = lexer.tokenize();
    boa::Parser parser(tokens);
    auto program = parser.parse_program();
    assert(program->statements.size() == 1);
}

TEST(parser_function_def) {
    std::string src = "fn add(a, b):\n    a + b\n";
    boa::Lexer lexer(src);
    auto tokens = lexer.tokenize();
    boa::Parser parser(tokens);
    auto program = parser.parse_program();
    assert(program->statements.size() == 1);
    auto* fn = dynamic_cast<boa::FnDef*>(program->statements[0].get());
    assert(fn != nullptr);
    EXPECT_EQ(fn->name, "add");
    EXPECT_EQ(fn->params.size(), static_cast<size_t>(2));
}

TEST(parser_if_stmt) {
    std::string src = "if x > 0:\n    x\n";
    boa::Lexer lexer(src);
    auto tokens = lexer.tokenize();
    boa::Parser parser(tokens);
    auto program = parser.parse_program();
    assert(program->statements.size() == 1);
    assert(dynamic_cast<boa::IfStmt*>(program->statements[0].get()) != nullptr);
}

TEST(parser_for_stmt) {
    std::string src = "for i in range(10):\n    i\n";
    boa::Lexer lexer(src);
    auto tokens = lexer.tokenize();
    boa::Parser parser(tokens);
    auto program = parser.parse_program();
    assert(program->statements.size() == 1);
    assert(dynamic_cast<boa::ForStmt*>(program->statements[0].get()) != nullptr);
}

// ===========================================================================
// Interpreter tests
// ===========================================================================
TEST(interp_arithmetic) {
    EXPECT_EQ(boa::run_and_capture("print(2 + 3)\n"), "5\n");
    EXPECT_EQ(boa::run_and_capture("print(10 - 3)\n"), "7\n");
    EXPECT_EQ(boa::run_and_capture("print(4 * 5)\n"), "20\n");
    EXPECT_EQ(boa::run_and_capture("print(10 / 3)\n"), "3\n");
    EXPECT_EQ(boa::run_and_capture("print(10 % 3)\n"), "1\n");
    EXPECT_EQ(boa::run_and_capture("print(2 ** 10)\n"), "1024\n");
}

TEST(interp_variables) {
    EXPECT_EQ(boa::run_and_capture("x = 42\nprint(x)\n"), "42\n");
    EXPECT_EQ(boa::run_and_capture("x = 1\nx += 2\nprint(x)\n"), "3\n");
}

TEST(interp_strings) {
    EXPECT_EQ(boa::run_and_capture("print(\"hello\" + \" world\")\n"), "hello world\n");
    EXPECT_EQ(boa::run_and_capture("print(\"ab\" * 3)\n"), "ababab\n");
    EXPECT_EQ(boa::run_and_capture("print(len(\"hello\"))\n"), "5\n");
}

TEST(interp_bool_ops) {
    EXPECT_EQ(boa::run_and_capture("print(true and false)\n"), "false\n");
    EXPECT_EQ(boa::run_and_capture("print(true or false)\n"), "true\n");
    EXPECT_EQ(boa::run_and_capture("print(not true)\n"), "false\n");
}

TEST(interp_comparisons) {
    EXPECT_EQ(boa::run_and_capture("print(1 < 2)\n"), "true\n");
    EXPECT_EQ(boa::run_and_capture("print(2 <= 2)\n"), "true\n");
    EXPECT_EQ(boa::run_and_capture("print(3 > 2)\n"), "true\n");
    EXPECT_EQ(boa::run_and_capture("print(2 == 2)\n"), "true\n");
    EXPECT_EQ(boa::run_and_capture("print(2 != 3)\n"), "true\n");
}

TEST(interp_if_stmt) {
    std::string src =
        "x = 10\n"
        "if x > 5:\n"
        "    print(\"big\")\n"
        "else:\n"
        "    print(\"small\")\n";
    EXPECT_EQ(boa::run_and_capture(src), "big\n");
}

TEST(interp_if_elif_else) {
    std::string src =
        "x = 5\n"
        "if x > 10:\n"
        "    print(\"a\")\n"
        "elif x > 3:\n"
        "    print(\"b\")\n"
        "else:\n"
        "    print(\"c\")\n";
    EXPECT_EQ(boa::run_and_capture(src), "b\n");
}

TEST(interp_for_loop) {
    std::string src =
        "for i in range(5):\n"
        "    print(i)\n";
    EXPECT_EQ(boa::run_and_capture(src), "0\n1\n2\n3\n4\n");
}

TEST(interp_while_loop) {
    std::string src =
        "x = 0\n"
        "while x < 3:\n"
        "    print(x)\n"
        "    x += 1\n";
    EXPECT_EQ(boa::run_and_capture(src), "0\n1\n2\n");
}

TEST(interp_function) {
    std::string src =
        "fn add(a, b):\n"
        "    a + b\n"
        "print(add(3, 4))\n";
    EXPECT_EQ(boa::run_and_capture(src), "7\n");
}

TEST(interp_function_return) {
    std::string src =
        "fn max_val(a, b):\n"
        "    if a > b:\n"
        "        ret a\n"
        "    ret b\n"
        "print(max_val(3, 7))\n";
    EXPECT_EQ(boa::run_and_capture(src), "7\n");
}

TEST(interp_recursion) {
    std::string src =
        "fn fib(n):\n"
        "    if n < 2:\n"
        "        n\n"
        "    else:\n"
        "        fib(n - 1) + fib(n - 2)\n"
        "print(fib(10))\n";
    EXPECT_EQ(boa::run_and_capture(src), "55\n");
}

TEST(interp_list_ops) {
    EXPECT_EQ(boa::run_and_capture("x = [1, 2, 3]\nprint(len(x))\n"), "3\n");
    EXPECT_EQ(boa::run_and_capture("x = [1, 2, 3]\nprint(x[0])\n"), "1\n");
    EXPECT_EQ(boa::run_and_capture("x = [1, 2, 3]\nprint(x[-1])\n"), "3\n");
}

TEST(interp_dict_ops) {
    std::string src =
        "d = {\"a\": 1, \"b\": 2}\n"
        "print(d[\"a\"])\n";
    EXPECT_EQ(boa::run_and_capture(src), "1\n");
}

TEST(interp_io_module) {
    std::string src =
        "imp io\n"
        "io.print(\"Hello, Boa!\")\n";
    EXPECT_EQ(boa::run_and_capture(src), "Hello, Boa!\n");
}

TEST(interp_nested_functions) {
    std::string src =
        "fn outer(x):\n"
        "    fn inner(y):\n"
        "        x + y\n"
        "    inner(10)\n"
        "print(outer(5))\n";
    EXPECT_EQ(boa::run_and_capture(src), "15\n");
}

TEST(interp_scope) {
    std::string src =
        "x = 1\n"
        "fn f():\n"
        "    x = 2\n"
        "    x\n"
        "print(f())\n"
        "print(x)\n";
    // f() modifies x in the outer scope (Python-like assignment)
    // Actually with our environment.set, it finds x in parent and updates it
    EXPECT_EQ(boa::run_and_capture(src), "2\n2\n");
}

TEST(interp_try_except) {
    std::string src =
        "try:\n"
        "    x = 1 / 0\n"
        "except e:\n"
        "    print(\"caught\")\n";
    EXPECT_EQ(boa::run_and_capture(src), "caught\n");
}

TEST(interp_builtin_type) {
    EXPECT_EQ(boa::run_and_capture("print(type(42))\n"), "int\n");
    EXPECT_EQ(boa::run_and_capture("print(type(3.14))\n"), "float\n");
    EXPECT_EQ(boa::run_and_capture("print(type(\"hi\"))\n"), "string\n");
    EXPECT_EQ(boa::run_and_capture("print(type(true))\n"), "bool\n");
    EXPECT_EQ(boa::run_and_capture("print(type(none))\n"), "none\n");
}

TEST(interp_builtin_conversions) {
    EXPECT_EQ(boa::run_and_capture("print(int(3.7))\n"), "3\n");
    EXPECT_EQ(boa::run_and_capture("print(float(3))\n"), "3\n");
    EXPECT_EQ(boa::run_and_capture("print(str(42))\n"), "42\n");
}

TEST(interp_list_append) {
    std::string src =
        "x = [1, 2]\n"
        "append(x, 3)\n"
        "print(x)\n";
    EXPECT_EQ(boa::run_and_capture(src), "[1, 2, 3]\n");
}

TEST(interp_multiple_print_args) {
    EXPECT_EQ(boa::run_and_capture("print(1, 2, 3)\n"), "1 2 3\n");
}

TEST(interp_unary_ops) {
    EXPECT_EQ(boa::run_and_capture("print(-5)\n"), "-5\n");
    EXPECT_EQ(boa::run_and_capture("print(+5)\n"), "5\n");
}

TEST(interp_undefined_var_error) {
    EXPECT_THROW(boa::run_and_capture("print(undefined_var)\n"));
}

TEST(interp_division_by_zero) {
    EXPECT_THROW(boa::run_and_capture("x = 1 / 0\n"));
}

TEST(interp_wrong_arg_count) {
    std::string src =
        "fn f(a, b):\n"
        "    a + b\n"
        "f(1)\n";
    EXPECT_THROW(boa::run_and_capture(src));
}

TEST(interp_range_function) {
    EXPECT_EQ(boa::run_and_capture("print(range(5))\n"), "[0, 1, 2, 3, 4]\n");
    EXPECT_EQ(boa::run_and_capture("print(range(2, 5))\n"), "[2, 3, 4]\n");
    EXPECT_EQ(boa::run_and_capture("print(range(0, 10, 3))\n"), "[0, 3, 6, 9]\n");
}

TEST(interp_pass_stmt) {
    std::string src =
        "fn empty():\n"
        "    pass\n"
        "empty()\n"
        "print(\"ok\")\n";
    EXPECT_EQ(boa::run_and_capture(src), "ok\n");
}

TEST(interp_string_index) {
    EXPECT_EQ(boa::run_and_capture("print(\"hello\"[0])\n"), "h\n");
    EXPECT_EQ(boa::run_and_capture("print(\"hello\"[4])\n"), "o\n");
}

TEST(interp_list_modify) {
    std::string src =
        "x = [1, 2, 3]\n"
        "x[1] = 20\n"
        "print(x)\n";
    EXPECT_EQ(boa::run_and_capture(src), "[1, 20, 3]\n");
}

TEST(interp_negative_index) {
    EXPECT_EQ(boa::run_and_capture("print([10, 20, 30][-1])\n"), "30\n");
    EXPECT_EQ(boa::run_and_capture("print([10, 20, 30][-2])\n"), "20\n");
}

TEST(interp_float_arithmetic) {
    EXPECT_EQ(boa::run_and_capture("print(1.5 + 2.5)\n"), "4\n");
    EXPECT_EQ(boa::run_and_capture("print(3.0 * 2.0)\n"), "6\n");
}

TEST(interp_comparison_mixed_types) {
    EXPECT_EQ(boa::run_and_capture("print(1 == 1.0)\n"), "true\n");
    EXPECT_EQ(boa::run_and_capture("print(2 > 1.5)\n"), "true\n");
}

// ===========================================================================
// Main
// ===========================================================================
int main() {
    std::cout << "Boa Language Test Suite\n";
    std::cout << "======================\n\n";

    // Tests are auto-registered via static objects above
    // (they run before main() body)

    std::cout << "\n======================\n";
    std::cout << "Results: " << tests_passed << " passed, "
              << tests_failed << " failed, "
              << tests_run << " total\n";

    return tests_failed > 0 ? 1 : 0;
}
