#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)   (AS_OBJ(value)->type)

#define IS_STRING(value)  isObjType(value, OBJ_STRING)

#define AS_STRING(value)  ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

/**
 * Lox object types.
 */
typedef enum {
    OBJ_STRING
} ObjType;

/**
 * Lox object.
 */
struct Obj {
    ObjType type;
    struct Obj *next;
};

/**
 * Lox string object.
 */
struct ObjString {
    Obj obj;
    int length;
    char *chars;
};

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
