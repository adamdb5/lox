#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include "common.h"

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
 * | non-zero | 0         | Free allocation.    |
 * | non-zero | < oldSize | Shrink allocation.  |
 * | non-zero | > oldSize | Grow allocation.    |
 *
 * @param pointer pointer to the memory.
 * @param oldSize the old memory size.
 * @param newSize the new memory size.
 * @return pointer to the new memory.
 */
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

#endif //CLOX_MEMORY_H
