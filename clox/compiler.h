#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "vm.h"

/**
 * Compiles the given source to bytecode and writes it to a chunk.
 * @param source the Lox source code.
 */
ObjFunction *compile(const char *source);

#endif //CLOX_COMPILER_H
