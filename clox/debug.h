#ifndef CLOX_DEBUG_H
#define CLOX_DEBUG_H

#include "chunk.h"

/**
 * Disassembles a chunk and writes the result to stdout.
 * @param chunk the chunk to disassemble.
 * @param name the name of the chunk.
 */
void disassembleChunk(Chunk *chunk, const char *name);

/**
 * Displays information about a simple instruction, and returns the new offset.
 * @param name the name of the instruction.
 * @param offset the offset for this instruction.
 * @return the offset for the next instruction.
 */
static int simpleInstruction(const char *name, int offset);

/**
 * Disassembles an instruction and writes the result to stdout.
 * @param chunk the chunk containing the instruction.
 * @param offset the instruction offset within the chunk.
 * @return the offset of the next instruction.
 */
int disassembleInstruction(Chunk *chunk, int offset);

#endif //CLOX_DEBUG_H
