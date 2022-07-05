

#ifndef CLOX_SCANNER_H
#define CLOX_SCANNER_H

/**
 * Lox token types.
 */
typedef enum {
    /* Single character tokens. */
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    /* One or two character tokens. */
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    /* Literals. */
    TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_STRING,

    /* Keywords. */
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_ERROR, TOKEN_EOF
} TokenType;

/**
 * A Lox token.
 */
typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

/**
 * Creates a scanner for the given source code.
 * @param source the source code to scan.
 */
void initScanner(const char *source);

/**
 * Scans the next token.
 * @return the next token.
 */
Token scanToken();

#endif //CLOX_SCANNER_H
