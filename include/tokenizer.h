#pragma once
#include <string>
#include <vector>
#include <stdexcept>

enum class TokenType {
    Keyword,
    Identifier,
    String,
    Boolean,

    Equals,
    Dot,
    Semicolon,
    LParen,
    RParen,
    LBrace,
    RBrace,
    Arrow,
    Comma,
    Percent,
    Plus,

    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

class Tokenizer {
public:
    explicit Tokenizer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos;
    int line, column;

    char peek() const;
    char advance();
    bool match(char expected);
    void skip_whitespace_and_comments();
    Token make_token(TokenType type, const std::string& value);
    Token string();
    Token identifier_or_keyword();
    Token symbol();

    bool is_alpha(char c) const;
    bool is_digit(char c) const;
    bool is_alphanumeric(char c) const;
};
