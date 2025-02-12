#pragma once

#include <string>

enum TokenType { 
    TOKEN_IDENTIFIER,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_INTEGER_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_EOF,
    TOKEN_UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
};