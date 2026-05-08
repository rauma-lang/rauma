// RauMa Bootstrap Compiler - Type Representation
// v0.0.5: Basic type checking

#include <stdio.h>
#include <string.h>

#include "rmb/type.h"
#include "rmb/arena.h"

// Static built-in singletons. Names are pointers to constant strings, so they
// outlive the arena and can be reused across files.
static RmbType g_unknown = { RMB_TYPE_UNKNOWN, {"unknown", 7}, NULL, 0, false, NULL };
static RmbType g_never   = { RMB_TYPE_NEVER,   {"never",   5}, NULL, 0, false, NULL };
static RmbType g_void    = { RMB_TYPE_VOID,    {"void",    4}, NULL, 0, false, NULL };
static RmbType g_int     = { RMB_TYPE_INT,     {"int",     3}, NULL, 0, false, NULL };
static RmbType g_uint    = { RMB_TYPE_UINT,    {"uint",    4}, NULL, 0, false, NULL };
static RmbType g_float   = { RMB_TYPE_FLOAT,   {"float",   5}, NULL, 0, false, NULL };
static RmbType g_byte    = { RMB_TYPE_BYTE,    {"byte",    4}, NULL, 0, false, NULL };
static RmbType g_bool    = { RMB_TYPE_BOOL,    {"bool",    4}, NULL, 0, false, NULL };
static RmbType g_str     = { RMB_TYPE_STR,     {"str",     3}, NULL, 0, false, NULL };

RmbType* rmb_type_unknown(void) { return &g_unknown; }
RmbType* rmb_type_never(void)   { return &g_never; }
RmbType* rmb_type_void(void)    { return &g_void; }
RmbType* rmb_type_int(void)     { return &g_int; }
RmbType* rmb_type_uint(void)    { return &g_uint; }
RmbType* rmb_type_float(void)   { return &g_float; }
RmbType* rmb_type_byte(void)    { return &g_byte; }
RmbType* rmb_type_bool(void)    { return &g_bool; }
RmbType* rmb_type_str(void)     { return &g_str; }

static RmbType* alloc_type(rmb_arena* arena) {
    RmbType* t = rmb_arena_alloc(arena, sizeof(RmbType));
    if (!t) return NULL;
    t->kind = RMB_TYPE_UNKNOWN;
    t->name = (rmb_string){NULL, 0};
    t->inner = NULL;
    t->array_len = 0;
    t->error_capable = false;
    t->error_type = NULL;
    return t;
}

RmbType* rmb_type_make_named(rmb_arena* arena, rmb_string name) {
    RmbType* t = alloc_type(arena);
    if (!t) return NULL;
    t->kind = RMB_TYPE_NAMED;
    t->name = name;
    return t;
}

RmbType* rmb_type_make_pointer(rmb_arena* arena, RmbType* inner) {
    RmbType* t = alloc_type(arena);
    if (!t) return NULL;
    t->kind = RMB_TYPE_POINTER;
    t->inner = inner;
    return t;
}

RmbType* rmb_type_make_slice(rmb_arena* arena, RmbType* inner) {
    RmbType* t = alloc_type(arena);
    if (!t) return NULL;
    t->kind = RMB_TYPE_SLICE;
    t->inner = inner;
    return t;
}

RmbType* rmb_type_make_array(rmb_arena* arena, RmbType* inner, size_t len) {
    RmbType* t = alloc_type(arena);
    if (!t) return NULL;
    t->kind = RMB_TYPE_ARRAY;
    t->inner = inner;
    t->array_len = len;
    return t;
}

RmbType* rmb_type_make_optional(rmb_arena* arena, RmbType* inner) {
    RmbType* t = alloc_type(arena);
    if (!t) return NULL;
    t->kind = RMB_TYPE_OPTIONAL;
    t->inner = inner;
    return t;
}

static bool name_eq(rmb_string a, const char* lit, size_t lit_len) {
    return a.len == lit_len && memcmp(a.ptr, lit, lit_len) == 0;
}

RmbType* rmb_type_lookup_primitive(rmb_string name) {
    if (name_eq(name, "int", 3))   return rmb_type_int();
    if (name_eq(name, "uint", 4))  return rmb_type_uint();
    if (name_eq(name, "float", 5)) return rmb_type_float();
    if (name_eq(name, "byte", 4))  return rmb_type_byte();
    if (name_eq(name, "bool", 4))  return rmb_type_bool();
    if (name_eq(name, "str", 3))   return rmb_type_str();
    if (name_eq(name, "void", 4))  return rmb_type_void();
    return NULL;
}

const char* rmb_type_kind_name(RmbTypeKind kind) {
    switch (kind) {
        case RMB_TYPE_UNKNOWN:  return "unknown";
        case RMB_TYPE_NEVER:    return "never";
        case RMB_TYPE_VOID:     return "void";
        case RMB_TYPE_INT:      return "int";
        case RMB_TYPE_UINT:     return "uint";
        case RMB_TYPE_FLOAT:    return "float";
        case RMB_TYPE_BYTE:     return "byte";
        case RMB_TYPE_BOOL:     return "bool";
        case RMB_TYPE_STR:      return "str";
        case RMB_TYPE_NAMED:    return "named";
        case RMB_TYPE_POINTER:  return "pointer";
        case RMB_TYPE_SLICE:    return "slice";
        case RMB_TYPE_ARRAY:    return "array";
        case RMB_TYPE_OPTIONAL: return "optional";
        case RMB_TYPE_FUNCTION: return "function";
    }
    return "?";
}

void rmb_type_print(RmbType* type) {
    if (!type) {
        printf("?");
        return;
    }
    switch (type->kind) {
        case RMB_TYPE_UNKNOWN:
        case RMB_TYPE_NEVER:
        case RMB_TYPE_VOID:
        case RMB_TYPE_INT:
        case RMB_TYPE_UINT:
        case RMB_TYPE_FLOAT:
        case RMB_TYPE_BYTE:
        case RMB_TYPE_BOOL:
        case RMB_TYPE_STR:
            printf("%s", rmb_type_kind_name(type->kind));
            break;
        case RMB_TYPE_NAMED:
            printf("%.*s", (int)type->name.len, type->name.ptr);
            break;
        case RMB_TYPE_POINTER:
            printf("*");
            rmb_type_print(type->inner);
            break;
        case RMB_TYPE_SLICE:
            printf("[]");
            rmb_type_print(type->inner);
            break;
        case RMB_TYPE_ARRAY:
            printf("[%zu]", type->array_len);
            rmb_type_print(type->inner);
            break;
        case RMB_TYPE_OPTIONAL:
            rmb_type_print(type->inner);
            printf("?");
            break;
        case RMB_TYPE_FUNCTION:
            printf("fn(...)");
            if (type->inner) {
                printf(" ");
                rmb_type_print(type->inner);
            }
            break;
    }
}

bool rmb_type_equal(RmbType* a, RmbType* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;
    switch (a->kind) {
        case RMB_TYPE_NAMED:
            return rmb_string_equal(a->name, b->name);
        case RMB_TYPE_POINTER:
        case RMB_TYPE_SLICE:
        case RMB_TYPE_OPTIONAL:
            return rmb_type_equal(a->inner, b->inner);
        case RMB_TYPE_ARRAY:
            return a->array_len == b->array_len && rmb_type_equal(a->inner, b->inner);
        case RMB_TYPE_FUNCTION:
            return rmb_type_equal(a->inner, b->inner);
        default:
            return true; // primitives with same kind are equal
    }
}

bool rmb_type_is_numeric(RmbType* type) {
    if (!type) return false;
    switch (type->kind) {
        case RMB_TYPE_INT:
        case RMB_TYPE_UINT:
        case RMB_TYPE_FLOAT:
        case RMB_TYPE_BYTE:
            return true;
        default:
            return false;
    }
}

bool rmb_type_is_optional(RmbType* type) {
    return type && type->kind == RMB_TYPE_OPTIONAL;
}

bool rmb_type_is_unknown(RmbType* type) {
    return !type || type->kind == RMB_TYPE_UNKNOWN;
}
