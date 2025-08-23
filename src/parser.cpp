#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

const Token& Parser::advance() {
    if (!check(TokenType::EndOfFile)) current++;
    return previous();
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

void Parser::consume(TokenType type, const std::string& errorMessage) {
    if (!match(type)) {
        throw std::runtime_error(errorMessage + " at line " + std::to_string(peek().line));
    }
}

std::vector<std::shared_ptr<Statement>> Parser::parse() {
    std::vector<std::shared_ptr<Statement>> statements;
    while (!check(TokenType::EndOfFile)) {
        statements.push_back(parse_statement());
    }
    return statements;
}

std::shared_ptr<Statement> Parser::parse_statement() {
    if (match(TokenType::Keyword)) {
        std::string keyword = previous().value;
        if (keyword == "macrodef") return parse_macro_def();
        if (keyword == "boolean") return parse_boolean();
        if (keyword == "if") return parse_if();
        if (keyword == "SYSTEM") return parse_generate();
        if (keyword == "print") return parse_print();
        if (keyword == "string")   return parse_string();
    }

    if (check(TokenType::Identifier)) return parse_set_or_read();

    throw std::runtime_error("Unknown statement at line " + std::to_string(peek().line));
}

std::shared_ptr<Statement> Parser::parse_macro_def() {
    std::string varName;
    std::string macroName;

    consume(TokenType::Identifier, "Expected macro variable name");
    varName = previous().value;

    consume(TokenType::Equals, "Expected '='");
    consume(TokenType::Keyword, "Expected 'Macro'");
    if (previous().value != "Macro") throw std::runtime_error("Expected 'Macro(...)'");
    consume(TokenType::LParen, "Expected '('");
    consume(TokenType::String, "Expected macro name string");
    macroName = previous().value;
    consume(TokenType::RParen, "Expected ')'");
    consume(TokenType::Semicolon, "Expected ';'");

    auto stmt = std::make_shared<MacroDefStatement>();
    stmt->varName = varName;
    stmt->macroName = macroName;
    return stmt;
}


// for c++ < 20:
bool starts_with(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

std::shared_ptr<Statement> Parser::parse_set_or_read() {
    std::string varName = advance().value;

    consume(TokenType::Dot, "Expected '.'");
    consume(TokenType::Keyword, "Expected setter/reader function");

    std::string func = previous().value;

    if (func == "lazy") {
        consume(TokenType::LParen, "Expected '(' after lazy");
        consume(TokenType::RParen, "Expected ')' after lazy()");
        consume(TokenType::Semicolon, "Expected ';' after lazy()");
        auto stmt = std::make_shared<LazyStatement>();
        stmt->varName = varName;
        return stmt;
    }

    bool isRead = starts_with(func, "read_");
    bool isDefault = func.find("default") != std::string::npos;

    consume(TokenType::LParen, "Expected '('");

    std::vector<std::string> parts;
    std::vector<bool> isLiteral;

    do {
        if (match(TokenType::String)) {
            parts.push_back(previous().value);
            isLiteral.push_back(true);
        } else if (match(TokenType::Identifier)) {
            parts.push_back(previous().value);
            isLiteral.push_back(false);
        } else {
            throw std::runtime_error("Expected string literal or string variable inside " + func);
        }
    } while (match(TokenType::Plus));

    consume(TokenType::RParen, "Expected ')'");
    consume(TokenType::Semicolon, "Expected ';'");

    if (isDefault) {
        auto stmt = std::make_shared<SetDefaultStatement>();
        stmt->varName = varName;
        stmt->parts = parts;
        stmt->isLiteral = isLiteral;
        stmt->isReadFromFile = isRead;
        return stmt;
    } else {
        auto stmt = std::make_shared<SetStatement>();
        stmt->varName = varName;
        stmt->isReadFromFile = isRead;
        stmt->parts = parts;
        stmt->isLiteral = isLiteral;

        size_t underscore = func.find('_');
        if (underscore != std::string::npos) {
            stmt->backend = func.substr(underscore + 1);
        } else {
            throw std::runtime_error("Invalid set/read function name");
        }

        return stmt;
    }
}


std::shared_ptr<Statement> Parser::parse_lazy() {
    throw std::runtime_error("lazy() must be used as a method on a macro");
}

std::shared_ptr<Statement> Parser::parse_boolean() {
    consume(TokenType::Identifier, "Expected boolean variable name");
    std::string name = previous().value;

    consume(TokenType::Equals, "Expected '='");
    consume(TokenType::Boolean, "Expected TRUE or FALSE");

    bool value = previous().value == "TRUE";
    consume(TokenType::Semicolon, "Expected ';'");

    auto stmt = std::make_shared<BooleanDeclaration>();
    stmt->name = name;
    stmt->value = value;
    return stmt;
}

std::shared_ptr<Statement> Parser::parse_if() {
    consume(TokenType::LParen, "Expected '(' after if");
    consume(TokenType::Identifier, "Expected condition variable");
    std::string conditionVar = previous().value;
    consume(TokenType::RParen, "Expected ')'");

    consume(TokenType::LBrace, "Expected '{'");

    std::vector<std::shared_ptr<Statement>> body;
    while (!check(TokenType::RBrace) && !check(TokenType::EndOfFile)) {
        body.push_back(parse_statement());
    }

    consume(TokenType::RBrace, "Expected '}'");

    auto stmt = std::make_shared<IfStatement>();
    stmt->conditionVar = conditionVar;
    stmt->body = body;
    return stmt;
}

std::shared_ptr<Statement> Parser::parse_generate() {
    consume(TokenType::Arrow, "Expected '->'");
    consume(TokenType::Keyword, "Expected generate function");

    std::string func = previous().value;
    consume(TokenType::LParen, "Expected '('");

    if (func == "generate_all") {
        consume(TokenType::RParen, "Expected ')'");
        consume(TokenType::Semicolon, "Expected ';'");
        return std::make_shared<GenerateAllStatement>();
    } else if (func == "generate_select") {
        std::vector<Backend> backends;

        while (!check(TokenType::RParen)) {
            consume(TokenType::Identifier, "Expected backend name");
            backends.push_back(parse_backend_enum(previous().value));
            if (!check(TokenType::RParen)) {
                consume(TokenType::Comma, "Expected ',' between backends");
            }
        }

        consume(TokenType::RParen, "Expected ')'");
        consume(TokenType::Semicolon, "Expected ';'");

        auto stmt = std::make_shared<GenerateSelectStatement>();
        stmt->backends = backends;
        return stmt;
    }

    throw std::runtime_error("Unknown generate function: " + func);
}

std::shared_ptr<Statement> Parser::parse_print() {
    consume(TokenType::Keyword, "Expected SYSTEM");
    if (previous().value != "SYSTEM") throw std::runtime_error("Expected SYSTEM");

    consume(TokenType::Arrow, "Expected '->'");
    consume(TokenType::Identifier, "Expected list_backends");
    std::string expr = previous().value;

    consume(TokenType::LParen, "Expected '('");
    consume(TokenType::RParen, "Expected ')'");
    consume(TokenType::Semicolon, "Expected ';'");

    auto stmt = std::make_shared<PrintStatement>();
    stmt->expression = expr;
    return stmt;
}

Backend Parser::parse_backend_enum(const std::string& value) {
    if (value == "GLSL") return Backend::GLSL;
    if (value == "HLSL") return Backend::HLSL;
    if (value == "MSL") return Backend::MSL;
    if (value == "SPIRV") return Backend::SPIRV;
    return Backend::UNKNOWN;
}

std::shared_ptr<Statement> Parser::parse_string() {
    consume(TokenType::Identifier, "Expected variable name after 'string'");
    std::string name = previous().value;

    consume(TokenType::Equals, "Expected '=' after variable name");

    std::string result;
    consume(TokenType::String, "Expected string or identifier");
    result += previous().value;

    while (match(TokenType::Plus)) {
        if (match(TokenType::String) || match(TokenType::Identifier)) {
            result += previous().value;
        } else {
            throw std::runtime_error("Expected string or identifier after '+'");
        }
    }

    consume(TokenType::Semicolon, "Expected ';' after string declaration");

    auto stmt = std::make_shared<StringDeclaration>();
    stmt->name = name;
    stmt->expression = result;
    return stmt;
}