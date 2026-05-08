// RauMa Bootstrap Compiler - C Code Generator
// v0.0.6: Emit portable C from checked AST.

#ifndef RMB_CGEN_H
#define RMB_CGEN_H

#include <stdio.h>

#include "common.h"
#include "arena.h"
#include "ast.h"

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
} RmbCGen;

// Initialize the code generator.
void rmb_cgen_init(RmbCGen* cgen, rmb_arena* arena, RmbAstFile* file);

// Emit C source for the AST file to out_path. Returns true on success.
bool rmb_cgen_emit_file(RmbCGen* cgen, const char* out_path);

#endif // RMB_CGEN_H
