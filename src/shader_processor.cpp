#include "shader_processor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

void ShaderProcessor::process_shader(const std::string& inputShaderPath,
                                     const std::string& outputPath,
                                     const std::map<std::string, Macro>& macros,
                                     const std::string& backendName) {
    std::ifstream infile(inputShaderPath);
    if (!infile.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + inputShaderPath);
    }

    std::stringstream buffer;
    buffer << infile.rdbuf();
    std::string shaderCode = buffer.str();

    // Regex to find %MACRONAME
    std::regex macroRegex(R"(%([A-Za-z_][A-Za-z0-9_]*))");
    std::smatch match;

    std::string output;
    std::string::const_iterator searchStart(shaderCode.cbegin());

    while (std::regex_search(searchStart, shaderCode.cend(), match, macroRegex)) {
        std::string beforeMatch = match.prefix().str();
        std::string macroName = match[1].str();

        output += beforeMatch;

        if (macros.count(macroName)) {
            const Macro& macro = macros.at(macroName);
            std::string lowerBackend = backendName;
            std::transform(lowerBackend.begin(), lowerBackend.end(), lowerBackend.begin(), ::tolower);

            if (macro.backendValues.count(lowerBackend)) {
                output += macro.backendValues.at(lowerBackend);
            } else if (!macro.defaultValue.empty()) {
                output += macro.defaultValue;
            } else if (macro.lazy) {
                output += macro.macroName;
            } else {
                std::cerr << "Warning: No definition found for macro %" << macroName
                          << " for backend " << backendName << "\n";
            }
        } else {
            std::cerr << "Warning: Undefined macro %" << macroName << " found in shader\n";
        }

        searchStart = match.suffix().first;
    }

    output += std::string(searchStart, shaderCode.cend());

    std::ofstream outfile(outputPath);
    if (!outfile.is_open()) {
        throw std::runtime_error("Failed to write to output: " + outputPath);
    }
    outfile << output;
}
