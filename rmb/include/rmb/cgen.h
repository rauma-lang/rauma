// RauMa Bootstrap Compiler - C Code Generator
// v0.0.7: Emit portable C from checked AST chunks.

#ifndef RMB_CGEN_H
#define RMB_CGEN_H

#include <stdio.h>

#include "common.h"
#include "arena.h"
#include "ast.h"

typedef struct RmbCGenExternalFn {
    const char* module_path;
    const char* module_prefix;
    RmbAstItem* item;
} RmbCGenExternalFn;

typedef struct RmbCGenExternalStruct {
    const char* module_path;
    const char* module_prefix;
    RmbAstItem* item;
} RmbCGenExternalStruct;

typedef struct RmbCGenOptions {
    const char* source_path;
    const char* module_path;
    const char* module_prefix;
    bool is_entry;
    RmbCGenExternalFn* external_fns;
    size_t external_fn_count;
    RmbCGenExternalStruct* external_structs;
    size_t external_struct_count;
} RmbCGenOptions;

// Code generator state. Keeps internal symbol tables as opaque pointers.
typedef struct RmbCGen {
    rmb_arena* arena;
    RmbAstFile* file;
    bool had_error;

    void* fn_table;        // CgFn*
    void* struct_table;    // CgStruct*
    void* current_scope;   // CgScope*
    void* current_fn;      // CgFn*

    FILE* out;
    int indent;
    RmbCGenOptions options;
} RmbCGen;

// Initialize the code generator.
void rmb_cgen_init(RmbCGen* cgen, rmb_arena* arena, RmbAstFile* file);

// Emit C source for the AST file to out_path. Returns true on success.
bool rmb_cgen_emit_file(RmbCGen* cgen, const char* out_path, RmbCGenOptions options);

#endif // RMB_CGEN_H
