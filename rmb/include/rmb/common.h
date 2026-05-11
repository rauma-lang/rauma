// RauMa Bootstrap Compiler - Common Definitions
// This file provides basic typedefs and common definitions.

#ifndef RMB_COMMON_H
#define RMB_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Basic integer types
typedef int64_t rmb_i64;
typedef uint64_t rmb_u64;
typedef int32_t rmb_i32;
typedef uint32_t rmb_u32;
typedef int16_t rmb_i16;
typedef uint16_t rmb_u16;
typedef int8_t rmb_i8;
typedef uint8_t rmb_u8;
typedef uint8_t rmb_byte;

// Floating point types
typedef double rmb_f64;
typedef float rmb_f32;

// Boolean type (already have stdbool.h, but alias for consistency)
typedef bool rmb_bool;

// Size type
typedef size_t rmb_size;

// Result type for functions that can fail
typedef enum {
    RMB_OK = 0,
    RMB_ERROR = 1,
    RMB_IO_ERROR = 2,
    RMB_MEMORY_ERROR = 3,
    RMB_SYNTAX_ERROR = 4,
    RMB_TYPE_ERROR = 5
} rmb_result;

// String view (non-owning)
typedef struct {
    const char* ptr;
    rmb_size len;
} rmb_string_view;

// Null pointer constant
#define RMB_NULL NULL

// Array length helper
#define RMB_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

// Assertion (only in debug builds)
#ifdef NDEBUG
#define RMB_ASSERT(expr) ((void)0)
#else
#include <assert.h>
#define RMB_ASSERT(expr) assert(expr)
#endif

// Unused parameter macro
#define RMB_UNUSED(x) (void)(x)

// Inline hint
#ifdef _MSC_VER
#define RMB_INLINE __inline
#else
#define RMB_INLINE inline
#endif

// Alignment helper
#define RMB_ALIGN_UP(value, alignment) \
    (((value) + (alignment) - 1) & ~((alignment) - 1))

// Minimum/maximum
#define RMB_MIN(a, b) ((a) < (b) ? (a) : (b))
#define RMB_MAX(a, b) ((a) > (b) ? (a) : (b))

// Clamp value
#define RMB_CLAMP(value, min, max) \
    ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#endif // RMB_COMMON_H
