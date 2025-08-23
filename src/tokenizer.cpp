#include "tokenizer.h"
#include <cctype>

Tokenizer::Tokenizer(const std::string& src)
    : source(src), pos(0), line(1), column(1) {}

char Tokenizer::peek() const {
    return pos < source.size() ? source[pos] : '\0';
}

char Tokenizer::advance() {
    if (pos < source.size()) {
        char c = source[pos++];
        if (c == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
        return c;
    }
    return '\0';
}

bool Tokenizer::match(char expected) {
    if (peek() == expected) {
        advance();
        return true;
    }
    return false;
}

void Tokenizer::skip_whitespace_and_comments() {
    while (true) {
        char c = peek();
        if (isspace(c)) {
            advance();
        } else if (c == '/' && pos + 1 < source.size() && source[pos + 1] == '/') {
            while (peek() != '\n' && peek() != '\0') advance();
        } else {
            break;
        }
    }
}

Token Tokenizer::make_token(TokenType type, const std::string& value) {
    return Token{ type, value, line, column };
}

Token Tokenizer::string() {
    advance(); // Skip opening quote
    std::string value;
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\\') { // Escape handling
            advance(); // skip '\'
            value += advance();
        } else {
            value += advance();
        }
    }

    if (peek() == '"') advance();
    else throw std::runtime_error("Unterminated string");

    return make_token(TokenType::String, value);
}

Token Tokenizer::identifier_or_keyword() {
    std::string value;
    while (is_alphanumeric(peek()) || peek() == '_') {
        value += advance();
    }

    if (value == "macrodef" || value == "Macro" ||
        value == "lazy" || value == "boolean" ||
        value == "if" || value == "print" ||
        value == "set_glsl" || value == "set_hlsl" ||
        value == "set_msl" || value == "set_spirv" ||
        value == "read_glsl" || value == "read_hlsl" ||
        value == "read_msl" || value == "read_spirv" ||
        value == "set_default" || value == "read_default" ||
        value == "generate_all" || value == "generate_select" ||
        value == "SYSTEM") {
        return make_token(TokenType::Keyword, value);
    } else if (value == "TRUE" || value == "FALSE") {
        return make_token(TokenType::Boolean, value);
    }

    return make_token(TokenType::Identifier, value);
}

Token Tokenizer::symbol() {
    char c = advance();
    switch (c) {
        case '=': return make_token(TokenType::Equals, "=");
        case '.': return make_token(TokenType::Dot, ".");
        case ';': return make_token(TokenType::Semicolon, ";");
        case '(': return make_token(TokenType::LParen, "(");
        case ')': return make_token(TokenType::RParen, ")");
        case '{': return make_token(TokenType::LBrace, "{");
        case '}': return make_token(TokenType::RBrace, "}");
        case ',': return make_token(TokenType::Comma, ",");
        case '%': return make_token(TokenType::Percent, "%");
        case '-':
            if (match('>')) return make_token(TokenType::Arrow, "->");
            break;
    }

    throw std::runtime_error(std::string("Unexpected character: ") + c);
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;

    while (pos < source.size()) {
        skip_whitespace_and_comments();
        char c = peek();
        if (c == '\0') break;

        if (isalpha(c) || c == '_') {
            tokens.push_back(identifier_or_keyword());
        } else if (c == '"') {
            tokens.push_back(string());
        } else {
            tokens.push_back(symbol());
        }
    }

    tokens.push_back(make_token(TokenType::EndOfFile, ""));
    return tokens;
}

bool Tokenizer::is_alpha(char c) const {
    return std::isalpha(static_cast<unsigned char>(c));
}

bool Tokenizer::is_digit(char c) const {
    return std::isdigit(static_cast<unsigned char>(c));
}

bool Tokenizer::is_alphanumeric(char c) const {
    return is_alpha(c) || is_digit(c);
}
