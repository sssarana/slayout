#include "interpreter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>

void Interpreter::interpret(const std::vector<std::shared_ptr<Statement>>& statements) {
    for (const auto& stmt : statements) {
        execute(stmt);
    }
}

void Interpreter::execute(const std::shared_ptr<Statement>& stmt) {
    if (auto def = std::dynamic_pointer_cast<MacroDefStatement>(stmt)) {
        if (macros.count(def->macroName)) {
            throw std::runtime_error("Duplicate macro name: " + def->macroName);
        }
        varToMacroName[def->varName] = def->macroName;
        macros[def->macroName] = Macro{def->macroName};
    }
    else if (auto set = std::dynamic_pointer_cast<SetStatement>(stmt)) {
        std::string macroName = varToMacroName.at(set->varName);
        auto& macro = macros.at(macroName);
        std::string backend = to_lower(set->backend);
        std::string value;
        if (set->isReadFromFile) {
            if (set->parts.empty() || !set->isLiteral[0]) {
                throw std::runtime_error("Expected literal path in read_*");
            }
            value = load_file(set->parts[0]);
        } else {
            for (size_t i = 0; i < set->parts.size(); ++i) {
                if (set->isLiteral[i]) {
                    value += set->parts[i];
                } else {
                    const std::string& varName = set->parts[i];
                    if (!strings.count(varName)) {
                        throw std::runtime_error("Undefined string variable: " + varName);
                    }
                    value += strings.at(varName);
                }
            }
        }

        macro.backendValues[backend] = value;
    }
    else if (auto setd = std::dynamic_pointer_cast<SetDefaultStatement>(stmt)) {
        std::string macroName = varToMacroName.at(setd->varName);
        auto& macro = macros.at(macroName);

        std::string value;

        if (setd->isReadFromFile) {
            if (setd->parts.empty() || !setd->isLiteral[0]) {
                throw std::runtime_error("Expected literal path in read_default");
            }
            value = load_file(setd->parts[0]);
        } else {
            for (size_t i = 0; i < setd->parts.size(); ++i) {
                if (setd->isLiteral[i]) {
                    value += setd->parts[i];
                } else {
                    const std::string& varName = setd->parts[i];
                    if (!strings.count(varName)) {
                        throw std::runtime_error("Undefined string variable: " + varName);
                    }
                    value += strings.at(varName);
                }
            }
        }

        macro.defaultValue = value;
    }
    else if (auto lazy = std::dynamic_pointer_cast<LazyStatement>(stmt)) {
        std::string macroName = varToMacroName.at(lazy->varName);
        macros.at(macroName).lazy = true;
    }
    else if (auto b = std::dynamic_pointer_cast<BooleanDeclaration>(stmt)) {
        bools[b->name] = b->value;
    }
    else if (auto iff = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        bool cond = bools.count(iff->conditionVar) && bools[iff->conditionVar];
        if (cond) {
            execute_block(iff->body);
        }
    }
    else if (auto genAll = std::dynamic_pointer_cast<GenerateAllStatement>(stmt)) {
        requiredBackends = {"glsl", "hlsl", "msl", "spirv"};
    }
    else if (auto genSel = std::dynamic_pointer_cast<GenerateSelectStatement>(stmt)) {
        for (auto b : genSel->backends) {
            switch (b) {
                case Backend::GLSL:  requiredBackends.insert("glsl");  break;
                case Backend::HLSL:  requiredBackends.insert("hlsl");  break;
                case Backend::MSL:   requiredBackends.insert("msl");   break;
                case Backend::SPIRV: requiredBackends.insert("spirv"); break;
                default: break;
            }
        }
    }
    else if (auto print = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        if (print->expression == "list_backends") {
            std::cout << "Supported backends: glsl, hlsl, msl, spirv\n";
        } else {
            std::cerr << "Unknown print expression: " << print->expression << "\n";
        }
    }
    else if (auto s = std::dynamic_pointer_cast<StringDeclaration>(stmt)) {
        strings[s->name] = s->expression;
    }
    else {
        throw std::runtime_error("Unknown statement type in interpreter");
    }
}

void Interpreter::execute_block(const std::vector<std::shared_ptr<Statement>>& body) {
    for (const auto& stmt : body) {
        execute(stmt);
    }
}

std::string Interpreter::load_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to read file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

const std::map<std::string, Macro>& Interpreter::get_macros() const {
    return macros;
}

const std::set<std::string>& Interpreter::get_required_backends() const {
    return requiredBackends;
}

std::string Interpreter::to_lower(const std::string& s) const {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

using json = nlohmann::json;

void Interpreter::export_macro_metadata(const std::string& outputPath) const {
    json j;
    for (const auto& [macroName, macro] : macros) {
        json macroJson;
        macroJson["lazy"] = macro.lazy;
        if (!macro.defaultValue.empty()) {
            macroJson["default"] = macro.defaultValue;
        }
        for (const auto& [backend, code] : macro.backendValues) {
            macroJson[backend] = code;
        }
        j[macroName] = macroJson;
    }

    std::ofstream out(outputPath);
    if (!out) {
        throw std::runtime_error("Failed to write macro metadata to: " + outputPath);
    }
    out << j.dump(4);
}