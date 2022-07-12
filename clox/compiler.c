#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "scanner.h"
#include "value.h"
#include "object.h"

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
typedef void (*ParseFn)(bool canAssign);

/**
 * Represents a parse rule for a specific precedence.
 */
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

/**
 * Represents a local variable.
 */
typedef struct {
    Token name;
    int depth;
    bool isCaptured;
} Local;

/**
 * Represents an upvalue.
 */
typedef struct {
    uint8_t index;
    bool isLocal;
} Upvalue;

/**
 * Function types.
 */
typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT
} FunctionType;

/**
 * Represents the compiler's scope state.
 */
struct Compiler {
    struct Compiler *enclosing;
    ObjFunction *function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    Upvalue upvalues[UINT8_COUNT];
    int scopeDepth;
};
typedef struct Compiler Compiler;

Parser parser;
Compiler *current = NULL;
Chunk *compilingChunk;

/* ===== Static functions ===== */

/**
 * Gets the current chunk.
 * @return the current chunk.
 */
static Chunk *currentChunk() {
    return &current->function->chunk;
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
 * Checks if the current token is of the given type.
 * @param type the token type.
 * @return if the current token is of the given type.
 */
static bool check(TokenType type) {
    return parser.current.type == type;
}

/**
 * Checks if the current token matches the given type.
 * @param type the token type.
 * @return if the current token is of the given type.
 */
static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
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
 * Emits a loop instruction.
 * @param loopStart the start index of the loop.
 */
static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

/**
 * Emits a jump instruction.
 * @param instruction the jump instruction.
 * @return the index of the jump instruction.
 */
static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

/**
 * Writes the return opcode to the chunk.
 */
static void emitReturn() {
    emitByte(OP_NIL);
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

static void patchJump(int offset) {
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

/**
 * Initialise the compiler.
 * @param compiler the compiler.
 */
static void initCompiler(Compiler *compiler, FunctionType type) {
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;
    if (type != TYPE_SCRIPT) {
        current->function->name = copyString(parser.previous.start, parser.previous.length);
    }

    Local *local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;
    local->name.start = "";
    local->name.length = 0;
}

/**
 * Terminates the compiler.
 */
static ObjFunction *endCompiler() {
    emitReturn();
    ObjFunction *function = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
    }
#endif

    current = current->enclosing;
    return function;
}

/**
 * Begin a new scope.
 */
static void beginScope() {
    current->scopeDepth++;
}

/**
 * End a scope.
 */
static void endScope() {
    current->scopeDepth--;

    while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
        emitByte(OP_POP);

        if (current->locals[current->localCount - 1].isCaptured) {
            emitByte(OP_CLOSE_UPVALUE);
        } else {
            emitByte(OP_POP);
        };

        current->localCount--;
    }
}

/**
 * Gets the next expression.
 */
static void expression();

/**
 * Gets the next statement.
 */
static void statement();

/**
 * Gets the next declaration.
 */
static void declaration();

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
 * Gets the next identifier constant.
 * @param name the name of the constant.
 * @return the index of the new constant.
 */
static uint8_t identifierConstant(Token *name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

/**
 * Determines whether two identifiers are equal.
 * @param a the first token.
 * @param b the second token.
 * @return if the two identifiers are equal.
 */
static bool identifiersEqual(Token *a, Token *b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

/**
 * Resolves a local variable.
 * @param compiler the compiler.
 * @param name the variable's name.
 * @return the index of the variable, or -1 if it does not exist.
 */
static int resolveLocal(Compiler *compiler, Token *name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

/**
 * Adds an upvalue.
 * @param compiler the compiler.
 * @param index the index of the upvalue.
 * @param isLocal whether the upvalue is local.
 * @return the number of upvalues.
 */
static int addUpvalue(Compiler *compiler, uint8_t index, bool isLocal) {
    int upvalueCount = compiler->function->upvalueCount;
    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;

    for (int i = 0; i < upvalueCount; i++) {
        Upvalue *upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }

    if (upvalueCount == UINT8_COUNT) {
        error("Too many closure variables in function.");
        return 0;
    }

    return compiler->function->upvalueCount++;
}

/**
 * Resolves an upvalue.
 * @param compiler the compiler.
 * @param name the name of the upvalue.
 * @return the number of upvalues.
 */
static int resolveUpvalue(Compiler *compiler, Token *name) {
    if (compiler->enclosing == NULL) return -1;

    int local = resolveLocal(compiler->enclosing, name);
    if (local != -1) {
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, (uint8_t)local, true);
    }

    int upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return addUpvalue(compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

/**
 * Adds a bew variable to the scope.
 * @param name the variable name.
 */
static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }

    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
    local->isCaptured = false;
}

/**
 * Declares a new variavle on the scope.
 */
static void declareVariable() {
    if (current->scopeDepth == 0) return;

    Token *name = &parser.previous;

    for (int i = current->localCount - 1; i >= 0; i--) {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(*name);
}

/**
 * Parses the current variable.
 * @param errorMessage the error message to display if the next token is not an identifier.
 * @return the index of the new variable.
 */
static uint8_t parseVariable(const char *errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0) return 0;

    return identifierConstant(&parser.previous);
}

/**
 * Marks a variable as initialised.
 */
static void markInitialised() {
    if (current->scopeDepth == 0) return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

/**
 * Defines a variable.
 * @param global the VM's global variables
 */
static void defineVariable(uint8_t global) {
    if (current->scopeDepth > 0) {
        markInitialised();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

/**
 * Reads the argument list of a function definition.
 * @return the number of arguments.
 */
static uint8_t argumentList() {
    uint8_t argCount = 0;
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            if (argCount == 255) {
                error("Can't have more than 255 arguments.");
            }
            argCount++;
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

/**
 * Compiles a logical and.
 * @param canAssign
 */
static void and_(bool canAssign) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

/**
 * Compiles a logical or.
 * @param canAssign
 */
static void or_(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

/**
 * Compiles a binary expression into bytecode.
 */
static void binary(bool canAssign) {
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
 * Compiles a function call.
 * @param canAssign
 */
static void call(bool canAssign) {
    uint8_t argCount = argumentList();
    emitBytes(OP_CALL, argCount);
}

/**
 * Compiles a literal into bytecode.
 */
static void literal(bool canAssign) {
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
 * Enters a new block.
 */
static void block() {
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/**
 * Compiles a function.
 * @param type the type of the function.
 */
static void function(FunctionType type) {
    Compiler compiler;
    initCompiler(&compiler, type);
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                errorAtCurrent("Can't have more than 255 parameters.");
            }
            uint8_t constant = parseVariable("Expect parameter name.");
            defineVariable(constant);
        } while (match(TOKEN_COMMA));
    }


    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    ObjFunction *function = endCompiler();
    emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++) {
        emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(compiler.upvalues[i].index);
    }
}

static void funDeclaration() {
    uint8_t global = parseVariable("Expect function name.");
    markInitialised();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

/* Forward declared. */
static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

/* Forward declared. */
static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

/**
 * Compiles a for statement.
 */
static void forStatement() {
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    if (match(TOKEN_SEMICOLON)) {

    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;
    int exitJump = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }

    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' for after clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP);
    }

    endScope();
}

/**
 * Compiles an if statement.
 */
static void ifStatement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);

    patchJump(thenJump);

    if (match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
    emitByte(OP_POP);
}

/**
 * Compiles a print statement.
 */
static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

/**
 * Compiles a return statement.
 */
static void returnStatement() {
    if (current->type == TYPE_SCRIPT) {
        error("Can't return from top-level code.");
    }

    if (match(TOKEN_SEMICOLON)) {
        emitReturn();
    } else {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }
}

/**
 * Compiles a while statement.
 */
static void whileStatement() {
    int loopStart = currentChunk()->count;
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

/**
 * Synchronizes the VM.
 */
static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                ;
        }

        advance();
    }
}

/**
 * Gets the next declaration.
 */
static void declaration() {
    if (match(TOKEN_FUN)) {
        funDeclaration();
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

/**
 * Gets the next statement.
 */
static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_RETURN)) {
        returnStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

/**
 * Compiles a grouping into bytecode.
 */
static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

/**
 * Compiles a number into bytecode.
 */
static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

/**
 * Gets the next string and copies it onto the heap.
 */
static void string(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

/**
 * Gets / creates a named variable.
 * @param name the name of the variable.
 * @param canAssign whether the variable can be assigned to.
 */
static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);

    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ((arg = resolveUpvalue(current, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    } else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

/**
 * Gets the next named variable.
 * @param canAssign  whether the variable can be assigned to.
 */
static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

/**
 * Compiles a unary unary expression into bytecode.
 */
static void unary(bool canAssign) {
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
        [TOKEN_LEFT_PAREN]     = {grouping, call,   PREC_CALL},
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
        [TOKEN_IDENTIFIER]     = {variable, NULL,   PREC_NONE},
        [TOKEN_STRING]         = {string,   NULL,   PREC_NONE},
        [TOKEN_NUMBER]         = {number,   NULL,   PREC_NONE},
        [TOKEN_AND]            = {NULL,     and_,   PREC_AND},
        [TOKEN_CLASS]          = {NULL,     NULL,   PREC_NONE},
        [TOKEN_ELSE]           = {NULL,     NULL,   PREC_NONE},
        [TOKEN_FALSE]          = {literal,  NULL,   PREC_NONE},
        [TOKEN_FOR]            = {NULL,     NULL,   PREC_NONE},
        [TOKEN_FUN]            = {NULL,     NULL,   PREC_NONE},
        [TOKEN_IF]             = {NULL,     NULL,   PREC_NONE},
        [TOKEN_NIL]            = {literal,  NULL,   PREC_NONE},
        [TOKEN_OR]             = {NULL,     or_,    PREC_OR},
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

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

/* Forward declared. */
static ParseRule *getRule(TokenType type) {
    return &rules[type];
}


/* ===== End static functions ===== */

ObjFunction *compile(const char *source) {
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    ObjFunction *function = endCompiler();

    return parser.hadError ? NULL : function;
}