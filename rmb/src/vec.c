// RauMa Bootstrap Compiler - Dynamic Array Implementation

#include <stdlib.h>
#include <string.h>

#include "rmb/vec.h"

// Default initial capacity
#define RMB_VEC_DEFAULT_CAPACITY 16

// Initialize empty vector
void rmb_vec_init(rmb_vec* vec) {
    if (!vec) {
        return;
    }

    vec->items = NULL;
    vec->len = 0;
    vec->cap = 0;
}

// Initialize vector with initial capacity
bool rmb_vec_init_with_capacity(rmb_vec* vec, rmb_size initial_capacity) {
    if (!vec) {
        return false;
    }

    if (initial_capacity == 0) {
        rmb_vec_init(vec);
        return true;
    }

    vec->items = malloc(initial_capacity * sizeof(void*));
    if (!vec->items) {
        rmb_vec_init(vec);
        return false;
    }

    vec->len = 0;
    vec->cap = initial_capacity;

    return true;
}

// Free vector memory
void rmb_vec_free(rmb_vec* vec) {
    if (vec && vec->items) {
        free(vec->items);
        vec->items = NULL;
        vec->len = 0;
        vec->cap = 0;
    }
}

// Ensure capacity for at least n items
static bool rmb_vec_ensure_capacity(rmb_vec* vec, rmb_size needed) {
    if (vec->cap >= needed) {
        return true;
    }

    // Calculate new capacity (double or needed)
    rmb_size new_cap = vec->cap == 0 ? RMB_VEC_DEFAULT_CAPACITY : vec->cap * 2;
    if (new_cap < needed) {
        new_cap = needed;
    }

    // Reallocate
    void** new_items = realloc(vec->items, new_cap * sizeof(void*));
    if (!new_items) {
        return false;
    }

    vec->items = new_items;
    vec->cap = new_cap;

    return true;
}

// Push item to end
bool rmb_vec_push(rmb_vec* vec, void* item) {
    if (!vec) {
        return false;
    }

    if (!rmb_vec_ensure_capacity(vec, vec->len + 1)) {
        return false;
    }

    vec->items[vec->len] = item;
    vec->len++;

    return true;
}

// Pop item from end
void* rmb_vec_pop(rmb_vec* vec) {
    if (!vec || vec->len == 0) {
        return NULL;
    }

    vec->len--;
    return vec->items[vec->len];
}

// Reserve capacity
bool rmb_vec_reserve(rmb_vec* vec, rmb_size additional) {
    if (!vec) {
        return false;
    }

    rmb_size needed = vec->len + additional;
    if (needed <= vec->cap) {
        return true;
    }

    return rmb_vec_ensure_capacity(vec, needed);
}

// Shrink to fit
bool rmb_vec_shrink_to_fit(rmb_vec* vec) {
    if (!vec || vec->len == vec->cap) {
        return true;
    }

    if (vec->len == 0) {
        free(vec->items);
        vec->items = NULL;
        vec->cap = 0;
        return true;
    }

    void** new_items = realloc(vec->items, vec->len * sizeof(void*));
    if (!new_items) {
        return false;
    }

    vec->items = new_items;
    vec->cap = vec->len;

    return true;
}

// Remove item at index
void* rmb_vec_remove(rmb_vec* vec, rmb_size index) {
    if (!vec || index >= vec->len) {
        return NULL;
    }

    void* item = vec->items[index];

    // Shift items left
    for (rmb_size i = index + 1; i < vec->len; i++) {
        vec->items[i - 1] = vec->items[i];
    }

    vec->len--;

    return item;
}

// Insert item at index
bool rmb_vec_insert(rmb_vec* vec, rmb_size index, void* item) {
    if (!vec || index > vec->len) {
        return false;
    }

    if (!rmb_vec_ensure_capacity(vec, vec->len + 1)) {
        return false;
    }

    // Shift items right
    for (rmb_size i = vec->len; i > index; i--) {
        vec->items[i] = vec->items[i - 1];
    }

    vec->items[index] = item;
    vec->len++;

    return true;
}