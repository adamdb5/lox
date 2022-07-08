#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include "common.h"
#include "object.h"

/**
 * Allocates a number of values on the heap.
 * @param type the value type.
 * @param count the number of values to allocate.
 */
#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

/**
 * Frees a value.
 * @param type the value type.
 * @param pointer the pointer to the value to free.
 */
#define FREE(type, pointer) \
    reallocate(pointer, sizeof(type), 0)

/**
 * If the capacity is zero, the new capacity will be 8.
 * Otherwise, the capacity will be doubled.
 * @param capacity The new capacity for the allocation.
 */
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)


/**
 * Grows an array to the specified size.
 * @param type the datatype of the array.
 * @param pointer the pointer to the array.
 * @param oldCount the previous number of elements in the array.
 * @param newCount the new number of elements in the array.
 */
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

/**
 * Free's an array.
 * @param type the datatype of the array.
 * @param pointer the pointer to the array.
 * @param oldCount the number of elements in the array.
 */
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * Reallocates or frees a allocation of memory.
 *
 * |oldSize   | newSize   | Operation           |
 * |----------|-----------|---------------------|
 * | zero     | non-zero  | Allocate new block. |
 * | non-zero | zero      | Free allocation.    |
 * | non-zero | < oldSize | Shrink allocation.  |
 * | non-zero | > oldSize | Grow allocation.    |
 *
 * @param pointer pointer to the memory.
 * @param oldSize the old memory size.
 * @param newSize the new memory size.
 * @return pointer to the new memory.
 */
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

/**
 * Frees the objects stored on the heap in the VM.
 */
void freeObjects();

#endif //CLOX_MEMORY_H
