#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "headers/syntax_parser.hpp"
#include "headers/semantic_parser.hpp"
#include "headers/icg.hpp"
#include "headers/target_generator.hpp"

int main(int argc, char** argv) {
    std::ifstream file;
    std::ofstream outputFile;
    std::stringstream buffer;
    std::string source_code;
    std::vector<std::string> assembly;

    // Validate input file integrity
    if (argc >= 2 && std::filesystem::exists(argv[1])) {
        if (std::filesystem::is_regular_file(argv[1])) {
            file.open(argv[1]);
        } else {
            std::cerr << "Not a valid input file." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "No input file provided, or the file provided does not exist." << std::endl;
        return 1;
    }

    if (argc >= 3 && !argv[2]) {
        std::cerr << "No output file path provided" << std::endl;
        return 1;
    }

    // Make sure input file is open
    if (!file.is_open()) {
        std::cerr << "An error occured while opening the input file." << std::endl;
        return 1;
    }

    // Read entire file
    buffer << file.rdbuf();
    source_code = buffer.str();
    file.close();

    // Make sure output file is provided
    if (argv[2]) {
        outputFile.open(argv[2]);
    } else {
        std::cerr << "No output file path provided." << std::endl;
        return 1;
    }

    try {
        // Syntax Analysis
        Parser parser(source_code);
        auto program = parser.parseProgram();
        // Semantic Analysis
        SemanticAnalyzer analyzer(program.get());

        // Intermediate Code Generation
        IntermediateCodeGen icg;
        program->accept(icg);
        std::vector<Quad> intermediate_code = icg.getIR();

        // Target code generation
        TargetCodeGen targetGen(intermediate_code);
        assembly = targetGen.getAssembly();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (outputFile.is_open()) {
        for (const auto& line : assembly) {
            outputFile << line << std::endl;
        }
        outputFile.close();
    } else {
        std::cerr << "Unable to create output file" << std::endl;
        return 1;
    }

    // Assemble and link assembly file
    std::string outputFileName(argv[2]);
    std::string assembleCommand = "gcc -c " + outputFileName + " -o out.o";
    system(assembleCommand.c_str());
    if (std::filesystem::exists("out.o")) {
        system("gcc out.o -o out");
    }
    if (std::filesystem::exists("out.exe")) {
        std::filesystem::remove("out.o");
    }

    return 0;
}