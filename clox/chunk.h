#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

/**
 * Lox opcodes.
 */
typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_LOCAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_METHOD
} OpCode;

/**
 * Lox chunk.
 */
typedef struct {
    /** The number of instructions in this chunk. */
    int count;

    /** The capacity of the chunk. */
    int capacity;

    /** The opcodes stored within this chunk. */
    uint8_t *code;

    /** The lines that each instruction originated within the source. */
    int *lines;

    /** The constants in this chunk. */
    ValueArray constants;
} Chunk;

/**
 * Initializes a new chunk.
 * @param chunk a pointer to the chunk to initialize.
 */
void initChunk(Chunk *chunk);

/**
 * Appends a byte to the end of the given chunk.
 * @param chunk a pointer to the chunk.
 * @param byte the byte to append.
 * @param line the line that the instruction originated within the source.
 */
void writeChunk(Chunk *chunk, uint8_t byte, int line);

/**
 * Frees a chunk's allocated memory.
 * @param chunk a pointer to the chunk.
 */
void freeChunk(Chunk *chunk);

/**
 * Appends a value to a chunk's constants.
 * @param chunk the chunk.
 * @param value the new value.
 * @return the index where the constant was appended.
 */
int addConstant(Chunk *chunk, Value value);

#endif //CLOX_CHUNK_H
