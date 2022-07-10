#include <stdio.h>

#include "debug.h"
#include "value.h"

/* ===== Static functions ===== */

/**
 * Prints information about a constant.
 * @param name the name of the instruction.
 * @param chunk the chunk that contains this instruction.
 * @param offset the offset of the instruction within the chunk.
 * @return the offset of the next instruction.
 */
static int constantInstruction(const char *name, Chunk *chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

/**
 * Prints information about a simple instruction with no operands.
 * @param name the name of the instruction.
 * @param offset the offset of the instruction within the chunk.
 * @return the offset of the next instruction.
 * @return the offset of the next instruction.
 */
static int simpleInstruction(const char *name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

/**
 * Prints information about a byte instruction.
 * @param name the name of the instruction.
 * @param chunk the chunk that contains this instruction.
 * @param offset the offset of the instruction within the chunk.
 * @return the offset of the next instruction.
 */
static int byteInstruction(const char *name, Chunk *chunk, int offset) {
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

/**
 * Prints information about a jump instruction.
 * @param name the name of the instruction.
 * @param sign whether the jump is positive or negative.
 * @param chunk the chunk that contains this instruction.
 * @param offset the offset of the instruction within the chunk.
 * @return
 */
static int jumpInstruction(const char *name, int sign, Chunk *chunk, int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

/* ===== End static functions ===== */


void disassembleChunk(Chunk *chunk, const char *name) {
    printf("== %s ==\n", name);
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(Chunk *chunk, int offset) {
    printf("%04d ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);
        case OP_GET_LOCAL:
            return constantInstruction("OP_GET_LOCAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL", chunk, offset);
        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_SET_LOCAL:
            byteInstruction("OP_SET_LOCAL", chunk, offset);
        case OP_SET_GLOBAL:
            byteInstruction("OP_SET_GLOBAL", chunk, offset);
        case OP_EQUAL:
            simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:
            simpleInstruction("OP_GREATER", offset);
        case OP_LESS:
            simpleInstruction("OP_LESS", offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_PRINT:
            return simpleInstruction("OP_PRINT", offset);
        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}


