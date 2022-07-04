#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

typedef double Value;

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
