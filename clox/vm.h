#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "object.h"
#include "table.h"
#include "value.h"
#include "chunk.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

/**
 * The state of the VM.
 */
typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value *stackTop;
    Table globals;
    Table strings;
    ObjUpvalue *openUpvalues;
    Obj *objects;
} VM;

/**
 * Interpreter results.
 */
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

/* The vm. */
extern VM vm;

/**
 * Initialises the virtual machine.
 */
void initVM();

/**
 * Frees the virtual machine.
 */
void freeVM();

/**
 * Interpret the given source and return the status.
 * @param source the source code to interpret.
 * @return the status.
 */
InterpretResult interpret(const char *source);

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
