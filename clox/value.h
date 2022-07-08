#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

/**
 * Lox value types.
 */
typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

/**
 * Lox value struct.
 * This struct holds a type and value.
 */
typedef struct {
    ValueType type;
    union {
        bool boolean;
        Obj *obj;
        double number;
    } as;
} Value;

#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

#define AS_OBJ(value)     ((value).as.obj)
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define BOOL_VAL(value)   ((Value){VAL_BOOL,   {.boolean = value}})
#define NIL_VAL           ((Value){VAL_NIL,    {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)   ((Value){VAL_OBJ,    {.obj = (Obj*)object}})

/**
 * Represents the values associated with the current chunk.
 */
typedef struct {

    /** The capacity of the array. */
    int capacity;

    /** The number of values in the array. */
    int count;

    /**  The array of values. */
    Value *values;
} ValueArray;

/**
 * Compares two values, returning true of the values are equal.
 * @param a the first value.
 * @param b the second value.
 * @return true if both values are equal.
 */
bool valuesEqual(Value a, Value b);

/**
 * Initializes a value array.
 * @param array the value array.
 */
void initValueArray(ValueArray *array);

/**
 * Appends a new value to the given value array.
 * @param array the value array.
 * @param value the new value.
 */
void writeValueArray(ValueArray *array, Value value);

/**
 * Frees the value array's memory.
 * @param array the value array.
 */
void freeValueArray(ValueArray *array);

/**
 * Writes a value to stdout.
 * @param value the value.
 */
void printValue(Value value);

#endif //CLOX_VALUE_H
