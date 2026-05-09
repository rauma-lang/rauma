// RauMa Bootstrap Compiler - Type Representation
// v0.0.5: Basic type checking

#ifndef RMB_TYPE_H
#define RMB_TYPE_H

#include "common.h"
#include "string.h"
#include "arena.h"

typedef enum RmbTypeKind {
    RMB_TYPE_UNKNOWN,
    RMB_TYPE_NEVER,
    RMB_TYPE_VOID,
    RMB_TYPE_INT,
    RMB_TYPE_UINT,
    RMB_TYPE_FLOAT,
    RMB_TYPE_BYTE,
    RMB_TYPE_BOOL,
    RMB_TYPE_STR,
    RMB_TYPE_ARGS,
    RMB_TYPE_NAMED,
    RMB_TYPE_POINTER,
    RMB_TYPE_SLICE,
    RMB_TYPE_ARRAY,
    RMB_TYPE_OPTIONAL,
    RMB_TYPE_FUNCTION
} RmbTypeKind;

typedef struct RmbType RmbType;

struct RmbType {
    RmbTypeKind kind;
    rmb_string name;       // for NAMED, also used as display label for primitives
    RmbType* inner;        // pointer/slice/array/optional element, function return
    size_t array_len;      // ARRAY length when known
    bool error_capable;    // for FUNCTION: has !! error type
    RmbType* error_type;   // for FUNCTION: error type if error_capable
};

// Built-in singletons (initialised at startup)
RmbType* rmb_type_unknown(void);
RmbType* rmb_type_never(void);
RmbType* rmb_type_void(void);
RmbType* rmb_type_int(void);
RmbType* rmb_type_uint(void);
RmbType* rmb_type_float(void);
RmbType* rmb_type_byte(void);
RmbType* rmb_type_bool(void);
RmbType* rmb_type_str(void);
RmbType* rmb_type_args(void);

// Constructors using arena
RmbType* rmb_type_make_named(rmb_arena* arena, rmb_string name);
RmbType* rmb_type_make_pointer(rmb_arena* arena, RmbType* inner);
RmbType* rmb_type_make_slice(rmb_arena* arena, RmbType* inner);
RmbType* rmb_type_make_array(rmb_arena* arena, RmbType* inner, size_t len);
RmbType* rmb_type_make_optional(rmb_arena* arena, RmbType* inner);

// Lookup primitive by name; returns NULL if not a primitive
RmbType* rmb_type_lookup_primitive(rmb_string name);

// Helpers
const char* rmb_type_kind_name(RmbTypeKind kind);
void rmb_type_print(RmbType* type);
bool rmb_type_equal(RmbType* a, RmbType* b);
bool rmb_type_is_numeric(RmbType* type);
bool rmb_type_is_optional(RmbType* type);
bool rmb_type_is_unknown(RmbType* type);

#endif // RMB_TYPE_H
