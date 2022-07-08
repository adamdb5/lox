#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

/**
 * Allocates an object on the heap.
 * @param type the c value type.
 * @param objectType the Lox object type.
 */
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

/**
 * Allocates an object on the heap.
 * @param size the size of the object.
 * @param type the object type.
 * @return the newly allocated object.
 */
static Obj *allocateObject(size_t size, ObjType type) {
    Obj *object = (Obj*) reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;
    return object;
}

/**
 * Allocates a string on the heap.
 * @param chars the string.
 * @param length the length of the string.
 * @return the newly allocated string.
 */
static ObjString *allocateString(char *chars, int length) {
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}

ObjString *takeString(char *chars, int length) {
    return allocateString(chars, length);
}

ObjString *copyString(const char *chars, int length) {
    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}