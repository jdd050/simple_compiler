#pragma once

#include <unordered_map>
#include "icg.hpp"

// Generate assembly code from IC
class TargetCodeGen {
private:
    std::vector<std::string> assembly;
    int stringLiteralCount = 0;
    
    // Method to make file header
    void generateHeader() {
        assembly.push_back(".text");
        assembly.push_back(".section .rdata,\"dr\"");
    }

    // Method to make a string literal
    std::string createStringLiteral(const std::string& str) {
        // Remove quotes if present
        std::string cleanStr = str;
        if (cleanStr.front() == '"' && cleanStr.back() == '"') {
            cleanStr = cleanStr.substr(1, cleanStr.length() - 2);
        }
        
        std::string label = ".LC" + std::to_string(stringLiteralCount++);
        assembly.push_back(label + ":");
        assembly.push_back("\t.ascii \"" + cleanStr + "\\0\"");
        return label;
    }
    
    // Method to make a function header
    void generateFunctionHeader(const std::string& name) {
        assembly.push_back(".text");
        assembly.push_back(".globl " + name);
        assembly.push_back(".def\t" + name + ";\t.scl\t2;\t.type\t32;\t.endef;");
        assembly.push_back(".seh_proc " + name);
        assembly.push_back(name + ":");
        // Function prologue
        assembly.push_back("\tpushq\t%rbp");
        assembly.push_back("\t.seh_pushreg\t%rbp");
        assembly.push_back("\tmovq\t%rsp, %rbp");
        assembly.push_back("\t.seh_setframe\t%rbp, 0");
        assembly.push_back("\tsubq\t$32, %rsp");
        assembly.push_back("\t.seh_stackalloc\t32");
        assembly.push_back("\t.seh_endprologue");
    }

    // Method to make a function footer
    void generateFunctionFooter(const std::string& functionName) {
        if (functionName == "main") {

        }
        // Function epilogue
        assembly.push_back("\taddq\t$32, %rsp");
        assembly.push_back("\tpopq\t%rbp");
        assembly.push_back("\tret");
        assembly.push_back("\t.seh_endproc");
    }

public:
    TargetCodeGen(const std::vector<Quad>& ir) {
        generateAssembly(ir);
    }

    // Main code generation method
    void generateAssembly(const std::vector<Quad>& ir) {
        generateHeader();
        
        // First pass: collect all string literals
        std::unordered_map<std::string, std::string> stringLabels;
        for (const auto& quad : ir) {
            if (quad.operation == "PRINT") {
                if (stringLabels.find(quad.arg1) == stringLabels.end()) {
                    stringLabels[quad.arg1] = createStringLiteral(quad.arg1);
                }
            }
        }

        // Second pass: generate code
        for (const auto& quad : ir) {
            if (quad.operation == "FUNC_BEGIN") {
                generateFunctionHeader(quad.arg1);
            }
            else if (quad.operation == "FUNC_END") {
                generateFunctionFooter(quad.arg1);
            }
            else if (quad.operation == "PRINT") {
                std::string label = stringLabels[quad.arg1];
                assembly.push_back("\tleaq\t" + label + "(%rip), %rcx");
                assembly.push_back("\tcall\t__mingw_printf");
            }
            else if (quad.operation == "RETURN") {
                assembly.push_back("\tmovl\t$" + quad.arg1 + ", %eax");
            }
        }

        // Add compiler identification
        assembly.push_back("\t.ident\t\"GOOF Compiler v1.0\"");
    }

    // Helper method to print assembly code line-by-line
    void printAssembly() const {
        for (const auto& line : assembly) {
            std::cout << line << std::endl;
        }
    }

    // Returns the assembly code vector
    std::vector<std::string> getAssembly() const {
        return assembly;
    }
};