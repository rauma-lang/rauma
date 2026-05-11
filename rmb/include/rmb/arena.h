// RauMa Bootstrap Compiler - Arena Allocator
// Simple bump allocator for temporary memory.

#ifndef RMB_ARENA_H
#define RMB_ARENA_H

#include "common.h"

// Opaque arena structure
typedef struct rmb_arena rmb_arena;

// Create a new arena with initial capacity
// Returns NULL on allocation failure
rmb_arena* rmb_arena_create(rmb_size initial_capacity);

// Allocate memory from arena
// Returns NULL on allocation failure
void* rmb_arena_alloc(rmb_arena* arena, rmb_size size);

// Allocate aligned memory from arena
// Alignment must be power of two
void* rmb_arena_alloc_aligned(rmb_arena* arena, rmb_size size, rmb_size alignment);

// Reset arena (free all allocations, keep buffer)
void rmb_arena_reset(rmb_arena* arena);

// Destroy arena and free all memory
void rmb_arena_destroy(rmb_arena* arena);

// Get current position in arena (for temporary markers)
rmb_size rmb_arena_pos(const rmb_arena* arena);

// Reset arena to previous position
void rmb_arena_restore(rmb_arena* arena, rmb_size pos);

#endif // RMB_ARENA_H
