#ifndef CLOX_TABLE_H
#define CLOX_TABLE_H

#include "common.h"
#include "value.h"

/**
 * Hash table entry.
 */
typedef struct {
    ObjString *key;
    Value value;
} Entry;

/**
 * Hash table.
 */
typedef struct {
    int count;
    int capacity;
    Entry *entries;
} Table;

/**
 * Initialises an empty hash table.
 * @param table the hash table.
 */
void initTable(Table *table);

/**
 * Frees a hash table.
 * @param table the hash table.
 */
void freeTable(Table *table);

/**
 * Gets a value from a hash table.
 * @param table the hash table.
 * @param key the key to find.
 * @param value the value.
 * @return whether the value was found.
 */
bool tableGet(Table *table, ObjString *key, Value *value);

/**
 * Sets a key-value in the hash table.
 * @param table the hash table.
 * @param key the key to add.
 * @param value the value to add.
 * @return if a new value was added.
 */
bool tableSet(Table *table, ObjString *key, Value value);

/**
 * Deletes an entry from a hash table.
 * @param table the table.
 * @param key the key to delete.
 * @return if the entry was deleted.
 */
bool tableDelete(Table *table, ObjString *key);

/**
 * Copies entries from one hash table to another.
 * @param from the origin hash table.
 * @param to the destination hash table.
 */
void tableAddAll(Table *from, Table *to);

/**
 * Finds a string stored within a hash table.
 * @param table the hash table.
 * @param chars the characters of the string.
 * @param length the length of the string.
 * @param hash the hash of the string.
 * @return
 */
ObjString *tableFindString(Table *table, const char *chars, int length, uint32_t hash);

/**
 * Removes white objects (no longer referenced) from the table.
 * @param table the table.
 */
void tableRemoveWhite(Table *table);

/**
 * Marks the objects within the table.
 * @param table the table.
 */
void markTable(Table *table);

#endif //CLOX_TABLE_H
