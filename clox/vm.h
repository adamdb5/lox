#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

/**
 * The state of the VM.
 */
typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;
} VM;

/**
 * Interpreter results.
 */
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

/**
 * Initialises the virtual machine.
 */
void initVM();

/**
 * Frees the virtual machine.
 */
void freeVM();

/**
 * Interpret the given chunk and return the status.
 * @param chunk the chunk to interpret.
 * @return the status.
 */
InterpretResult interpret(Chunk *chunk);

/**
 * Pushses a new value onto the stack.
 * @param value the value to push onto the stack.
 */
void push(Value value);

/**
 * Pops a value from the stack.
 * @return the value from the stack.
 */
Value pop();

#endif //CLOX_VM_H
