#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace boa {

// Forward declarations
struct ASTNode;
using ASTNodePtr = std::unique_ptr<ASTNode>;

// Source location for error reporting
struct SourceLoc {
    int line   = 0;
    int column = 0;
};

// ---------------------------------------------------------------------------
// Base
// ---------------------------------------------------------------------------
struct ASTNode {
    SourceLoc loc;

    explicit ASTNode(SourceLoc loc) : loc(loc) {}
    virtual ~ASTNode() = default;

    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    ASTNode(ASTNode&&) = default;
    ASTNode& operator=(ASTNode&&) = default;
};

// ---------------------------------------------------------------------------
// Literals
// ---------------------------------------------------------------------------
struct NumberLiteral : ASTNode {
    double value;

    NumberLiteral(double value, SourceLoc loc)
        : ASTNode(loc), value(value) {}
};

struct StringLiteral : ASTNode {
    std::string value;

    StringLiteral(std::string value, SourceLoc loc)
        : ASTNode(loc), value(std::move(value)) {}
};

struct BoolLiteral : ASTNode {
    bool value;

    BoolLiteral(bool value, SourceLoc loc)
        : ASTNode(loc), value(value) {}
};

struct NoneLiteral : ASTNode {
    explicit NoneLiteral(SourceLoc loc) : ASTNode(loc) {}
};

// ---------------------------------------------------------------------------
// Identifier
// ---------------------------------------------------------------------------
struct Identifier : ASTNode {
    std::string name;

    Identifier(std::string name, SourceLoc loc)
        : ASTNode(loc), name(std::move(name)) {}
};

// ---------------------------------------------------------------------------
// Operators
// ---------------------------------------------------------------------------
struct BinaryOp : ASTNode {
    ASTNodePtr  left;
    TokenType   op;
    ASTNodePtr  right;

    BinaryOp(ASTNodePtr left, TokenType op, ASTNodePtr right, SourceLoc loc)
        : ASTNode(loc)
        , left(std::move(left))
        , op(op)
        , right(std::move(right)) {}
};

struct UnaryOp : ASTNode {
    TokenType  op;
    ASTNodePtr operand;

    UnaryOp(TokenType op, ASTNodePtr operand, SourceLoc loc)
        : ASTNode(loc), op(op), operand(std::move(operand)) {}
};

// ---------------------------------------------------------------------------
// Assignment   target (= | += | -= | *= | /=) value
// ---------------------------------------------------------------------------
struct Assignment : ASTNode {
    ASTNodePtr  target;
    TokenType   op;       // Eq, PlusEq, MinusEq, StarEq, SlashEq
    ASTNodePtr  value;

    Assignment(ASTNodePtr target, TokenType op, ASTNodePtr value, SourceLoc loc)
        : ASTNode(loc)
        , target(std::move(target))
        , op(op)
        , value(std::move(value)) {}
};

// ---------------------------------------------------------------------------
// Collection literals
// ---------------------------------------------------------------------------
struct ListLiteral : ASTNode {
    std::vector<ASTNodePtr> elements;

    ListLiteral(std::vector<ASTNodePtr> elements, SourceLoc loc)
        : ASTNode(loc), elements(std::move(elements)) {}
};

struct DictLiteral : ASTNode {
    std::vector<std::pair<ASTNodePtr, ASTNodePtr>> entries;

    DictLiteral(std::vector<std::pair<ASTNodePtr, ASTNodePtr>> entries,
                SourceLoc loc)
        : ASTNode(loc), entries(std::move(entries)) {}
};

// ---------------------------------------------------------------------------
// Index / member / call
// ---------------------------------------------------------------------------
struct IndexExpr : ASTNode {
    ASTNodePtr object;
    ASTNodePtr index;

    IndexExpr(ASTNodePtr object, ASTNodePtr index, SourceLoc loc)
        : ASTNode(loc)
        , object(std::move(object))
        , index(std::move(index)) {}
};

struct MemberAccess : ASTNode {
    ASTNodePtr  object;
    std::string member;

    MemberAccess(ASTNodePtr object, std::string member, SourceLoc loc)
        : ASTNode(loc)
        , object(std::move(object))
        , member(std::move(member)) {}
};

struct FunctionCall : ASTNode {
    ASTNodePtr              callee;
    std::vector<ASTNodePtr> args;

    FunctionCall(ASTNodePtr callee, std::vector<ASTNodePtr> args,
                 SourceLoc loc)
        : ASTNode(loc)
        , callee(std::move(callee))
        , args(std::move(args)) {}
};

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------
struct ExpressionStmt : ASTNode {
    ASTNodePtr expr;

    ExpressionStmt(ASTNodePtr expr, SourceLoc loc)
        : ASTNode(loc), expr(std::move(expr)) {}
};

struct Block : ASTNode {
    std::vector<ASTNodePtr> statements;

    Block(std::vector<ASTNodePtr> stmts, SourceLoc loc)
        : ASTNode(loc), statements(std::move(stmts)) {}
};

struct PassStmt : ASTNode {
    explicit PassStmt(SourceLoc loc) : ASTNode(loc) {}
};

struct ReturnStmt : ASTNode {
    ASTNodePtr value;   // may be nullptr

    ReturnStmt(ASTNodePtr value, SourceLoc loc)
        : ASTNode(loc), value(std::move(value)) {}
};

struct FnDef : ASTNode {
    std::string             name;
    std::vector<std::string> params;
    std::vector<ASTNodePtr> body;

    FnDef(std::string name, std::vector<std::string> params,
          std::vector<ASTNodePtr> body, SourceLoc loc)
        : ASTNode(loc)
        , name(std::move(name))
        , params(std::move(params))
        , body(std::move(body)) {}
};

// ---------------------------------------------------------------------------
// Control flow
// ---------------------------------------------------------------------------
struct ElifClause {
    ASTNodePtr              condition;
    std::vector<ASTNodePtr> body;
};

struct IfStmt : ASTNode {
    ASTNodePtr              condition;
    std::vector<ASTNodePtr> body;
    std::vector<ElifClause> elif_clauses;
    std::vector<ASTNodePtr> else_body;

    IfStmt(ASTNodePtr condition,
           std::vector<ASTNodePtr> body,
           std::vector<ElifClause> elif_clauses,
           std::vector<ASTNodePtr> else_body,
           SourceLoc loc)
        : ASTNode(loc)
        , condition(std::move(condition))
        , body(std::move(body))
        , elif_clauses(std::move(elif_clauses))
        , else_body(std::move(else_body)) {}
};

struct ForStmt : ASTNode {
    std::string             var_name;
    ASTNodePtr              iterable;
    std::vector<ASTNodePtr> body;

    ForStmt(std::string var_name, ASTNodePtr iterable,
            std::vector<ASTNodePtr> body, SourceLoc loc)
        : ASTNode(loc)
        , var_name(std::move(var_name))
        , iterable(std::move(iterable))
        , body(std::move(body)) {}
};

struct WhileStmt : ASTNode {
    ASTNodePtr              condition;
    std::vector<ASTNodePtr> body;

    WhileStmt(ASTNodePtr condition, std::vector<ASTNodePtr> body,
              SourceLoc loc)
        : ASTNode(loc)
        , condition(std::move(condition))
        , body(std::move(body)) {}
};

// ---------------------------------------------------------------------------
// Import
// ---------------------------------------------------------------------------
struct ImportStmt : ASTNode {
    std::vector<std::string> modules;

    ImportStmt(std::vector<std::string> modules, SourceLoc loc)
        : ASTNode(loc), modules(std::move(modules)) {}
};

// ---------------------------------------------------------------------------
// Try / except / finally
// ---------------------------------------------------------------------------
struct TryStmt : ASTNode {
    std::vector<ASTNodePtr> try_body;
    std::string             except_var;     // may be empty
    std::vector<ASTNodePtr> except_body;
    std::vector<ASTNodePtr> finally_body;

    TryStmt(std::vector<ASTNodePtr> try_body,
            std::string except_var,
            std::vector<ASTNodePtr> except_body,
            std::vector<ASTNodePtr> finally_body,
            SourceLoc loc)
        : ASTNode(loc)
        , try_body(std::move(try_body))
        , except_var(std::move(except_var))
        , except_body(std::move(except_body))
        , finally_body(std::move(finally_body)) {}
};

// ---------------------------------------------------------------------------
// Class
// ---------------------------------------------------------------------------
struct ClassDef : ASTNode {
    std::string             name;
    std::vector<ASTNodePtr> methods;

    ClassDef(std::string name, std::vector<ASTNodePtr> methods,
             SourceLoc loc)
        : ASTNode(loc)
        , name(std::move(name))
        , methods(std::move(methods)) {}
};

// ---------------------------------------------------------------------------
// Program (root)
// ---------------------------------------------------------------------------
struct Program : ASTNode {
    std::vector<ASTNodePtr> statements;

    Program(std::vector<ASTNodePtr> stmts, SourceLoc loc)
        : ASTNode(loc), statements(std::move(stmts)) {}
};

} // namespace boa
