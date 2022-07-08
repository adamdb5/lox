#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

/**
 * Lox parser.
 */
typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

/**
 * Lox precedence levels.
 */
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} Precedence;

/**
 * Generic function for other parse functions.
 */
typedef void (*ParseFn)();

/**
 * Represents a parse rule for a specific precedence.
 */
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk *compilingChunk;

/* ===== Static functions ===== */

/**
 * Gets the current chunk.
 * @return the current chunk.
 */
static Chunk *currentChunk() {
    return compilingChunk;
}

/**
 * Displays an error message for the specified token.
 * This function also sets the hadError flag.
 * @param token the token causing the error.
 * @param message the error message to display.
 */
static void errorAt(Token *token, const char *message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

/**
 * Reports an error with the given message.
 * @param message the error message.
 */
static void error(const char *message) {
    errorAt(&parser.previous, message);
}

/**
 * Reports an error at the current token.
 * @param message the error message.
 */
static void errorAtCurrent(const char *message) {
    errorAt(&parser.current, message);
}

/**
 * Advances the parser and scans the next token.
 */
static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

/**
 * Consumes the next token of the given type. If the next token
 * is not of the expected type, an error is reported.
 * @param type the expected token type.
 * @param message the error message to display.
 */
static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

/**
 * Writes a byte to the chunk.
 * @param byte the byte to write.
 */
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

/**
 * Writes two bytes to the chunk.
 * @param byte1 the first byte.
 * @param byte2 the second byte.
 */
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

/**
 * Writes the return opcode to the chunk.
 */
static void emitReturn() {
    emitByte(OP_RETURN);
}

/**
 * Creates a new constant in the chunk, and returns the constant's index.
 * @param value the constant's value.
 * @return the index of the constant in the chunk.
 */
static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

/**
 * Writes a new constant to the chunk.
 * @param value the value of the constant.
 */
static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

/**
 * Terminates the compiler.
 */
static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

/**
 * Gets the next expression.
 */
static void expression();

/**
 * Gets the parse rule for the given type.
 * @param type the token type.
 * @return the parse rule.
 */
static ParseRule *getRule(TokenType type);

/**
 * Identifies the token's precedence and executes the relevant prefix and infix functions.
 * @param precedence the token's precedence.
 */
static void parsePrecedence(Precedence precedence);

/**
 * Compiles a binary expression into bytecode.
 */
static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        default: return;
    }
}

/**
 * Compiles a literal into bytecode.
 */
static void literal() {
    switch (parser.previous.type) {
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NIL:   emitByte(OP_NIL); break;
        case TOKEN_TRUE:  emitByte(OP_TRUE); break;
        default: return;
    }
}

/* Forward declared. */
static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

/**
 * Compiles a grouping into bytecode.
 */
static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

/**
 * Compiles a number into bytecode.
 */
static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}


/**
 * Compiles a unary unary expression into bytecode.
 */
static void unary() {
    TokenType operatorType = parser.previous.type;

    parsePrecedence(PREC_UNARY);

    switch (operatorType) {
        TOKEN_BANG:       emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }
}

/**
 * The map of parse rules for each token type.
 */
ParseRule rules[] = {
        [TOKEN_LEFT_PAREN]     = {grouping, NULL,   PREC_NONE},
        [TOKEN_RIGHT_PAREN]    = {NULL,     NULL,   PREC_NONE},
        [TOKEN_LEFT_BRACE]     = {NULL,     NULL,   PREC_NONE},
        [TOKEN_RIGHT_BRACE]    = {NULL,     NULL,   PREC_NONE},
        [TOKEN_COMMA]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_DOT]            = {NULL,     NULL,   PREC_NONE},
        [TOKEN_MINUS]          = {unary,    binary, PREC_TERM},
        [TOKEN_PLUS]           = {NULL,     binary, PREC_TERM},
        [TOKEN_SEMICOLON]      = {NULL,     NULL,   PREC_NONE},
        [TOKEN_SLASH]          = {NULL,     binary, PREC_FACTOR},
        [TOKEN_STAR]           = {NULL,     binary, PREC_FACTOR},
        [TOKEN_BANG]           = {unary,    NULL,   PREC_NONE},
        [TOKEN_BANG_EQUAL]     = {NULL,     binary, PREC_EQUALITY},
        [TOKEN_EQUAL]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_EQUAL_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
        [TOKEN_GREATER]        = {NULL,     binary, PREC_COMPARISON},
        [TOKEN_GREATER_EQUAL]  = {NULL,     binary, PREC_COMPARISON},
        [TOKEN_LESS]           = {NULL,     binary, PREC_COMPARISON},
        [TOKEN_LESS_EQUAL]     = {NULL,     binary, PREC_COMPARISON},
        [TOKEN_IDENTIFIER]     = {NULL,     NULL,   PREC_NONE},
        [TOKEN_STRING]         = {NULL,     NULL,   PREC_NONE},
        [TOKEN_NUMBER]         = {number,   NULL,   PREC_NONE},
        [TOKEN_AND]            = {NULL,     NULL,   PREC_NONE},
        [TOKEN_CLASS]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_ELSE]           = {NULL,     NULL,   PREC_NONE},
        [TOKEN_FALSE]          = {literal,  NULL,   PREC_NONE},
        [TOKEN_FOR]            = {NULL,     NULL,   PREC_NONE},
        [TOKEN_FUN]            = {NULL,     NULL,   PREC_NONE},
        [TOKEN_IF]             = {NULL,     NULL,   PREC_NONE},
        [TOKEN_NIL]            = {literal,  NULL,   PREC_NONE},
        [TOKEN_OR]             = {NULL,     NULL,   PREC_NONE},
        [TOKEN_PRINT]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_RETURN]         = {NULL,     NULL,   PREC_NONE},
        [TOKEN_SUPER]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_THIS]           = {NULL,     NULL,   PREC_NONE},
        [TOKEN_TRUE]           = {literal,  NULL,   PREC_NONE},
        [TOKEN_VAR]            = {NULL,     NULL,   PREC_NONE},
        [TOKEN_WHILE]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_ERROR]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_EOF]            = {NULL,     NULL,   PREC_NONE},
};

/* Forward declared. */
static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

/* Forward declared. */
static ParseRule *getRule(TokenType type) {
    return &rules[type];
}


/* ===== End static functions ===== */

bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}