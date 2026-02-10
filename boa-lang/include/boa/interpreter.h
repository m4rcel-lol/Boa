#pragma once

#include "ast.h"
#include "parser.h"
#include "token.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace boa {

// Forward declarations
class Environment;
class Interpreter;

using EnvPtr = std::shared_ptr<Environment>;

// ============================================================================
// Runtime errors
// ============================================================================
struct BoaRuntimeError : std::runtime_error {
    int line;
    int column;
    BoaRuntimeError(const std::string& msg, int line = 0, int col = 0)
        : std::runtime_error(msg), line(line), column(col) {}
};

// ============================================================================
// Value types
// ============================================================================
enum class ValueType {
    None, Bool, Int, Float, String, List, Dict,
    Function, BuiltinFunction, Module
};

inline std::string value_type_name(ValueType t) {
    switch (t) {
        case ValueType::None:     return "none";
        case ValueType::Bool:     return "bool";
        case ValueType::Int:      return "int";
        case ValueType::Float:    return "float";
        case ValueType::String:   return "string";
        case ValueType::List:     return "list";
        case ValueType::Dict:     return "dict";
        case ValueType::Function: return "function";
        case ValueType::BuiltinFunction: return "builtin_function";
        case ValueType::Module:   return "module";
    }
    return "unknown";
}

struct BoaValue;
using BoaValuePtr = std::shared_ptr<BoaValue>;
using BuiltinFn = std::function<BoaValuePtr(std::vector<BoaValuePtr>)>;

struct BoaFunction {
    std::string name;
    std::vector<std::string> params;
    std::vector<ASTNode*> body;  // non-owning; owned by AST
    EnvPtr closure;
};

struct BoaModule {
    std::string name;
    std::unordered_map<std::string, BoaValuePtr> members;
};

struct BoaValue {
    ValueType type;
    bool bool_val = false;
    int64_t int_val = 0;
    double float_val = 0.0;
    std::string string_val;
    std::vector<BoaValuePtr> list_val;
    std::vector<std::pair<BoaValuePtr, BoaValuePtr>> dict_val;
    BoaFunction func_val;
    BuiltinFn builtin_val;
    BoaModule module_val;

    static BoaValuePtr make_none() {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::None;
        return v;
    }
    static BoaValuePtr make_bool(bool b) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::Bool;
        v->bool_val = b;
        return v;
    }
    static BoaValuePtr make_int(int64_t i) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::Int;
        v->int_val = i;
        return v;
    }
    static BoaValuePtr make_float(double f) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::Float;
        v->float_val = f;
        return v;
    }
    static BoaValuePtr make_string(const std::string& s) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::String;
        v->string_val = s;
        return v;
    }
    static BoaValuePtr make_list(std::vector<BoaValuePtr> elems) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::List;
        v->list_val = std::move(elems);
        return v;
    }
    static BoaValuePtr make_dict(std::vector<std::pair<BoaValuePtr, BoaValuePtr>> entries) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::Dict;
        v->dict_val = std::move(entries);
        return v;
    }
    static BoaValuePtr make_function(const std::string& name,
                                     const std::vector<std::string>& params,
                                     std::vector<ASTNode*> body,
                                     EnvPtr closure) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::Function;
        v->func_val = {name, params, std::move(body), std::move(closure)};
        return v;
    }
    static BoaValuePtr make_builtin(BuiltinFn fn) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::BuiltinFunction;
        v->builtin_val = std::move(fn);
        return v;
    }
    static BoaValuePtr make_module(const std::string& name,
                                   std::unordered_map<std::string, BoaValuePtr> members) {
        auto v = std::make_shared<BoaValue>();
        v->type = ValueType::Module;
        v->module_val = {name, std::move(members)};
        return v;
    }

    bool is_truthy() const {
        switch (type) {
            case ValueType::None:  return false;
            case ValueType::Bool:  return bool_val;
            case ValueType::Int:   return int_val != 0;
            case ValueType::Float: return float_val != 0.0;
            case ValueType::String: return !string_val.empty();
            case ValueType::List:  return !list_val.empty();
            case ValueType::Dict:  return !dict_val.empty();
            default: return true;
        }
    }

    std::string to_string() const {
        switch (type) {
            case ValueType::None:  return "none";
            case ValueType::Bool:  return bool_val ? "true" : "false";
            case ValueType::Int:   return std::to_string(int_val);
            case ValueType::Float: {
                std::ostringstream oss;
                oss << float_val;
                return oss.str();
            }
            case ValueType::String: return string_val;
            case ValueType::List: {
                std::string s = "[";
                for (size_t i = 0; i < list_val.size(); ++i) {
                    if (i > 0) s += ", ";
                    if (list_val[i]->type == ValueType::String) {
                        s += "\"" + list_val[i]->to_string() + "\"";
                    } else {
                        s += list_val[i]->to_string();
                    }
                }
                s += "]";
                return s;
            }
            case ValueType::Dict: {
                std::string s = "{";
                for (size_t i = 0; i < dict_val.size(); ++i) {
                    if (i > 0) s += ", ";
                    s += dict_val[i].first->to_string() + ": " + dict_val[i].second->to_string();
                }
                s += "}";
                return s;
            }
            case ValueType::Function:
                return "<function " + func_val.name + ">";
            case ValueType::BuiltinFunction:
                return "<builtin_function>";
            case ValueType::Module:
                return "<module " + module_val.name + ">";
        }
        return "<unknown>";
    }

    double as_number() const {
        if (type == ValueType::Int) return static_cast<double>(int_val);
        if (type == ValueType::Float) return float_val;
        throw BoaRuntimeError("Expected numeric value, got " + value_type_name(type));
    }
};

// ============================================================================
// Environment (scope)
// ============================================================================
class Environment : public std::enable_shared_from_this<Environment> {
public:
    EnvPtr parent;
    std::unordered_map<std::string, BoaValuePtr> vars;

    explicit Environment(EnvPtr parent = nullptr) : parent(std::move(parent)) {}

    BoaValuePtr get(const std::string& name) const {
        auto it = vars.find(name);
        if (it != vars.end()) return it->second;
        if (parent) return parent->get(name);
        return nullptr;
    }

    void set(const std::string& name, BoaValuePtr val) {
        // Look up the scope chain; if found, update there
        Environment* env = this;
        while (env) {
            auto it = env->vars.find(name);
            if (it != env->vars.end()) {
                it->second = std::move(val);
                return;
            }
            env = env->parent.get();
        }
        // Not found -> define in current scope
        vars[name] = std::move(val);
    }

    void define(const std::string& name, BoaValuePtr val) {
        vars[name] = std::move(val);
    }
};

// ============================================================================
// Control flow exceptions (used internally)
// ============================================================================
struct ReturnException {
    BoaValuePtr value;
};

struct BreakException {};
struct ContinueException {};

struct BoaException {
    BoaValuePtr value;
};

// ============================================================================
// Interpreter
// ============================================================================
class Interpreter {
public:
    Interpreter() : global_env_(std::make_shared<Environment>()) {
        register_builtins();
    }

    // Execute a program AST
    BoaValuePtr exec(Program* program) {
        return exec_body(program->statements, global_env_);
    }

    // Execute source code string
    BoaValuePtr run(const std::string& source, const std::string& filename = "<stdin>") {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse_program();
        current_file_ = filename;
        return exec(program.get());
    }

    EnvPtr global_env() { return global_env_; }

    // Set the base directory for module resolution
    void set_base_dir(const std::string& dir) { base_dir_ = dir; }

    // Get output captured (for testing)
    std::string get_output() const { return output_.str(); }
    void clear_output() { output_.str(""); output_.clear(); }

    // Set output stream (for capturing)
    void set_capture_output(bool capture) { capture_output_ = capture; }

    // Store ASTs for module lifetime management
    std::vector<std::unique_ptr<Program>> module_asts_;

private:
    EnvPtr global_env_;
    std::string base_dir_ = ".";
    std::string current_file_ = "<stdin>";
    std::unordered_map<std::string, BoaValuePtr> module_cache_;
    std::ostringstream output_;
    bool capture_output_ = false;

    void print_output(const std::string& s) {
        if (capture_output_) {
            output_ << s;
        } else {
            std::cout << s;
        }
    }

    // -----------------------------------------------------------------------
    // Built-in functions & modules
    // -----------------------------------------------------------------------
    void register_builtins() {
        // io module
        std::unordered_map<std::string, BoaValuePtr> io_members;
        io_members["print"] = BoaValue::make_builtin(
            [this](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) print_output(" ");
                    print_output(args[i]->to_string());
                }
                print_output("\n");
                return BoaValue::make_none();
            });
        io_members["println"] = io_members["print"];
        io_members["input"] = BoaValue::make_builtin(
            [this](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (!args.empty()) {
                    print_output(args[0]->to_string());
                }
                std::string line;
                std::getline(std::cin, line);
                return BoaValue::make_string(line);
            });
        module_cache_["io"] = BoaValue::make_module("io", std::move(io_members));

        // fs module
        std::unordered_map<std::string, BoaValuePtr> fs_members;
        fs_members["read_all_bytes"] = BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.empty() || args[0]->type != ValueType::String)
                    throw BoaRuntimeError("fs.read_all_bytes: expected string argument");
                std::ifstream file(args[0]->string_val, std::ios::binary);
                if (!file)
                    throw BoaRuntimeError("fs.read_all_bytes: cannot open file '" + args[0]->string_val + "'");
                std::string content((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
                return BoaValue::make_string(content);
            });
        fs_members["write_all_bytes"] = BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() < 2 || args[0]->type != ValueType::String)
                    throw BoaRuntimeError("fs.write_all_bytes: expected (filename, data)");
                std::ofstream file(args[0]->string_val, std::ios::binary);
                if (!file)
                    throw BoaRuntimeError("fs.write_all_bytes: cannot open file '" + args[0]->string_val + "'");
                file << args[1]->to_string();
                return BoaValue::make_none();
            });
        fs_members["read_text"] = BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.empty() || args[0]->type != ValueType::String)
                    throw BoaRuntimeError("fs.read_text: expected string argument");
                std::ifstream file(args[0]->string_val);
                if (!file)
                    throw BoaRuntimeError("fs.read_text: cannot open file '" + args[0]->string_val + "'");
                std::string content((std::istreambuf_iterator<char>(file)),
                                     std::istreambuf_iterator<char>());
                return BoaValue::make_string(content);
            });
        fs_members["write_text"] = BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() < 2)
                    throw BoaRuntimeError("fs.write_text: expected (filename, text)");
                std::ofstream file(args[0]->string_val);
                if (!file)
                    throw BoaRuntimeError("fs.write_text: cannot open file '" + args[0]->string_val + "'");
                file << args[1]->to_string();
                return BoaValue::make_none();
            });
        module_cache_["fs"] = BoaValue::make_module("fs", std::move(fs_members));

        // Global built-in functions
        global_env_->define("len", BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() != 1)
                    throw BoaRuntimeError("len: expected 1 argument");
                auto& a = args[0];
                switch (a->type) {
                    case ValueType::String:
                        return BoaValue::make_int(static_cast<int64_t>(a->string_val.size()));
                    case ValueType::List:
                        return BoaValue::make_int(static_cast<int64_t>(a->list_val.size()));
                    case ValueType::Dict:
                        return BoaValue::make_int(static_cast<int64_t>(a->dict_val.size()));
                    default:
                        throw BoaRuntimeError("len: unsupported type " + value_type_name(a->type));
                }
            }));

        global_env_->define("str", BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() != 1)
                    throw BoaRuntimeError("str: expected 1 argument");
                return BoaValue::make_string(args[0]->to_string());
            }));

        global_env_->define("int", BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() != 1)
                    throw BoaRuntimeError("int: expected 1 argument");
                auto& a = args[0];
                switch (a->type) {
                    case ValueType::Int: return a;
                    case ValueType::Float: return BoaValue::make_int(static_cast<int64_t>(a->float_val));
                    case ValueType::String: {
                        try {
                            return BoaValue::make_int(std::stoll(a->string_val));
                        } catch (...) {
                            throw BoaRuntimeError("int: cannot convert '" + a->string_val + "' to int");
                        }
                    }
                    case ValueType::Bool: return BoaValue::make_int(a->bool_val ? 1 : 0);
                    default:
                        throw BoaRuntimeError("int: unsupported type " + value_type_name(a->type));
                }
            }));

        global_env_->define("float", BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() != 1)
                    throw BoaRuntimeError("float: expected 1 argument");
                auto& a = args[0];
                switch (a->type) {
                    case ValueType::Float: return a;
                    case ValueType::Int: return BoaValue::make_float(static_cast<double>(a->int_val));
                    case ValueType::String: {
                        try {
                            return BoaValue::make_float(std::stod(a->string_val));
                        } catch (...) {
                            throw BoaRuntimeError("float: cannot convert '" + a->string_val + "' to float");
                        }
                    }
                    case ValueType::Bool: return BoaValue::make_float(a->bool_val ? 1.0 : 0.0);
                    default:
                        throw BoaRuntimeError("float: unsupported type " + value_type_name(a->type));
                }
            }));

        global_env_->define("type", BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() != 1)
                    throw BoaRuntimeError("type: expected 1 argument");
                return BoaValue::make_string(value_type_name(args[0]->type));
            }));

        global_env_->define("range", BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                int64_t start = 0, stop = 0, step = 1;
                if (args.size() == 1) {
                    stop = static_cast<int64_t>(args[0]->as_number());
                } else if (args.size() == 2) {
                    start = static_cast<int64_t>(args[0]->as_number());
                    stop  = static_cast<int64_t>(args[1]->as_number());
                } else if (args.size() == 3) {
                    start = static_cast<int64_t>(args[0]->as_number());
                    stop  = static_cast<int64_t>(args[1]->as_number());
                    step  = static_cast<int64_t>(args[2]->as_number());
                } else {
                    throw BoaRuntimeError("range: expected 1-3 arguments");
                }
                if (step == 0) throw BoaRuntimeError("range: step cannot be zero");
                std::vector<BoaValuePtr> result;
                if (step > 0) {
                    for (int64_t i = start; i < stop; i += step)
                        result.push_back(BoaValue::make_int(i));
                } else {
                    for (int64_t i = start; i > stop; i += step)
                        result.push_back(BoaValue::make_int(i));
                }
                return BoaValue::make_list(std::move(result));
            }));

        global_env_->define("append", BoaValue::make_builtin(
            [](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                if (args.size() != 2)
                    throw BoaRuntimeError("append: expected 2 arguments (list, value)");
                if (args[0]->type != ValueType::List)
                    throw BoaRuntimeError("append: first argument must be a list");
                args[0]->list_val.push_back(args[1]);
                return BoaValue::make_none();
            }));

        global_env_->define("print", BoaValue::make_builtin(
            [this](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) print_output(" ");
                    print_output(args[i]->to_string());
                }
                print_output("\n");
                return BoaValue::make_none();
            }));
    }

    // -----------------------------------------------------------------------
    // AST evaluation
    // -----------------------------------------------------------------------
    BoaValuePtr exec_body(const std::vector<ASTNodePtr>& stmts, EnvPtr env) {
        BoaValuePtr result = BoaValue::make_none();
        for (auto& stmt : stmts) {
            result = eval(stmt.get(), env);
        }
        return result;
    }

    BoaValuePtr exec_body_raw(const std::vector<ASTNode*>& stmts, EnvPtr env) {
        BoaValuePtr result = BoaValue::make_none();
        for (auto* stmt : stmts) {
            result = eval(stmt, env);
        }
        return result;
    }

    BoaValuePtr eval(ASTNode* node, EnvPtr env) {
        if (!node) return BoaValue::make_none();

        if (auto* n = dynamic_cast<NumberLiteral*>(node))
            return eval_number(n);
        if (auto* n = dynamic_cast<StringLiteral*>(node))
            return BoaValue::make_string(n->value);
        if (auto* n = dynamic_cast<BoolLiteral*>(node))
            return BoaValue::make_bool(n->value);
        if (dynamic_cast<NoneLiteral*>(node))
            return BoaValue::make_none();
        if (auto* n = dynamic_cast<Identifier*>(node))
            return eval_identifier(n, env);
        if (auto* n = dynamic_cast<BinaryOp*>(node))
            return eval_binary(n, env);
        if (auto* n = dynamic_cast<UnaryOp*>(node))
            return eval_unary(n, env);
        if (auto* n = dynamic_cast<Assignment*>(node))
            return eval_assignment(n, env);
        if (auto* n = dynamic_cast<ListLiteral*>(node))
            return eval_list(n, env);
        if (auto* n = dynamic_cast<DictLiteral*>(node))
            return eval_dict(n, env);
        if (auto* n = dynamic_cast<IndexExpr*>(node))
            return eval_index(n, env);
        if (auto* n = dynamic_cast<MemberAccess*>(node))
            return eval_member(n, env);
        if (auto* n = dynamic_cast<FunctionCall*>(node))
            return eval_call(n, env);
        if (auto* n = dynamic_cast<ExpressionStmt*>(node))
            return eval(n->expr.get(), env);
        if (auto* n = dynamic_cast<FnDef*>(node))
            return eval_fndef(n, env);
        if (auto* n = dynamic_cast<ReturnStmt*>(node))
            return eval_return(n, env);
        if (auto* n = dynamic_cast<IfStmt*>(node))
            return eval_if(n, env);
        if (auto* n = dynamic_cast<ForStmt*>(node))
            return eval_for(n, env);
        if (auto* n = dynamic_cast<WhileStmt*>(node))
            return eval_while(n, env);
        if (auto* n = dynamic_cast<ImportStmt*>(node))
            return eval_import(n, env);
        if (auto* n = dynamic_cast<TryStmt*>(node))
            return eval_try(n, env);
        if (auto* n = dynamic_cast<Block*>(node))
            return exec_body(n->statements, env);
        if (dynamic_cast<PassStmt*>(node))
            return BoaValue::make_none();

        throw BoaRuntimeError("Unknown AST node", node->loc.line, node->loc.column);
    }

    BoaValuePtr eval_number(NumberLiteral* n) {
        double v = n->value;
        if (v == static_cast<int64_t>(v) && v >= -9e18 && v <= 9e18) {
            return BoaValue::make_int(static_cast<int64_t>(v));
        }
        return BoaValue::make_float(v);
    }

    BoaValuePtr eval_identifier(Identifier* n, EnvPtr env) {
        auto val = env->get(n->name);
        if (!val)
            throw BoaRuntimeError("Undefined variable '" + n->name + "'",
                                  n->loc.line, n->loc.column);
        return val;
    }

    BoaValuePtr eval_binary(BinaryOp* n, EnvPtr env) {
        auto left = eval(n->left.get(), env);
        auto right = eval(n->right.get(), env);

        switch (n->op) {
            case TokenType::Plus:     return add(left, right, n);
            case TokenType::Minus:    return subtract(left, right, n);
            case TokenType::Star:     return multiply(left, right, n);
            case TokenType::Slash:    return divide(left, right, n);
            case TokenType::Percent:  return modulo(left, right, n);
            case TokenType::DoubleStar: return power(left, right, n);
            case TokenType::EqEq:    return BoaValue::make_bool(values_equal(left, right));
            case TokenType::BangEq:  return BoaValue::make_bool(!values_equal(left, right));
            case TokenType::Less:    return BoaValue::make_bool(compare(left, right) < 0);
            case TokenType::LessEq:  return BoaValue::make_bool(compare(left, right) <= 0);
            case TokenType::Greater: return BoaValue::make_bool(compare(left, right) > 0);
            case TokenType::GreaterEq: return BoaValue::make_bool(compare(left, right) >= 0);
            case TokenType::And:     return left->is_truthy() ? right : left;
            case TokenType::Or:      return left->is_truthy() ? left : right;
            default:
                throw BoaRuntimeError("Unknown binary operator", n->loc.line, n->loc.column);
        }
    }

    BoaValuePtr eval_unary(UnaryOp* n, EnvPtr env) {
        auto val = eval(n->operand.get(), env);
        switch (n->op) {
            case TokenType::Minus:
                if (val->type == ValueType::Int) return BoaValue::make_int(-val->int_val);
                if (val->type == ValueType::Float) return BoaValue::make_float(-val->float_val);
                throw BoaRuntimeError("Cannot negate " + value_type_name(val->type),
                                      n->loc.line, n->loc.column);
            case TokenType::Plus:
                if (val->type == ValueType::Int || val->type == ValueType::Float) return val;
                throw BoaRuntimeError("Cannot apply unary + to " + value_type_name(val->type),
                                      n->loc.line, n->loc.column);
            case TokenType::Not:
                return BoaValue::make_bool(!val->is_truthy());
            default:
                throw BoaRuntimeError("Unknown unary operator", n->loc.line, n->loc.column);
        }
    }

    BoaValuePtr eval_assignment(Assignment* n, EnvPtr env) {
        auto val = eval(n->value.get(), env);

        // Simple variable assignment
        if (auto* id = dynamic_cast<Identifier*>(n->target.get())) {
            if (n->op == TokenType::Eq) {
                env->set(id->name, val);
            } else {
                auto existing = env->get(id->name);
                if (!existing)
                    throw BoaRuntimeError("Undefined variable '" + id->name + "'",
                                          n->loc.line, n->loc.column);
                BoaValuePtr result;
                switch (n->op) {
                    case TokenType::PlusEq:  result = add(existing, val, n); break;
                    case TokenType::MinusEq: result = subtract(existing, val, n); break;
                    case TokenType::StarEq:  result = multiply(existing, val, n); break;
                    case TokenType::SlashEq: result = divide(existing, val, n); break;
                    default: throw BoaRuntimeError("Unknown assignment operator");
                }
                env->set(id->name, result);
            }
            return val;
        }

        // Index assignment: a[i] = v
        if (auto* idx = dynamic_cast<IndexExpr*>(n->target.get())) {
            auto obj = eval(idx->object.get(), env);
            auto index = eval(idx->index.get(), env);
            if (obj->type == ValueType::List) {
                int64_t i = static_cast<int64_t>(index->as_number());
                if (i < 0) i += static_cast<int64_t>(obj->list_val.size());
                if (i < 0 || i >= static_cast<int64_t>(obj->list_val.size()))
                    throw BoaRuntimeError("Index out of range", n->loc.line, n->loc.column);
                obj->list_val[static_cast<size_t>(i)] = val;
            } else if (obj->type == ValueType::Dict) {
                for (auto& p : obj->dict_val) {
                    if (values_equal(p.first, index)) {
                        p.second = val;
                        return val;
                    }
                }
                obj->dict_val.push_back({index, val});
            } else {
                throw BoaRuntimeError("Cannot index " + value_type_name(obj->type));
            }
            return val;
        }

        // Member assignment: obj.member = v
        if (auto* mem = dynamic_cast<MemberAccess*>(n->target.get())) {
            auto obj = eval(mem->object.get(), env);
            if (obj->type == ValueType::Module) {
                obj->module_val.members[mem->member] = val;
            } else {
                throw BoaRuntimeError("Cannot set member on " + value_type_name(obj->type));
            }
            return val;
        }

        throw BoaRuntimeError("Invalid assignment target", n->loc.line, n->loc.column);
    }

    BoaValuePtr eval_list(ListLiteral* n, EnvPtr env) {
        std::vector<BoaValuePtr> elems;
        elems.reserve(n->elements.size());
        for (auto& e : n->elements) {
            elems.push_back(eval(e.get(), env));
        }
        return BoaValue::make_list(std::move(elems));
    }

    BoaValuePtr eval_dict(DictLiteral* n, EnvPtr env) {
        std::vector<std::pair<BoaValuePtr, BoaValuePtr>> entries;
        entries.reserve(n->entries.size());
        for (auto& p : n->entries) {
            entries.push_back({eval(p.first.get(), env), eval(p.second.get(), env)});
        }
        return BoaValue::make_dict(std::move(entries));
    }

    BoaValuePtr eval_index(IndexExpr* n, EnvPtr env) {
        auto obj = eval(n->object.get(), env);
        auto index = eval(n->index.get(), env);

        if (obj->type == ValueType::List) {
            int64_t i = static_cast<int64_t>(index->as_number());
            if (i < 0) i += static_cast<int64_t>(obj->list_val.size());
            if (i < 0 || i >= static_cast<int64_t>(obj->list_val.size()))
                throw BoaRuntimeError("Index out of range", n->loc.line, n->loc.column);
            return obj->list_val[static_cast<size_t>(i)];
        }
        if (obj->type == ValueType::String) {
            int64_t i = static_cast<int64_t>(index->as_number());
            if (i < 0) i += static_cast<int64_t>(obj->string_val.size());
            if (i < 0 || i >= static_cast<int64_t>(obj->string_val.size()))
                throw BoaRuntimeError("String index out of range", n->loc.line, n->loc.column);
            return BoaValue::make_string(std::string(1, obj->string_val[static_cast<size_t>(i)]));
        }
        if (obj->type == ValueType::Dict) {
            for (auto& p : obj->dict_val) {
                if (values_equal(p.first, index)) return p.second;
            }
            throw BoaRuntimeError("Key not found in dict", n->loc.line, n->loc.column);
        }

        throw BoaRuntimeError("Cannot index " + value_type_name(obj->type),
                              n->loc.line, n->loc.column);
    }

    BoaValuePtr eval_member(MemberAccess* n, EnvPtr env) {
        auto obj = eval(n->object.get(), env);

        if (obj->type == ValueType::Module) {
            auto it = obj->module_val.members.find(n->member);
            if (it != obj->module_val.members.end()) return it->second;
            throw BoaRuntimeError("Module '" + obj->module_val.name + "' has no member '" + n->member + "'",
                                  n->loc.line, n->loc.column);
        }

        // List methods
        if (obj->type == ValueType::List) {
            if (n->member == "append") {
                auto list_ref = obj;
                return BoaValue::make_builtin(
                    [list_ref](std::vector<BoaValuePtr> args) -> BoaValuePtr {
                        if (args.size() != 1)
                            throw BoaRuntimeError("append: expected 1 argument");
                        list_ref->list_val.push_back(args[0]);
                        return BoaValue::make_none();
                    });
            }
            if (n->member == "length") {
                return BoaValue::make_int(static_cast<int64_t>(obj->list_val.size()));
            }
        }

        // String methods
        if (obj->type == ValueType::String) {
            if (n->member == "length") {
                return BoaValue::make_int(static_cast<int64_t>(obj->string_val.size()));
            }
            if (n->member == "upper") {
                auto s = obj->string_val;
                return BoaValue::make_builtin(
                    [s](std::vector<BoaValuePtr>) -> BoaValuePtr {
                        std::string result = s;
                        for (auto& c : result) c = static_cast<char>(toupper(c));
                        return BoaValue::make_string(result);
                    });
            }
            if (n->member == "lower") {
                auto s = obj->string_val;
                return BoaValue::make_builtin(
                    [s](std::vector<BoaValuePtr>) -> BoaValuePtr {
                        std::string result = s;
                        for (auto& c : result) c = static_cast<char>(tolower(c));
                        return BoaValue::make_string(result);
                    });
            }
        }

        throw BoaRuntimeError("Cannot access member '" + n->member + "' on " + value_type_name(obj->type),
                              n->loc.line, n->loc.column);
    }

    BoaValuePtr eval_call(FunctionCall* n, EnvPtr env) {
        auto callee = eval(n->callee.get(), env);
        std::vector<BoaValuePtr> args;
        args.reserve(n->args.size());
        for (auto& a : n->args) {
            args.push_back(eval(a.get(), env));
        }

        if (callee->type == ValueType::BuiltinFunction) {
            return callee->builtin_val(args);
        }

        if (callee->type == ValueType::Function) {
            auto& fn = callee->func_val;
            if (args.size() != fn.params.size())
                throw BoaRuntimeError("Function '" + fn.name + "' expected " +
                                      std::to_string(fn.params.size()) + " arguments, got " +
                                      std::to_string(args.size()),
                                      n->loc.line, n->loc.column);

            auto fn_env = std::make_shared<Environment>(fn.closure);
            for (size_t i = 0; i < fn.params.size(); ++i) {
                fn_env->define(fn.params[i], args[i]);
            }

            try {
                BoaValuePtr result = BoaValue::make_none();
                for (size_t i = 0; i < fn.body.size(); ++i) {
                    result = eval(fn.body[i], fn_env);
                }
                // Implicit return: return last expression value
                return result;
            } catch (ReturnException& ret) {
                return ret.value;
            }
        }

        throw BoaRuntimeError("Object is not callable", n->loc.line, n->loc.column);
    }

    BoaValuePtr eval_fndef(FnDef* n, EnvPtr env) {
        std::vector<ASTNode*> body_ptrs;
        body_ptrs.reserve(n->body.size());
        for (auto& stmt : n->body) {
            body_ptrs.push_back(stmt.get());
        }
        auto fn = BoaValue::make_function(n->name, n->params, std::move(body_ptrs), env);
        env->define(n->name, fn);
        return fn;
    }

    BoaValuePtr eval_return(ReturnStmt* n, EnvPtr env) {
        BoaValuePtr val = BoaValue::make_none();
        if (n->value) {
            val = eval(n->value.get(), env);
        }
        throw ReturnException{val};
    }

    BoaValuePtr eval_if(IfStmt* n, EnvPtr env) {
        if (eval(n->condition.get(), env)->is_truthy()) {
            return exec_body(n->body, env);
        }
        for (auto& elif : n->elif_clauses) {
            if (eval(elif.condition.get(), env)->is_truthy()) {
                return exec_body(elif.body, env);
            }
        }
        if (!n->else_body.empty()) {
            return exec_body(n->else_body, env);
        }
        return BoaValue::make_none();
    }

    BoaValuePtr eval_for(ForStmt* n, EnvPtr env) {
        auto iterable = eval(n->iterable.get(), env);
        if (iterable->type != ValueType::List)
            throw BoaRuntimeError("for: can only iterate over lists", n->loc.line, n->loc.column);

        BoaValuePtr result = BoaValue::make_none();
        for (auto& item : iterable->list_val) {
            env->set(n->var_name, item);
            try {
                result = exec_body(n->body, env);
            } catch (BreakException&) {
                break;
            } catch (ContinueException&) {
                continue;
            }
        }
        return result;
    }

    BoaValuePtr eval_while(WhileStmt* n, EnvPtr env) {
        BoaValuePtr result = BoaValue::make_none();
        while (eval(n->condition.get(), env)->is_truthy()) {
            try {
                result = exec_body(n->body, env);
            } catch (BreakException&) {
                break;
            } catch (ContinueException&) {
                continue;
            }
        }
        return result;
    }

    BoaValuePtr eval_import(ImportStmt* n, EnvPtr env) {
        for (auto& mod_name : n->modules) {
            auto it = module_cache_.find(mod_name);
            if (it != module_cache_.end()) {
                env->define(mod_name, it->second);
                continue;
            }
            // Try to load from file
            std::string path = base_dir_ + "/" + mod_name + ".boa";
            std::ifstream file(path);
            if (!file) {
                throw BoaRuntimeError("Cannot find module '" + mod_name + "' (looked in " + path + ")",
                                      n->loc.line, n->loc.column);
            }
            std::string source((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());

            // Parse and execute the module
            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            auto program = parser.parse_program();

            auto mod_env = std::make_shared<Environment>(global_env_);
            exec(program.get());

            // Collect module exports
            std::unordered_map<std::string, BoaValuePtr> members;
            for (auto& p : mod_env->vars) {
                members[p.first] = p.second;
            }
            auto module = BoaValue::make_module(mod_name, std::move(members));
            module_cache_[mod_name] = module;
            // Store the AST to keep it alive
            module_asts_.push_back(std::move(program));
            env->define(mod_name, module);
        }
        return BoaValue::make_none();
    }

    BoaValuePtr eval_try(TryStmt* n, EnvPtr env) {
        try {
            return exec_body(n->try_body, env);
        } catch (BoaRuntimeError& e) {
            if (!n->except_body.empty()) {
                if (!n->except_var.empty()) {
                    env->set(n->except_var, BoaValue::make_string(e.what()));
                }
                auto result = exec_body(n->except_body, env);
                if (!n->finally_body.empty()) {
                    exec_body(n->finally_body, env);
                }
                return result;
            }
            if (!n->finally_body.empty()) {
                exec_body(n->finally_body, env);
            }
            throw;
        } catch (BoaException& e) {
            if (!n->except_body.empty()) {
                if (!n->except_var.empty()) {
                    env->set(n->except_var, e.value);
                }
                auto result = exec_body(n->except_body, env);
                if (!n->finally_body.empty()) {
                    exec_body(n->finally_body, env);
                }
                return result;
            }
            if (!n->finally_body.empty()) {
                exec_body(n->finally_body, env);
            }
            throw;
        }
    }

    // -----------------------------------------------------------------------
    // Arithmetic helpers
    // -----------------------------------------------------------------------
    BoaValuePtr add(BoaValuePtr left, BoaValuePtr right, ASTNode* n) {
        if (left->type == ValueType::String && right->type == ValueType::String)
            return BoaValue::make_string(left->string_val + right->string_val);
        if (left->type == ValueType::List && right->type == ValueType::List) {
            auto result = left->list_val;
            result.insert(result.end(), right->list_val.begin(), right->list_val.end());
            return BoaValue::make_list(std::move(result));
        }
        if (left->type == ValueType::Int && right->type == ValueType::Int)
            return BoaValue::make_int(left->int_val + right->int_val);
        if ((left->type == ValueType::Int || left->type == ValueType::Float) &&
            (right->type == ValueType::Int || right->type == ValueType::Float))
            return BoaValue::make_float(left->as_number() + right->as_number());
        throw BoaRuntimeError("Cannot add " + value_type_name(left->type) + " and " + value_type_name(right->type),
                              n->loc.line, n->loc.column);
    }

    BoaValuePtr subtract(BoaValuePtr left, BoaValuePtr right, ASTNode* n) {
        if (left->type == ValueType::Int && right->type == ValueType::Int)
            return BoaValue::make_int(left->int_val - right->int_val);
        if ((left->type == ValueType::Int || left->type == ValueType::Float) &&
            (right->type == ValueType::Int || right->type == ValueType::Float))
            return BoaValue::make_float(left->as_number() - right->as_number());
        throw BoaRuntimeError("Cannot subtract " + value_type_name(left->type) + " and " + value_type_name(right->type),
                              n->loc.line, n->loc.column);
    }

    BoaValuePtr multiply(BoaValuePtr left, BoaValuePtr right, ASTNode* n) {
        if (left->type == ValueType::Int && right->type == ValueType::Int)
            return BoaValue::make_int(left->int_val * right->int_val);
        if ((left->type == ValueType::Int || left->type == ValueType::Float) &&
            (right->type == ValueType::Int || right->type == ValueType::Float))
            return BoaValue::make_float(left->as_number() * right->as_number());
        // String repetition
        if (left->type == ValueType::String && right->type == ValueType::Int) {
            std::string result;
            for (int64_t i = 0; i < right->int_val; ++i) result += left->string_val;
            return BoaValue::make_string(result);
        }
        throw BoaRuntimeError("Cannot multiply " + value_type_name(left->type) + " and " + value_type_name(right->type),
                              n->loc.line, n->loc.column);
    }

    BoaValuePtr divide(BoaValuePtr left, BoaValuePtr right, ASTNode* n) {
        double r = right->as_number();
        if (r == 0.0)
            throw BoaRuntimeError("Division by zero", n->loc.line, n->loc.column);
        if (left->type == ValueType::Int && right->type == ValueType::Int)
            return BoaValue::make_int(left->int_val / right->int_val);
        return BoaValue::make_float(left->as_number() / r);
    }

    BoaValuePtr modulo(BoaValuePtr left, BoaValuePtr right, ASTNode* n) {
        if (left->type == ValueType::Int && right->type == ValueType::Int) {
            if (right->int_val == 0)
                throw BoaRuntimeError("Modulo by zero", n->loc.line, n->loc.column);
            return BoaValue::make_int(left->int_val % right->int_val);
        }
        double r = right->as_number();
        if (r == 0.0)
            throw BoaRuntimeError("Modulo by zero", n->loc.line, n->loc.column);
        return BoaValue::make_float(std::fmod(left->as_number(), r));
    }

    BoaValuePtr power(BoaValuePtr left, BoaValuePtr right, ASTNode* n) {
        if (left->type == ValueType::Int && right->type == ValueType::Int && right->int_val >= 0) {
            int64_t result = 1;
            int64_t base = left->int_val;
            int64_t exp = right->int_val;
            while (exp > 0) {
                if (exp % 2 == 1) result *= base;
                base *= base;
                exp /= 2;
            }
            return BoaValue::make_int(result);
        }
        if ((left->type == ValueType::Int || left->type == ValueType::Float) &&
            (right->type == ValueType::Int || right->type == ValueType::Float))
            return BoaValue::make_float(std::pow(left->as_number(), right->as_number()));
        throw BoaRuntimeError("Cannot exponentiate " + value_type_name(left->type),
                              n->loc.line, n->loc.column);
    }

    bool values_equal(BoaValuePtr a, BoaValuePtr b) {
        if (a->type == ValueType::None && b->type == ValueType::None) return true;
        if (a->type == ValueType::Bool && b->type == ValueType::Bool) return a->bool_val == b->bool_val;
        if (a->type == ValueType::Int && b->type == ValueType::Int) return a->int_val == b->int_val;
        if (a->type == ValueType::Float && b->type == ValueType::Float) return a->float_val == b->float_val;
        if (a->type == ValueType::Int && b->type == ValueType::Float) return static_cast<double>(a->int_val) == b->float_val;
        if (a->type == ValueType::Float && b->type == ValueType::Int) return a->float_val == static_cast<double>(b->int_val);
        if (a->type == ValueType::String && b->type == ValueType::String) return a->string_val == b->string_val;
        return false;
    }

    int compare(BoaValuePtr a, BoaValuePtr b) {
        if ((a->type == ValueType::Int || a->type == ValueType::Float) &&
            (b->type == ValueType::Int || b->type == ValueType::Float)) {
            double av = a->as_number(), bv = b->as_number();
            if (av < bv) return -1;
            if (av > bv) return 1;
            return 0;
        }
        if (a->type == ValueType::String && b->type == ValueType::String) {
            return a->string_val.compare(b->string_val);
        }
        throw BoaRuntimeError("Cannot compare " + value_type_name(a->type) + " and " + value_type_name(b->type));
    }
};

// ============================================================================
// Convenience: run source code and return output string (for testing)
// ============================================================================
inline std::string run_and_capture(const std::string& source, const std::string& filename = "<test>") {
    Interpreter interp;
    interp.set_capture_output(true);
    interp.run(source, filename);
    return interp.get_output();
}

} // namespace boa
