#pragma once
#include "parser.h"
#include <map>
#include <set>
#include <string>

struct Macro {
    std::string macroName;
    bool lazy = false;
    std::map<std::string, std::string> backendValues;
    std::string defaultValue;
};

class Interpreter {
public:
    void interpret(const std::vector<std::shared_ptr<Statement>>& statements);

    const std::map<std::string, Macro>& get_macros() const;
    const std::set<std::string>& get_required_backends() const;

private:
    std::map<std::string, Macro> macros;
    std::map<std::string, bool> bools;
    std::set<std::string> requiredBackends;
    std::map<std::string, std::string> varToMacroName;
    std::map<std::string, std::string> strings;

    void execute(const std::shared_ptr<Statement>& stmt);
    void execute_block(const std::vector<std::shared_ptr<Statement>>& body);

    std::string load_file(const std::string& path);

    std::string to_lower(const std::string& s) const;
};
