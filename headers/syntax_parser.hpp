#pragma once

#include <stdexcept>
#include "ast.hpp"
#include "token.hpp"

// Custom error class for lexer
class LexerError : public std::runtime_error {
public:
    LexerError(const std::string& message) : std::runtime_error(message) {}
};

// Read file input and create tokens
class Lexer {
private:
    std::string source;
    size_t pos = 0;
    int line = 1;
    int column = 1;

public:
    Lexer(const std::string& src) {
        this->source = src;
    }

    // Returns char at current position (from source file)
    char peek() {
        return (pos < source.size()) ? source[pos] : '\0';
    }

    // Returns char at position ahead (from source file)
    char advance() {
        if (pos < source.size()) {
            char current = source[pos++];
            if (current == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            return current;
        }
        return '\0';
    }

    // Helper method to skip whitespace
    void skipWhitespace() {
        while (isspace(peek())) {
            advance();
        }
    }

    // Identify next token
    Token nextToken() {
        skipWhitespace();

        // End of file
        if (pos >= source.size()) return {TOKEN_EOF, ""};
        
        // Go to next char
        char ch = advance();
        
        // Check: integer literal
        if (isdigit(ch)) {
            std::string number(1, ch);
            while (isdigit(peek())) {
                number += advance();
            }
            return {TOKEN_INTEGER_LITERAL, number};
        }

        // Check: identifier
        if (isalpha(ch)) {
            std::string ident(1, ch);
            while (isalnum(peek()) || peek() == '_') {
                ident += advance();
            }

            // RESERVED KEYWORDS
            if (ident == "return") return {TOKEN_RETURN, ident};
            if (ident == "print") return {TOKEN_PRINT, ident};

            return {TOKEN_IDENTIFIER, ident};
        }

        // Check: string literals
        if (ch == '"') {
            std::string str_literal;
            int start_line = line;
            int start_column = column;
            while (peek() != '"' && peek() != '\0') {
                // Catch unterminated strings spanning lines
                if (peek() == '\n') {
                    throw LexerError("Error: Unterminated string literal starting at line " +
                        std::to_string(start_line) + ", column " + std::to_string(start_column));
                }
                str_literal += advance();
            }
            if (peek() == '"') {
                // Consume closing quote
                advance();
            } else {
                throw LexerError("Error: Unterminated string literal starting at line " +
                    std::to_string(start_line) + ", column " + std::to_string(start_column));
            }
            return {TOKEN_STRING_LITERAL, str_literal};
        }

        // Check: misc chars
        if (ch == '{') return {TOKEN_LBRACE, "{"};
        if (ch == '}') return {TOKEN_RBRACE, "}"};

        // Token is not valid
        return {TOKEN_UNKNOWN, std::string (1, ch)};
    }
};

// Custom error class for parser
class SyntaxError : public std::runtime_error {
public:
    SyntaxError(const std::string& message) : std::runtime_error(message) {}
};

// Interpret tokens (syntax analysis)
class Parser {
private:
    Lexer lexer;
    Token currentToken;

    // Goes to next token
    void advance() {
        currentToken = lexer.nextToken();
    }

    // Checks current token type then continues
    void expect(TokenType type, const std::string& errorMsg) {
        if (currentToken.type != type) {
            std::string detailedError = errorMsg + " Found: '" + currentToken.value + "'";
            if (currentToken.type == TOKEN_EOF) {
                detailedError += " (Unexpected end of file)";
            }
            throw SyntaxError(detailedError);
        }
        advance();
    }

public:
    Parser(const std::string& source) : lexer(source) {
        advance(); // Initialize first token
    }

    // Makes pointer to whole program
    std::unique_ptr<Program> parseProgram() {
        auto program = std::make_unique<Program>();
        while (currentToken.type != TOKEN_EOF) {
            program->functions.push_back(parseFunction());
        }
        return program;
    }

    // Makes pointer to a specific function
    std::unique_ptr<Function> parseFunction() {
        if (currentToken.type != TOKEN_IDENTIFIER) {
            throw SyntaxError("Expected function name. Found: '" + currentToken.value + "'");
        }
        std::string funcName = currentToken.value;
        advance();
        
        // Check for opening brace after func identifier
        expect(TOKEN_LBRACE, "Expected '{' after function definition");
        auto func = std::make_unique<Function>(funcName);
        
        // Read function body
        while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
            // Check if we're encountering what looks like a new function declaration unexpectedly
            if (currentToken.type == TOKEN_IDENTIFIER) {
                throw SyntaxError(
                    "Error: Missing closing '}' for function '" + funcName +
                    "'. Found possible new function declaration: '" + currentToken.value + "'"
                );
            }
            func->body.push_back(parseStatement());
        }
        
        // Make sure function domain is closed
        if (currentToken.type == TOKEN_EOF) {
            throw SyntaxError("Error: Unexpected end of file. Missing '}' to close function '" + funcName + "'");
        }
    
        expect(TOKEN_RBRACE, "Expected '}' at the end of the function definition");
        return func;
    }

    // Makes pointer to a keyword
    std::unique_ptr<Statement> parseStatement() {
        if (currentToken.type == TOKEN_PRINT) {
            advance();
            Token stringToken = currentToken;
            expect(TOKEN_STRING_LITERAL, "Expected string literal after 'print'");
            return std::make_unique<PrintStatement>(stringToken.value);
        }
        else if (currentToken.type == TOKEN_RETURN) {
            advance();
            if (currentToken.type != TOKEN_INTEGER_LITERAL) {
                throw SyntaxError("Expected integer literal after 'return'");
            }
            int returnValue = std::stoi(currentToken.value);
            advance();
            return std::make_unique<ReturnStatement>(returnValue);
        }
        else {
            throw SyntaxError("Unknown statement starting with: " + currentToken.value);
        }
    }
};
    