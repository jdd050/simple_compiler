#pragma once

#include <iostream>
#include "ast.hpp"

// Intermediate code structure (mimics assembly)
struct Quad {
    std::string operation;  // Represents the operation ("PRINT", "RETURN", e.g.)
    std::string arg1;       // First argument
    std::string arg2;       // Second argument
    std::string result;     // Result variable (if applicable)

    Quad(const std::string& operation, const std::string& arg1 = "", const std::string arg2 = "", const std::string result = "")
        : operation(operation), arg1(arg1), arg2(arg2), result(result) {}
};

// Generate inermediate code to mimic assembly
class IntermediateCodeGen : public ASTVisitor {
public:
    std::vector<Quad> quads;

    void visitProgram(Program* node) override {
        for (const auto& func : node->functions) {
            func->accept(*this);
        }
    }

    void visitFunction(Function* node) override {
        quads.emplace_back("FUNC_BEGIN", node->name);
        for (const auto& stmt : node->body) {
            stmt->accept(*this);
        }
        quads.emplace_back("FUNC_END", node->name);
    }

    void visitPrintStatement(PrintStatement* node) override {
        quads.emplace_back("PRINT", node->message);
    }

    void visitReturnStatement(ReturnStatement* node) override {
        quads.emplace_back("RETURN", std::to_string(node->returnValue));
    }

    void printIR() const {
        for (const auto& quad : quads) {
            std::cout << "(" << quad.operation << "," << quad.arg1 << "," << quad.arg2 << "," << quad.result << ")" << std::endl;
        }
    }

    std::vector<Quad> getIR() const {
        return quads;
    }
};