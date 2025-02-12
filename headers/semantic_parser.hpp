#pragma once

#include <set>
#include "syntax_parser.hpp"

// Custom error class for semantic analysis
class SemanticError : public std::runtime_error {
public:
    SemanticError(const std::string& message) : std::runtime_error(message) {}
};

// Syntax is correct, check for semantic errors
class SemanticAnalyzer : public ASTVisitor {
private:
    std::set<std::string> definedFunctions;
    bool hasMainFunction = false;
    bool currentFunctionhasReturn = false;
    std::string currentFunction;
    // Allow for multiple errors
    std::vector<std::string> errors;

    // Helper method to format errors found
    std::string formatErrors() {
        std::string result = "Semantic errors found:\n";
        for (const auto& error: errors) {
            result += "- " + error + "\n";
        }
        return result;
    }

public:
    SemanticAnalyzer(Program* program) {
        // Iterate over all nodes in the program AST
        visitProgram(program);

        // After visiting all functions, check if main function exists
        if (!hasMainFunction) {
            errors.push_back("Program must have a 'main' function.");
        }

        // If errors were detected, display them
        if (!errors.empty()) {
            throw SemanticError(formatErrors());
        }
    }

    // Iterates over all nodes in the program AST
    void visitProgram(Program* node) override {
        for (const auto& func : node->functions) {
            // Check for duplicate functions
            if (definedFunctions.count(func->name) > 0) {
                errors.push_back("Function '" + func->name + "' is already defined.");
            }
            definedFunctions.insert(func->name);

            // Check if its the main function
            if (func->name == "main") {
                hasMainFunction = true;
            }
            
            // Go to function node
            visitFunction(func.get());
        }
    }

    // Iterates over function body nodes
    void visitFunction(Function* node) override {
        currentFunction = node->name;
        currentFunctionhasReturn = false;

        // Visit all nodes in function body
        for (const auto& stmt : node->body) {
            stmt->accept(*this);
        }

        // Check if function has return statement
        if (!currentFunctionhasReturn) {
            errors.push_back("Function '" + node->name + "' must return a value.");
        }
    }

    void visitPrintStatement(PrintStatement* node) override {
        // Verify print statement has content
        if (node->message.empty()) {
            errors.push_back("Print statement in function '" + currentFunction + "' cannot be empty");
        }
    }

    void visitReturnStatement(ReturnStatement* node) override {
        currentFunctionhasReturn = true;
        // for now, any integer value is accepted for return value
    }
};
    