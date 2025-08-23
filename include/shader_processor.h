#pragma once
#include "interpreter.h"
#include <string>

class ShaderProcessor {
public:
    static void process_shader(const std::string& inputShaderPath,
                               const std::string& outputPath,
                               const std::map<std::string, Macro>& macros,
                               const std::string& backendName);
};
