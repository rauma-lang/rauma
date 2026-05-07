// RauMa Bootstrap Compiler - Arena Allocator Implementation

#include <stdlib.h>
#include <string.h>

#include "rmb/arena.h"

// Arena block structure
struct rmb_arena {
    rmb_byte* memory;      // Allocated memory
    rmb_size capacity;     // Total capacity
    rmb_size used;         // Bytes used
};

// Default alignment (cache line friendly)
#define RMB_DEFAULT_ALIGNMENT 64

// Create a new arena
rmb_arena* rmb_arena_create(rmb_size initial_capacity) {
    // Minimum capacity
    if (initial_capacity < 1024) {
        initial_capacity = 1024;
    }

    // Allocate arena structure
    rmb_arena* arena = malloc(sizeof(rmb_arena));
    if (!arena) {
        return NULL;
    }

    // Allocate memory
    arena->memory = malloc(initial_capacity);
    if (!arena->memory) {
        free(arena);
        return NULL;
    }

    arena->capacity = initial_capacity;
    arena->used = 0;

    return arena;
}

// Allocate memory from arena
void* rmb_arena_alloc(rmb_arena* arena, rmb_size size) {
    return rmb_arena_alloc_aligned(arena, size, 1);
}

// Allocate aligned memory from arena
void* rmb_arena_alloc_aligned(rmb_arena* arena, rmb_size size, rmb_size alignment) {
    if (!arena || size == 0) {
        return NULL;
    }

    // Alignment must be power of two
    if ((alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    // Align current position
    rmb_size aligned_used = RMB_ALIGN_UP(arena->used, alignment);

    // Check if we need more space
    if (aligned_used + size > arena->capacity) {
        // Calculate new capacity (double or enough for allocation)
        rmb_size new_capacity = arena->capacity * 2;
        rmb_size needed = aligned_used + size;

        if (new_capacity < needed) {
            new_capacity = RMB_ALIGN_UP(needed, 1024);
        }

        // Reallocate memory
        rmb_byte* new_memory = realloc(arena->memory, new_capacity);
        if (!new_memory) {
            return NULL;
        }

        arena->memory = new_memory;
        arena->capacity = new_capacity;
    }

    // Get pointer to aligned memory
    void* ptr = arena->memory + aligned_used;
    arena->used = aligned_used + size;

    // Zero memory for safety
    memset(ptr, 0, size);

    return ptr;
}

// Reset arena
void rmb_arena_reset(rmb_arena* arena) {
    if (arena) {
        arena->used = 0;
    }
}

// Destroy arena
void rmb_arena_destroy(rmb_arena* arena) {
    if (arena) {
        free(arena->memory);
        free(arena);
    }
}

// Get current position
rmb_size rmb_arena_pos(const rmb_arena* arena) {
    return arena ? arena->used : 0;
}

// Reset arena to previous position
void rmb_arena_restore(rmb_arena* arena, rmb_size pos) {
    if (arena && pos <= arena->used) {
        arena->used = pos;
    }
}