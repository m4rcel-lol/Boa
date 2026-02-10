#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <unordered_map>
#include <sstream>

namespace boa {

enum class TokenType {
    // Keywords
    Fn,
    Imp,
    Ret,
    If,
    Elif,
    Else,
    For,
    In,
    While,
    Try,
    Except,
    Finally,
    Pass,
    And,
    Or,
    Not,
    True,
    False,
    None,
    Class,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    DoubleStar,
    EqEq,
    BangEq,
    Less,
    LessEq,
    Greater,
    GreaterEq,
    Eq,
    PlusEq,
    MinusEq,
    StarEq,
    SlashEq,

    // Delimiters
    LParen,
    RParen,
    LBracket,
    RBracket,
    LBrace,
    RBrace,
    Colon,
    Comma,
    Dot,

    // Literals
    Int,
    Float,
    String,

    // Structural
    Indent,
    Dedent,
    Newline,
    Eof,

    // Identifier
    Identifier,
};

inline std::string token_type_to_string(TokenType t) {
    switch (t) {
        case TokenType::Fn:        return "Fn";
        case TokenType::Imp:       return "Imp";
        case TokenType::Ret:       return "Ret";
        case TokenType::If:        return "If";
        case TokenType::Elif:      return "Elif";
        case TokenType::Else:      return "Else";
        case TokenType::For:       return "For";
        case TokenType::In:        return "In";
        case TokenType::While:     return "While";
        case TokenType::Try:       return "Try";
        case TokenType::Except:    return "Except";
        case TokenType::Finally:   return "Finally";
        case TokenType::Pass:      return "Pass";
        case TokenType::And:       return "And";
        case TokenType::Or:        return "Or";
        case TokenType::Not:       return "Not";
        case TokenType::True:      return "True";
        case TokenType::False:     return "False";
        case TokenType::None:      return "None";
        case TokenType::Class:     return "Class";
        case TokenType::Plus:      return "Plus";
        case TokenType::Minus:     return "Minus";
        case TokenType::Star:      return "Star";
        case TokenType::Slash:     return "Slash";
        case TokenType::Percent:   return "Percent";
        case TokenType::DoubleStar: return "DoubleStar";
        case TokenType::EqEq:     return "EqEq";
        case TokenType::BangEq:   return "BangEq";
        case TokenType::Less:     return "Less";
        case TokenType::LessEq:   return "LessEq";
        case TokenType::Greater:  return "Greater";
        case TokenType::GreaterEq: return "GreaterEq";
        case TokenType::Eq:       return "Eq";
        case TokenType::PlusEq:   return "PlusEq";
        case TokenType::MinusEq:  return "MinusEq";
        case TokenType::StarEq:   return "StarEq";
        case TokenType::SlashEq:  return "SlashEq";
        case TokenType::LParen:   return "LParen";
        case TokenType::RParen:   return "RParen";
        case TokenType::LBracket: return "LBracket";
        case TokenType::RBracket: return "RBracket";
        case TokenType::LBrace:   return "LBrace";
        case TokenType::RBrace:   return "RBrace";
        case TokenType::Colon:    return "Colon";
        case TokenType::Comma:    return "Comma";
        case TokenType::Dot:      return "Dot";
        case TokenType::Int:      return "Int";
        case TokenType::Float:    return "Float";
        case TokenType::String:   return "String";
        case TokenType::Indent:   return "Indent";
        case TokenType::Dedent:   return "Dedent";
        case TokenType::Newline:  return "Newline";
        case TokenType::Eof:      return "Eof";
        case TokenType::Identifier: return "Identifier";
    }
    return "Unknown";
}

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

class LexerError : public std::runtime_error {
public:
    int line;
    int column;

    LexerError(const std::string& message, int line, int column)
        : std::runtime_error(format_message(message, line, column))
        , line(line)
        , column(column) {}

private:
    static std::string format_message(const std::string& msg, int ln, int col) {
        std::ostringstream oss;
        oss << "LexerError at line " << ln << ", column " << col << ": " << msg;
        return oss.str();
    }
};

class Lexer {
public:
    explicit Lexer(const std::string& source)
        : source_(source)
        , pos_(0)
        , line_(1)
        , column_(1)
        , at_line_start_(true) {
        indent_stack_.push_back(0);
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (!at_end()) {
            // At the start of a logical line, handle indentation
            if (at_line_start_) {
                handle_indentation(tokens);
                at_line_start_ = false;
                if (at_end()) break;
            }

            char c = peek();

            // Skip spaces/tabs within a line (not at line start)
            if (c == ' ' || c == '\t') {
                advance();
                continue;
            }

            // Newline
            if (c == '\n') {
                tokens.push_back({TokenType::Newline, "\\n", line_, column_});
                advance();
                at_line_start_ = true;
                continue;
            }

            // Carriage return (handle \r\n)
            if (c == '\r') {
                advance();
                if (!at_end() && peek() == '\n') {
                    advance();
                }
                tokens.push_back({TokenType::Newline, "\\n", line_ - 1, column_});
                at_line_start_ = true;
                continue;
            }

            // Comment
            if (c == '#') {
                skip_comment();
                continue;
            }

            // String literal
            if (c == '"' || c == '\'') {
                tokens.push_back(read_string());
                continue;
            }

            // Numeric literal
            if (std::isdigit(c)) {
                tokens.push_back(read_number());
                continue;
            }

            // Identifier or keyword
            if (std::isalpha(c) || c == '_') {
                tokens.push_back(read_identifier_or_keyword());
                continue;
            }

            // Operators and delimiters
            Token op_token = read_operator_or_delimiter();
            tokens.push_back(op_token);
        }

        // Emit a final newline if the last token isn't one
        if (!tokens.empty() && tokens.back().type != TokenType::Newline) {
            tokens.push_back({TokenType::Newline, "\\n", line_, column_});
        }

        // Emit DEDENT tokens to close all open indentation levels
        while (indent_stack_.size() > 1) {
            indent_stack_.pop_back();
            tokens.push_back({TokenType::Dedent, "", line_, column_});
        }

        tokens.push_back({TokenType::Eof, "", line_, column_});
        return tokens;
    }

private:
    std::string source_;
    size_t pos_;
    int line_;
    int column_;
    bool at_line_start_;
    std::vector<int> indent_stack_;

    static const std::unordered_map<std::string, TokenType>& keywords() {
        static const std::unordered_map<std::string, TokenType> kw = {
            {"fn",      TokenType::Fn},
            {"imp",     TokenType::Imp},
            {"ret",     TokenType::Ret},
            {"if",      TokenType::If},
            {"elif",    TokenType::Elif},
            {"else",    TokenType::Else},
            {"for",     TokenType::For},
            {"in",      TokenType::In},
            {"while",   TokenType::While},
            {"try",     TokenType::Try},
            {"except",  TokenType::Except},
            {"finally", TokenType::Finally},
            {"pass",    TokenType::Pass},
            {"and",     TokenType::And},
            {"or",      TokenType::Or},
            {"not",     TokenType::Not},
            {"true",    TokenType::True},
            {"false",   TokenType::False},
            {"none",    TokenType::None},
            {"class",   TokenType::Class},
        };
        return kw;
    }

    bool at_end() const {
        return pos_ >= source_.size();
    }

    char peek() const {
        return source_[pos_];
    }

    char peek_next() const {
        if (pos_ + 1 >= source_.size()) return '\0';
        return source_[pos_ + 1];
    }

    char advance() {
        char c = source_[pos_++];
        if (c == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        return c;
    }

    void handle_indentation(std::vector<Token>& tokens) {
        int indent = 0;
        int indent_line = line_;
        int indent_col = column_;

        while (!at_end()) {
            char c = peek();
            if (c == ' ') {
                indent++;
                advance();
            } else if (c == '\t') {
                // Tab advances indent to the next column that is a multiple of 8.
                // E.g. at position 3, a tab jumps to 8; at position 8, it jumps to 16.
                indent = ((indent / 8) + 1) * 8;
                advance();
            } else {
                break;
            }
        }

        // Blank line or comment-only line: skip indentation processing
        if (at_end() || peek() == '\n' || peek() == '\r' || peek() == '#') {
            return;
        }

        int current = indent_stack_.back();

        if (indent > current) {
            indent_stack_.push_back(indent);
            tokens.push_back({TokenType::Indent, "", indent_line, indent_col});
        } else if (indent < current) {
            // Dedent: may need to pop multiple levels
            while (indent_stack_.size() > 1 && indent_stack_.back() > indent) {
                indent_stack_.pop_back();
                tokens.push_back({TokenType::Dedent, "", indent_line, indent_col});
            }
            if (indent_stack_.back() != indent) {
                throw LexerError(
                    "unindent does not match any outer indentation level",
                    indent_line, indent_col);
            }
        }
    }

    void skip_comment() {
        while (!at_end() && peek() != '\n') {
            advance();
        }
    }

    Token read_string() {
        int start_line = line_;
        int start_col = column_;
        char quote = advance();
        std::string value;

        while (!at_end()) {
            char c = peek();

            if (c == '\n') {
                throw LexerError("unterminated string literal (newline in string)",
                                 start_line, start_col);
            }

            if (c == '\\') {
                advance();
                if (at_end()) {
                    throw LexerError("unterminated escape sequence at end of input",
                                     line_, column_);
                }
                char escaped = advance();
                switch (escaped) {
                    case 'n':  value += '\n'; break;
                    case 't':  value += '\t'; break;
                    case 'r':  value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '\'': value += '\''; break;
                    case '"':  value += '"';  break;
                    case '0':  value += '\0'; break;
                    default:
                        throw LexerError(
                            std::string("invalid escape sequence: \\") + escaped,
                            line_, column_ - 1);
                }
                continue;
            }

            if (c == quote) {
                advance();
                return {TokenType::String, value, start_line, start_col};
            }

            value += advance();
        }

        throw LexerError("unterminated string literal (reached end of input)",
                         start_line, start_col);
    }

    Token read_number() {
        int start_line = line_;
        int start_col = column_;
        std::string value;
        bool is_float = false;

        while (!at_end() && std::isdigit(peek())) {
            value += advance();
        }

        if (!at_end() && peek() == '.' && std::isdigit(peek_next())) {
            is_float = true;
            value += advance(); // '.'
            while (!at_end() && std::isdigit(peek())) {
                value += advance();
            }
        }

        // Scientific notation
        if (!at_end() && (peek() == 'e' || peek() == 'E')) {
            is_float = true;
            value += advance();
            if (!at_end() && (peek() == '+' || peek() == '-')) {
                value += advance();
            }
            if (at_end() || !std::isdigit(peek())) {
                throw LexerError("invalid numeric literal: expected digit after exponent",
                                 line_, column_);
            }
            while (!at_end() && std::isdigit(peek())) {
                value += advance();
            }
        }

        return {is_float ? TokenType::Float : TokenType::Int, value, start_line, start_col};
    }

    Token read_identifier_or_keyword() {
        int start_line = line_;
        int start_col = column_;
        std::string value;

        while (!at_end() && (std::isalnum(peek()) || peek() == '_')) {
            value += advance();
        }

        auto& kw = keywords();
        auto it = kw.find(value);
        if (it != kw.end()) {
            return {it->second, value, start_line, start_col};
        }
        return {TokenType::Identifier, value, start_line, start_col};
    }

    Token read_operator_or_delimiter() {
        int start_line = line_;
        int start_col = column_;
        char c = advance();

        switch (c) {
            case '(':  return {TokenType::LParen,   "(", start_line, start_col};
            case ')':  return {TokenType::RParen,   ")", start_line, start_col};
            case '[':  return {TokenType::LBracket, "[", start_line, start_col};
            case ']':  return {TokenType::RBracket, "]", start_line, start_col};
            case '{':  return {TokenType::LBrace,   "{", start_line, start_col};
            case '}':  return {TokenType::RBrace,   "}", start_line, start_col};
            case ':':  return {TokenType::Colon,    ":", start_line, start_col};
            case ',':  return {TokenType::Comma,    ",", start_line, start_col};
            case '.':  return {TokenType::Dot,      ".", start_line, start_col};
            case '%':  return {TokenType::Percent,  "%", start_line, start_col};

            case '+':
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::PlusEq, "+=", start_line, start_col};
                }
                return {TokenType::Plus, "+", start_line, start_col};

            case '-':
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::MinusEq, "-=", start_line, start_col};
                }
                return {TokenType::Minus, "-", start_line, start_col};

            case '*':
                if (!at_end() && peek() == '*') {
                    advance();
                    return {TokenType::DoubleStar, "**", start_line, start_col};
                }
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::StarEq, "*=", start_line, start_col};
                }
                return {TokenType::Star, "*", start_line, start_col};

            case '/':
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::SlashEq, "/=", start_line, start_col};
                }
                return {TokenType::Slash, "/", start_line, start_col};

            case '=':
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::EqEq, "==", start_line, start_col};
                }
                return {TokenType::Eq, "=", start_line, start_col};

            case '!':
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::BangEq, "!=", start_line, start_col};
                }
                throw LexerError(
                    "unexpected character '!' (did you mean '!='?)",
                    start_line, start_col);

            case '<':
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::LessEq, "<=", start_line, start_col};
                }
                return {TokenType::Less, "<", start_line, start_col};

            case '>':
                if (!at_end() && peek() == '=') {
                    advance();
                    return {TokenType::GreaterEq, ">=", start_line, start_col};
                }
                return {TokenType::Greater, ">", start_line, start_col};

            default:
                throw LexerError(
                    std::string("unexpected character: '") + c + "'",
                    start_line, start_col);
        }
    }
};

} // namespace boa
