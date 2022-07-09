#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "memory.h"
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
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
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
    OP_RETURN
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
