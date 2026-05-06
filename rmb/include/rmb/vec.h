// RauMa Bootstrap Compiler - Dynamic Array
// Simple dynamic array (vector) implementation.

#ifndef RMB_VEC_H
#define RMB_VEC_H

#include "common.h"

// Dynamic array structure
typedef struct {
    void** items;     // Array of pointers
    rmb_size len;     // Number of items
    rmb_size cap;     // Allocated capacity
} rmb_vec;

// Initialize empty vector
void rmb_vec_init(rmb_vec* vec);

// Initialize vector with initial capacity
bool rmb_vec_init_with_capacity(rmb_vec* vec, rmb_size initial_capacity);

// Free vector memory (doesn't free items)
void rmb_vec_free(rmb_vec* vec);

// Push item to end of vector
// Returns false on allocation failure
bool rmb_vec_push(rmb_vec* vec, void* item);

// Pop item from end of vector
// Returns NULL if vector is empty
void* rmb_vec_pop(rmb_vec* vec);

// Get item at index (without bounds checking)
static RMB_INLINE void* rmb_vec_get(const rmb_vec* vec, rmb_size index) {
    RMB_ASSERT(index < vec->len);
    return vec->items[index];
}

// Set item at index (without bounds checking)
static RMB_INLINE void rmb_vec_set(rmb_vec* vec, rmb_size index, void* item) {
    RMB_ASSERT(index < vec->len);
    vec->items[index] = item;
}

// Get vector length
static RMB_INLINE rmb_size rmb_vec_len(const rmb_vec* vec) {
    return vec->len;
}

// Check if vector is empty
static RMB_INLINE bool rmb_vec_is_empty(const rmb_vec* vec) {
    return vec->len == 0;
}

// Clear vector (set length to 0, doesn't free items)
static RMB_INLINE void rmb_vec_clear(rmb_vec* vec) {
    vec->len = 0;
}

// Reserve capacity for at least n more items
// Returns false on allocation failure
bool rmb_vec_reserve(rmb_vec* vec, rmb_size additional);

// Shrink vector to fit current length
bool rmb_vec_shrink_to_fit(rmb_vec* vec);

// Remove item at index, shifting later items left
void* rmb_vec_remove(rmb_vec* vec, rmb_size index);

// Insert item at index, shifting later items right
// Returns false on allocation failure
bool rmb_vec_insert(rmb_vec* vec, rmb_size index, void* item);

// Swap two items
static RMB_INLINE void rmb_vec_swap(rmb_vec* vec, rmb_size i, rmb_size j) {
    RMB_ASSERT(i < vec->len && j < vec->len);
    void* temp = vec->items[i];
    vec->items[i] = vec->items[j];
    vec->items[j] = temp;
}

// Typed vector helpers
#define RMB_VEC_TYPE(name, type) \
    typedef struct { \
        type* items; \
        rmb_size len; \
        rmb_size cap; \
    } name

// Initialize typed vector
#define RMB_VEC_INIT(vec) \
    do { \
        (vec)->items = NULL; \
        (vec)->len = 0; \
        (vec)->cap = 0; \
    } while (0)

// Push to typed vector
#define RMB_VEC_PUSH(vec, item) \
    do { \
        if ((vec)->len >= (vec)->cap) { \
            rmb_size new_cap = (vec)->cap == 0 ? 4 : (vec)->cap * 2; \
            type* new_items = realloc((vec)->items, new_cap * sizeof(type)); \
            if (!new_items) return false; \
            (vec)->items = new_items; \
            (vec)->cap = new_cap; \
        } \
        (vec)->items[(vec)->len++] = (item); \
    } while (0)

#endif // RMB_VEC_H