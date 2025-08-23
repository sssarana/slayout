#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
#include "shader_processor.h"
#include <iostream>
#include <fstream>


int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: slayoutc <layout.slayout> <input.shader> <output_dir>\n";
        return 1;
    }

    std::string slayoutFile = argv[1];
    std::string shaderFile = argv[2];
    std::string outputDir = argv[3];

    std::ifstream file(slayoutFile);
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Tokenizer tokenizer(source);
    auto tokens = tokenizer.tokenize();

    Parser parser(tokens);
    auto statements = parser.parse();

    Interpreter interpreter;
    interpreter.interpret(statements);

    const auto& macros = interpreter.get_macros();
    const auto& backends = interpreter.get_required_backends();

    for (const auto& backend : backends) {
        std::string outputPath = outputDir + "/shader." + backend;
        ShaderProcessor::process_shader(shaderFile, outputPath, macros, backend);
        std::cout << "Generated: " << outputPath << "\n";
    }

    return 0;
}
