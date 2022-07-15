#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include <string.h>

#include "common.h"
#include "value.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN     ((uint64_t)0x7ffc000000000000)

#define TAG_NIL   1
#define TAG_FALSE 2
#define TAG_TRUE  3

typedef uint64_t Value;

#define IS_NIL(value) ((value) == NIL_VAL)

#define IS_BOOL(value)   (((value) | 1) == TRUE_VAL)
#define IS_NUMBER(value) (((value) & QNAN) != QNAN)
#define IS_OBJ(value) \
    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value)   ((value) == TRUE_VAL)
#define AS_NUMBER(value) valueToNum(value)
#define AS_OBJ(value) \
    ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(b)     ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL       ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL        ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL         ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(num) numToValue(num)
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double valueToNum(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value numToValue(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else

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

#endif

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
