#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value)    (AS_OBJ(value)->type)

#define IS_CLOSURE(value)  isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)   isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)   isObjType(value, OBJ_STRING)

#define AS_CLOSURE(value)  ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)   (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->chars)

/**
 * Lox object types.
 */
typedef enum {
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
} ObjType;

/**
 * Lox object.
 */
struct Obj {
    ObjType type;
    struct Obj *next;
};

/**
 * Lox function.
 */
typedef struct {
    Obj obj;
    int arity;
    int upvalueCount;
    Chunk chunk;
    ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

/**
 * Lox native function.
 */
typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

/**
 * Lox string object.
 */
struct ObjString {
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

/**
 * Upvalue.
 */
typedef struct ObjUpvalue {
    Obj obj;
    Value *location;
    Value closed;
    struct ObjUpvalue *next;
}  ObjUpvalue;

/**
 * Closure.
 */
typedef struct {
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalueCount;
} ObjClosure;

/**
 * Creates a new function.
 * @return the function.
 */
ObjFunction *newFunction();

/**
 * Creates a new native function in the Lox interpreter..
 * @param function the native function.
 * @return the native function.
 */
ObjNative *newNative(NativeFn function);

/**
 * Creates a new closure.
 * @param function the function for the closure.
 * @return the closure.
 */
ObjClosure *newClosure(ObjFunction *function);

/**
 * Allocates a string.
 * @param chars the string.
 * @param length the length of the string.
 * @return the newly allocated string.
 */
ObjString *takeString(char *chars, int length);

/**
 * Copies a string onto the heap.
 * @param chars the string.
 * @param length the length of the string.
 * @return the newly allocated string.
 */
ObjString *copyString(const char *chars, int length);

/**
 * Creates a new upvalue.
 * @param slot the upvalue's slot.
 * @return the upvalue.
 */
ObjUpvalue *newUpvalue(Value *slot);

/**
 * Prints an object to stdout.
 * @param value the object.
 */
void printObject(Value value);

/**
 * Determines whether the value is of the specified type.
 * @param value the value.
 * @param type the object type.
 * @return if the object is of the specified type.
 */
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //CLOX_OBJECT_H
