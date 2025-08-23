#pragma once
#include "tokenizer.h"
#include <memory>
#include <vector>
#include <string>
#include <map>

enum class Backend {
    GLSL,
    HLSL,
    MSL,
    SPIRV,
    UNKNOWN
};

struct Statement {
    virtual ~Statement() = default;
};

struct MacroDefStatement : public Statement {
    std::string varName;
    std::string macroName;
};

struct SetStatement : public Statement {
    std::string varName;
    std::string backend;
    std::string value;
    bool isReadFromFile;
    bool isVariable = false;
};

struct StringDeclaration : public Statement {
    std::string name;
    std::string expression;
};

struct SetDefaultStatement : public Statement {
    std::string varName;
    std::string value;
    bool isReadFromFile;
    bool isVariable = false;
};

struct LazyStatement : public Statement {
    std::string varName;
};

struct BooleanDeclaration : public Statement {
    std::string name;
    bool value;
};

struct IfStatement : public Statement {
    std::string conditionVar;
    std::vector<std::shared_ptr<Statement>> body;
};

struct GenerateAllStatement : public Statement {};
struct GenerateSelectStatement : public Statement {
    std::vector<Backend> backends;
};

struct PrintStatement : public Statement {
    std::string expression;  // e.g., SYSTEM->list_backends()
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::vector<std::shared_ptr<Statement>> parse();

private:
    std::vector<Token> tokens;
    size_t current;

    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    void consume(TokenType type, const std::string& errorMessage);

    std::shared_ptr<Statement> parse_statement();
    std::shared_ptr<Statement> parse_macro_def();
    std::shared_ptr<Statement> parse_set_or_read();
    std::shared_ptr<Statement> parse_lazy();
    std::shared_ptr<Statement> parse_boolean();
    std::shared_ptr<Statement> parse_if();
    std::shared_ptr<Statement> parse_generate();
    std::shared_ptr<Statement> parse_print();
    std::shared_ptr<Statement> parse_string();

    Backend parse_backend_enum(const std::string& value);
};
