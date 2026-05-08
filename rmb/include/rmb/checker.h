// RauMa Bootstrap Compiler - Type Checker
// v0.0.5: Basic type checking

#ifndef RMB_CHECKER_H
#define RMB_CHECKER_H

#include "common.h"
#include "arena.h"
#include "ast.h"
#include "type.h"

typedef struct RmbChecker {
    rmb_arena* arena;
    RmbAstFile* file;
    bool had_error;

    // Global symbols (linked lists, internal)
    void* fn_symbols;       // RmbFnSymbol*
    void* struct_symbols;   // RmbStructSymbol*
    void* enum_symbols;     // RmbEnumSymbol*

    // Current function context
    void* current_fn;       // RmbFnSymbol* during body checking
    void* current_scope;    // RmbScope* (linked stack)
} RmbChecker;

void rmb_checker_init(RmbChecker* checker, rmb_arena* arena, RmbAstFile* file);
bool rmb_check_file(RmbChecker* checker);

#endif // RMB_CHECKER_H
