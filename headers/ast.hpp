#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declarations
struct ASTNode;
struct ASTVisitor;

struct Statement;
struct PrintStatement;
struct ReturnStatement;
struct Function;
struct Program;

// Base class for all AST nodes
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// AST node visitor method definitions
struct ASTVisitor {
    virtual void visitProgram(Program* node) = 0;
    virtual void visitFunction(Function* node) = 0;
    virtual void visitPrintStatement(PrintStatement* node) = 0;
    virtual void visitReturnStatement(ReturnStatement* node) = 0;
};

// Base class for statement nodes
struct Statement : ASTNode {
    virtual void accept(ASTVisitor& visitor) = 0;
};

struct PrintStatement : Statement {
    std::string message;
    PrintStatement(const std::string msg) {
        this->message = msg;
    }

    void accept(ASTVisitor& visitor) override {
        visitor.visitPrintStatement(this);
    }
};

struct ReturnStatement : Statement {
    int returnValue;
    ReturnStatement(int value) {
        this->returnValue = value;
    }

    void accept(ASTVisitor& visitor) override {
        visitor.visitReturnStatement(this);
    }
};

// Function node class
struct Function : ASTNode {
    std::string name;
    std::vector<std::unique_ptr<Statement>> body;

    Function(const std::string& funcName) {
        this->name = funcName;
    }

    void accept(ASTVisitor& visitor) override {
        visitor.visitFunction(this);
    }
};

// Program node class
struct Program : ASTNode {
    std::vector<std::unique_ptr<Function>> functions;

    void accept(ASTVisitor& visitor) {
        visitor.visitProgram(this);
    }
};
