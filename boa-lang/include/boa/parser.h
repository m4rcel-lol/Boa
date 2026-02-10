#pragma once

#include "token.h"
#include "ast.h"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace boa {

// ---------------------------------------------------------------------------
// Parse error
// ---------------------------------------------------------------------------
class ParseError : public std::runtime_error {
public:
    int line;
    int column;

    ParseError(const std::string& message, int line, int column)
        : std::runtime_error(format(message, line, column))
        , line(line)
        , column(column) {}

private:
    static std::string format(const std::string& msg, int ln, int col) {
        std::ostringstream oss;
        oss << "ParseError at line " << ln << ", column " << col << ": " << msg;
        return oss.str();
    }
};

// ---------------------------------------------------------------------------
// Recursive-descent parser
// ---------------------------------------------------------------------------
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens)
        : tokens_(tokens), pos_(0) {}

    // Entry point: parse a complete program.
    std::unique_ptr<Program> parse_program() {
        auto loc = current_loc();
        std::vector<ASTNodePtr> stmts;
        skip_newlines();
        while (!check(TokenType::Eof)) {
            stmts.push_back(parse_statement());
            skip_newlines();
        }
        return std::make_unique<Program>(std::move(stmts), loc);
    }

private:
    const std::vector<Token>& tokens_;
    size_t pos_;

    // ------------------------------------------------------------------
    // Token helpers
    // ------------------------------------------------------------------
    const Token& current() const {
        if (pos_ < tokens_.size()) return tokens_[pos_];
        return tokens_.back(); // Eof sentinel
    }

    SourceLoc current_loc() const {
        const auto& t = current();
        return {t.line, t.column};
    }

    bool check(TokenType t) const { return current().type == t; }

    bool match(TokenType t) {
        if (check(t)) { advance(); return true; }
        return false;
    }

    const Token& advance() {
        const Token& t = current();
        if (pos_ < tokens_.size()) ++pos_;
        return t;
    }

    const Token& expect(TokenType t, const std::string& ctx = "") {
        if (check(t)) return advance();
        const auto& c = current();
        std::ostringstream oss;
        oss << "expected " << token_type_to_string(t);
        if (!ctx.empty()) oss << " " << ctx;
        oss << ", got " << token_type_to_string(c.type);
        if (!c.value.empty()) oss << " '" << c.value << "'";
        throw ParseError(oss.str(), c.line, c.column);
    }

    void skip_newlines() {
        while (check(TokenType::Newline)) advance();
    }

    // ------------------------------------------------------------------
    // Indented block: expects INDENT { statement } DEDENT
    // ------------------------------------------------------------------
    std::vector<ASTNodePtr> parse_block() {
        expect(TokenType::Newline, "before indented block");
        skip_newlines();
        expect(TokenType::Indent, "to start indented block");
        skip_newlines();

        std::vector<ASTNodePtr> stmts;
        while (!check(TokenType::Dedent) && !check(TokenType::Eof)) {
            stmts.push_back(parse_statement());
            skip_newlines();
        }
        expect(TokenType::Dedent, "to end indented block");
        return stmts;
    }

    // ------------------------------------------------------------------
    // Statements
    // ------------------------------------------------------------------
    ASTNodePtr parse_statement() {
        skip_newlines();
        switch (current().type) {
            case TokenType::Fn:      return parse_fn_def();
            case TokenType::If:      return parse_if_stmt();
            case TokenType::For:     return parse_for_stmt();
            case TokenType::While:   return parse_while_stmt();
            case TokenType::Ret:     return parse_return_stmt();
            case TokenType::Imp:     return parse_import_stmt();
            case TokenType::Try:     return parse_try_stmt();
            case TokenType::Pass:    return parse_pass_stmt();
            case TokenType::Class:   return parse_class_def();
            default:                 return parse_assignment_or_expr_stmt();
        }
    }

    // fn name(params): block
    ASTNodePtr parse_fn_def() {
        auto loc = current_loc();
        expect(TokenType::Fn);
        auto name = expect(TokenType::Identifier, "as function name").value;
        expect(TokenType::LParen);
        auto params = parse_param_list();
        expect(TokenType::RParen);
        expect(TokenType::Colon);
        auto body = parse_block();
        return std::make_unique<FnDef>(std::move(name), std::move(params),
                                       std::move(body), loc);
    }

    std::vector<std::string> parse_param_list() {
        std::vector<std::string> params;
        if (check(TokenType::RParen)) return params;
        params.push_back(expect(TokenType::Identifier, "as parameter name").value);
        while (match(TokenType::Comma)) {
            params.push_back(expect(TokenType::Identifier, "as parameter name").value);
        }
        return params;
    }

    // if cond: block [elif cond: block]* [else: block]
    ASTNodePtr parse_if_stmt() {
        auto loc = current_loc();
        expect(TokenType::If);
        auto cond = parse_expression();
        expect(TokenType::Colon);
        auto body = parse_block();

        std::vector<ElifClause> elifs;
        skip_newlines();
        while (check(TokenType::Elif)) {
            advance();
            auto elif_cond = parse_expression();
            expect(TokenType::Colon);
            auto elif_body = parse_block();
            elifs.push_back({std::move(elif_cond), std::move(elif_body)});
            skip_newlines();
        }

        std::vector<ASTNodePtr> else_body;
        if (check(TokenType::Else)) {
            advance();
            expect(TokenType::Colon);
            else_body = parse_block();
        }

        return std::make_unique<IfStmt>(std::move(cond), std::move(body),
                                         std::move(elifs), std::move(else_body),
                                         loc);
    }

    // for var in iterable: block
    ASTNodePtr parse_for_stmt() {
        auto loc = current_loc();
        expect(TokenType::For);
        auto var = expect(TokenType::Identifier, "as loop variable").value;
        expect(TokenType::In);
        auto iter = parse_expression();
        expect(TokenType::Colon);
        auto body = parse_block();
        return std::make_unique<ForStmt>(std::move(var), std::move(iter),
                                          std::move(body), loc);
    }

    // while cond: block
    ASTNodePtr parse_while_stmt() {
        auto loc = current_loc();
        expect(TokenType::While);
        auto cond = parse_expression();
        expect(TokenType::Colon);
        auto body = parse_block();
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body), loc);
    }

    // ret [expr]
    ASTNodePtr parse_return_stmt() {
        auto loc = current_loc();
        expect(TokenType::Ret);
        ASTNodePtr value;
        if (!check(TokenType::Newline) && !check(TokenType::Eof) &&
            !check(TokenType::Dedent)) {
            value = parse_expression();
        }
        expect_end_of_stmt();
        return std::make_unique<ReturnStmt>(std::move(value), loc);
    }

    // imp module [, module]*
    ASTNodePtr parse_import_stmt() {
        auto loc = current_loc();
        expect(TokenType::Imp);
        std::vector<std::string> modules;
        modules.push_back(expect(TokenType::Identifier, "as module name").value);
        while (match(TokenType::Comma)) {
            modules.push_back(expect(TokenType::Identifier, "as module name").value);
        }
        expect_end_of_stmt();
        return std::make_unique<ImportStmt>(std::move(modules), loc);
    }

    // try: block except [var]: block [finally: block]
    ASTNodePtr parse_try_stmt() {
        auto loc = current_loc();
        expect(TokenType::Try);
        expect(TokenType::Colon);
        auto try_body = parse_block();

        std::string except_var;
        std::vector<ASTNodePtr> except_body;
        skip_newlines();
        if (check(TokenType::Except)) {
            advance();
            if (check(TokenType::Identifier)) {
                except_var = advance().value;
            }
            expect(TokenType::Colon);
            except_body = parse_block();
        }

        std::vector<ASTNodePtr> finally_body;
        skip_newlines();
        if (check(TokenType::Finally)) {
            advance();
            expect(TokenType::Colon);
            finally_body = parse_block();
        }

        return std::make_unique<TryStmt>(std::move(try_body),
                                          std::move(except_var),
                                          std::move(except_body),
                                          std::move(finally_body), loc);
    }

    // pass
    ASTNodePtr parse_pass_stmt() {
        auto loc = current_loc();
        expect(TokenType::Pass);
        expect_end_of_stmt();
        return std::make_unique<PassStmt>(loc);
    }

    // class Name: block
    ASTNodePtr parse_class_def() {
        auto loc = current_loc();
        expect(TokenType::Class);
        auto name = expect(TokenType::Identifier, "as class name").value;
        expect(TokenType::Colon);
        auto methods = parse_block();
        return std::make_unique<ClassDef>(std::move(name), std::move(methods), loc);
    }

    // Assignment or expression statement
    ASTNodePtr parse_assignment_or_expr_stmt() {
        auto loc = current_loc();
        auto expr = parse_expression();

        if (is_assign_op(current().type)) {
            TokenType op = advance().type;
            auto value = parse_expression();
            expect_end_of_stmt();
            return std::make_unique<Assignment>(std::move(expr), op,
                                                 std::move(value), loc);
        }

        expect_end_of_stmt();
        return std::make_unique<ExpressionStmt>(std::move(expr), loc);
    }

    static bool is_assign_op(TokenType t) {
        return t == TokenType::Eq      || t == TokenType::PlusEq  ||
               t == TokenType::MinusEq || t == TokenType::StarEq  ||
               t == TokenType::SlashEq;
    }

    void expect_end_of_stmt() {
        if (check(TokenType::Newline) || check(TokenType::Eof) ||
            check(TokenType::Dedent)) {
            if (check(TokenType::Newline)) advance();
            return;
        }
        const auto& c = current();
        throw ParseError("expected end of statement, got " +
                          token_type_to_string(c.type) + " '" + c.value + "'",
                          c.line, c.column);
    }

    // ------------------------------------------------------------------
    // Expressions (precedence climbing)
    //   assignment < or < and < not < comparison < add < mul < unary < power < postfix < atom
    // ------------------------------------------------------------------
    ASTNodePtr parse_expression() {
        return parse_or_expr();
    }

    // or
    ASTNodePtr parse_or_expr() {
        auto left = parse_and_expr();
        while (check(TokenType::Or)) {
            auto loc = current_loc();
            advance();
            auto right = parse_and_expr();
            left = std::make_unique<BinaryOp>(std::move(left), TokenType::Or,
                                               std::move(right), loc);
        }
        return left;
    }

    // and
    ASTNodePtr parse_and_expr() {
        auto left = parse_not_expr();
        while (check(TokenType::And)) {
            auto loc = current_loc();
            advance();
            auto right = parse_not_expr();
            left = std::make_unique<BinaryOp>(std::move(left), TokenType::And,
                                               std::move(right), loc);
        }
        return left;
    }

    // not (prefix)
    ASTNodePtr parse_not_expr() {
        if (check(TokenType::Not)) {
            auto loc = current_loc();
            advance();
            auto operand = parse_not_expr();
            return std::make_unique<UnaryOp>(TokenType::Not, std::move(operand), loc);
        }
        return parse_comparison();
    }

    // comparison: == != < <= > >=
    ASTNodePtr parse_comparison() {
        auto left = parse_addition();
        while (is_comparison_op(current().type)) {
            auto loc = current_loc();
            TokenType op = advance().type;
            auto right = parse_addition();
            left = std::make_unique<BinaryOp>(std::move(left), op,
                                               std::move(right), loc);
        }
        return left;
    }

    static bool is_comparison_op(TokenType t) {
        return t == TokenType::EqEq    || t == TokenType::BangEq   ||
               t == TokenType::Less    || t == TokenType::LessEq   ||
               t == TokenType::Greater || t == TokenType::GreaterEq;
    }

    // addition: + -
    ASTNodePtr parse_addition() {
        auto left = parse_multiplication();
        while (check(TokenType::Plus) || check(TokenType::Minus)) {
            auto loc = current_loc();
            TokenType op = advance().type;
            auto right = parse_multiplication();
            left = std::make_unique<BinaryOp>(std::move(left), op,
                                               std::move(right), loc);
        }
        return left;
    }

    // multiplication: * / %
    ASTNodePtr parse_multiplication() {
        auto left = parse_unary();
        while (check(TokenType::Star) || check(TokenType::Slash) ||
               check(TokenType::Percent)) {
            auto loc = current_loc();
            TokenType op = advance().type;
            auto right = parse_unary();
            left = std::make_unique<BinaryOp>(std::move(left), op,
                                               std::move(right), loc);
        }
        return left;
    }

    // unary: - +
    ASTNodePtr parse_unary() {
        if (check(TokenType::Minus) || check(TokenType::Plus)) {
            auto loc = current_loc();
            TokenType op = advance().type;
            auto operand = parse_unary();
            return std::make_unique<UnaryOp>(op, std::move(operand), loc);
        }
        return parse_power();
    }

    // power: base ** exponent  (right-associative)
    ASTNodePtr parse_power() {
        auto base = parse_postfix();
        if (check(TokenType::DoubleStar)) {
            auto loc = current_loc();
            advance();
            auto exp = parse_unary(); // right-associative
            return std::make_unique<BinaryOp>(std::move(base), TokenType::DoubleStar,
                                               std::move(exp), loc);
        }
        return base;
    }

    // postfix: call, index, member access
    ASTNodePtr parse_postfix() {
        auto node = parse_atom();

        for (;;) {
            if (check(TokenType::LParen)) {
                // Function call
                auto loc = current_loc();
                advance();
                auto args = parse_arg_list();
                expect(TokenType::RParen, "to close function call");
                node = std::make_unique<FunctionCall>(std::move(node),
                                                      std::move(args), loc);
            } else if (check(TokenType::LBracket)) {
                // Index
                auto loc = current_loc();
                advance();
                auto index = parse_expression();
                expect(TokenType::RBracket, "to close index expression");
                node = std::make_unique<IndexExpr>(std::move(node),
                                                    std::move(index), loc);
            } else if (check(TokenType::Dot)) {
                // Member access
                auto loc = current_loc();
                advance();
                auto member = expect(TokenType::Identifier, "after '.'").value;
                node = std::make_unique<MemberAccess>(std::move(node),
                                                       std::move(member), loc);
            } else {
                break;
            }
        }
        return node;
    }

    std::vector<ASTNodePtr> parse_arg_list() {
        std::vector<ASTNodePtr> args;
        if (check(TokenType::RParen)) return args;
        args.push_back(parse_expression());
        while (match(TokenType::Comma)) {
            args.push_back(parse_expression());
        }
        return args;
    }

    // ------------------------------------------------------------------
    // Atoms
    // ------------------------------------------------------------------
    ASTNodePtr parse_atom() {
        auto loc = current_loc();
        const auto& tok = current();

        // Number literals
        if (check(TokenType::Int) || check(TokenType::Float)) {
            double val = std::stod(advance().value);
            return std::make_unique<NumberLiteral>(val, loc);
        }

        // String literal
        if (check(TokenType::String)) {
            auto val = advance().value;
            return std::make_unique<StringLiteral>(std::move(val), loc);
        }

        // Bool literals
        if (check(TokenType::True)) {
            advance();
            return std::make_unique<BoolLiteral>(true, loc);
        }
        if (check(TokenType::False)) {
            advance();
            return std::make_unique<BoolLiteral>(false, loc);
        }

        // None
        if (check(TokenType::None)) {
            advance();
            return std::make_unique<NoneLiteral>(loc);
        }

        // Identifier
        if (check(TokenType::Identifier)) {
            auto name = advance().value;
            return std::make_unique<Identifier>(std::move(name), loc);
        }

        // Parenthesised expression
        if (check(TokenType::LParen)) {
            advance();
            auto expr = parse_expression();
            expect(TokenType::RParen, "to close parenthesised expression");
            return expr;
        }

        // List literal [a, b, c]
        if (check(TokenType::LBracket)) {
            return parse_list_literal();
        }

        // Dict literal {k: v, ...}
        if (check(TokenType::LBrace)) {
            return parse_dict_literal();
        }

        throw ParseError("unexpected token " + token_type_to_string(tok.type) +
                          " '" + tok.value + "'", tok.line, tok.column);
    }

    ASTNodePtr parse_list_literal() {
        auto loc = current_loc();
        expect(TokenType::LBracket);
        std::vector<ASTNodePtr> elems;
        if (!check(TokenType::RBracket)) {
            elems.push_back(parse_expression());
            while (match(TokenType::Comma)) {
                if (check(TokenType::RBracket)) break; // trailing comma
                elems.push_back(parse_expression());
            }
        }
        expect(TokenType::RBracket, "to close list literal");
        return std::make_unique<ListLiteral>(std::move(elems), loc);
    }

    ASTNodePtr parse_dict_literal() {
        auto loc = current_loc();
        expect(TokenType::LBrace);
        std::vector<std::pair<ASTNodePtr, ASTNodePtr>> entries;
        if (!check(TokenType::RBrace)) {
            auto key = parse_expression();
            expect(TokenType::Colon, "in dict literal");
            auto val = parse_expression();
            entries.emplace_back(std::move(key), std::move(val));
            while (match(TokenType::Comma)) {
                if (check(TokenType::RBrace)) break; // trailing comma
                auto k = parse_expression();
                expect(TokenType::Colon, "in dict literal");
                auto v = parse_expression();
                entries.emplace_back(std::move(k), std::move(v));
            }
        }
        expect(TokenType::RBrace, "to close dict literal");
        return std::make_unique<DictLiteral>(std::move(entries), loc);
    }
};

} // namespace boa
