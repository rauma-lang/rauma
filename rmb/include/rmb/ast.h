// RauMa Bootstrap Compiler - Abstract Syntax Tree
// v0.0.3: Parser foundation

#ifndef RMB_AST_H
#define RMB_AST_H

#include "common.h"
#include "span.h"
#include "token.h"
#include "arena.h"

// Forward declarations
typedef struct RmbAstFile RmbAstFile;
typedef struct RmbAstItem RmbAstItem;
typedef struct RmbAstStmt RmbAstStmt;
typedef struct RmbAstExpr RmbAstExpr;
typedef struct RmbAstTypeRef RmbAstTypeRef;
typedef struct RmbAstParam RmbAstParam;
typedef struct RmbAstField RmbAstField;
typedef struct RmbAstVariant RmbAstVariant;

// AST node kinds
typedef enum {
    RMB_AST_FILE,

    // Item kinds
    RMB_AST_ITEM_USE,
    RMB_AST_ITEM_FN,
    RMB_AST_ITEM_STRUCT,
    RMB_AST_ITEM_ENUM,

    // Statement kinds
    RMB_AST_STMT_VAR,
    RMB_AST_STMT_ASSIGN,
    RMB_AST_STMT_RETURN,
    RMB_AST_STMT_IF,
    RMB_AST_STMT_WHILE,
    RMB_AST_STMT_FOR,
    RMB_AST_STMT_MATCH,
    RMB_AST_STMT_DEFER,
    RMB_AST_STMT_EXPR,

    // Expression kinds
    RMB_AST_EXPR_IDENT,
    RMB_AST_EXPR_INT,
    RMB_AST_EXPR_STRING,
    RMB_AST_EXPR_BOOL,
    RMB_AST_EXPR_NONE,
    RMB_AST_EXPR_CALL,
    RMB_AST_EXPR_FIELD,
    RMB_AST_EXPR_UNARY,
    RMB_AST_EXPR_BINARY,
    RMB_AST_EXPR_ERROR_PROP,
    RMB_AST_EXPR_ERROR_PANIC,
    RMB_AST_EXPR_ELSE,

    // Type reference kinds
    RMB_AST_TYPE_SIMPLE,
    RMB_AST_TYPE_POINTER,
    RMB_AST_TYPE_SLICE,
    RMB_AST_TYPE_ARRAY,
    RMB_AST_TYPE_OPTIONAL,
    RMB_AST_TYPE_QUALIFIED
} RmbAstKind;

// Type reference
struct RmbAstTypeRef {
    RmbAstKind kind;
    RmbSpan span;
    union {
        // Simple type (int, str, User)
        struct {
            rmb_string name;
        } simple;

        // Pointer type (*T)
        struct {
            RmbAstTypeRef* elem;
        } pointer;

        // Slice type ([]T)
        struct {
            RmbAstTypeRef* elem;
        } slice;

        // Array type ([N]T)
        struct {
            RmbAstExpr* size;
            RmbAstTypeRef* elem;
        } array;

        // Optional type (T?)
        struct {
            RmbAstTypeRef* elem;
        } optional;

        // Qualified type (module.Type)
        struct {
            rmb_string module;
            rmb_string name;
        } qualified;
    };
};

// Function parameter
struct RmbAstParam {
    rmb_string name;
    RmbAstTypeRef* type;  // NULL for inferred parameters
    RmbSpan span;
    RmbAstParam* next;
};

// Struct field
struct RmbAstField {
    rmb_string name;
    RmbAstTypeRef* type;
    RmbSpan span;
    RmbAstField* next;
};

// Enum variant
struct RmbAstVariant {
    rmb_string name;
    RmbAstTypeRef* type;  // NULL for simple variants
    RmbSpan span;
    RmbAstVariant* next;
};

// Expression
struct RmbAstExpr {
    RmbAstKind kind;
    RmbSpan span;
    union {
        // Identifier
        struct {
            rmb_string name;
        } ident;

        // Integer literal
        struct {
            rmb_string value;
        } int_lit;

        // String literal
        struct {
            rmb_string value;
        } string_lit;

        // Boolean literal
        struct {
            bool value;
        } bool_lit;

        // None literal (no data)

        // Function call
        struct {
            RmbAstExpr* callee;
            RmbAstExpr** args;
            size_t arg_count;
        } call;

        // Field access
        struct {
            RmbAstExpr* object;
            rmb_string field;
        } field;

        // Unary expression
        struct {
            RmbTokenKind op;
            RmbAstExpr* operand;
        } unary;

        // Binary expression
        struct {
            RmbTokenKind op;
            RmbAstExpr* left;
            RmbAstExpr* right;
        } binary;

        // Error propagation (expr?)
        struct {
            RmbAstExpr* operand;
        } error_prop;

        // Error panic (expr!)
        struct {
            RmbAstExpr* operand;
        } error_panic;

        // Else expression (expr else fallback)
        struct {
            RmbAstExpr* value;
            RmbAstExpr* fallback;
        } else_expr;
    };
};

// Statement
struct RmbAstStmt {
    RmbAstKind kind;
    RmbSpan span;
    union {
        // Variable declaration
        struct {
            rmb_string name;
            RmbAstTypeRef* type;  // NULL for inferred
            RmbAstExpr* value;
        } var;

        // Assignment
        struct {
            RmbAstExpr* target;
            RmbTokenKind op;  // =, +=, -=, *=, /=
            RmbAstExpr* value;
        } assign;

        // Return statement
        struct {
            RmbAstExpr* value;  // NULL for empty return
        } ret;

        // If statement
        struct {
            RmbAstExpr* cond;
            RmbAstStmt** then_body;
            size_t then_count;
            RmbAstStmt** else_body;
            size_t else_count;
        } if_stmt;

        // While statement
        struct {
            RmbAstExpr* cond;
            RmbAstStmt** body;
            size_t body_count;
        } while_stmt;

        // For statement
        struct {
            RmbAstStmt* init;   // NULL if no init
            RmbAstExpr* cond;   // NULL if no cond
            RmbAstExpr* step;   // NULL if no step
            RmbAstStmt** body;
            size_t body_count;
        } for_stmt;

        // Match statement (simplified for v0.0.3)
        struct {
            RmbAstExpr* value;
            // Cases omitted for v0.0.3
        } match_stmt;

        // Defer statement
        struct {
            RmbAstExpr* expr;
        } defer_stmt;

        // Expression statement
        struct {
            RmbAstExpr* expr;
        } expr_stmt;
    };
};

// Top-level item
struct RmbAstItem {
    RmbAstKind kind;
    RmbSpan span;
    union {
        // Use declaration
        struct {
            rmb_string path;
        } use_item;

        // Function declaration
        struct {
            bool is_pub;
            rmb_string name;
            RmbAstParam* params;
            RmbAstTypeRef* return_type;  // NULL for inferred
            RmbAstTypeRef* error_type;   // NULL if no error
            RmbAstStmt** body;
            size_t body_count;
        } fn_item;

        // Struct declaration
        struct {
            bool is_pub;
            rmb_string name;
            RmbAstField* fields;
        } struct_item;

        // Enum declaration
        struct {
            bool is_pub;
            rmb_string name;
            RmbAstVariant* variants;
        } enum_item;
    };
    RmbAstItem* next;
};

// File (root node)
struct RmbAstFile {
    const char* path;
    RmbAstItem* items;
    size_t item_count;
};

// AST creation functions
RmbAstTypeRef* rmb_ast_type_simple(RmbSpan span, rmb_string name);
RmbAstTypeRef* rmb_ast_type_pointer(RmbSpan span, RmbAstTypeRef* elem);
RmbAstTypeRef* rmb_ast_type_slice(RmbSpan span, RmbAstTypeRef* elem);
RmbAstTypeRef* rmb_ast_type_array(RmbSpan span, RmbAstExpr* size, RmbAstTypeRef* elem);
RmbAstTypeRef* rmb_ast_type_optional(RmbSpan span, RmbAstTypeRef* elem);
RmbAstTypeRef* rmb_ast_type_qualified(RmbSpan span, rmb_string module, rmb_string name);

RmbAstExpr* rmb_ast_expr_ident(RmbSpan span, rmb_string name);
RmbAstExpr* rmb_ast_expr_int(RmbSpan span, rmb_string value);
RmbAstExpr* rmb_ast_expr_string(RmbSpan span, rmb_string value);
RmbAstExpr* rmb_ast_expr_bool(RmbSpan span, bool value);
RmbAstExpr* rmb_ast_expr_none(RmbSpan span);
RmbAstExpr* rmb_ast_expr_call(RmbSpan span, RmbAstExpr* callee, RmbAstExpr** args, size_t arg_count);
RmbAstExpr* rmb_ast_expr_field(RmbSpan span, RmbAstExpr* object, rmb_string field);
RmbAstExpr* rmb_ast_expr_unary(RmbSpan span, RmbTokenKind op, RmbAstExpr* operand);
RmbAstExpr* rmb_ast_expr_binary(RmbSpan span, RmbTokenKind op, RmbAstExpr* left, RmbAstExpr* right);
RmbAstExpr* rmb_ast_expr_error_prop(RmbSpan span, RmbAstExpr* operand);
RmbAstExpr* rmb_ast_expr_error_panic(RmbSpan span, RmbAstExpr* operand);
RmbAstExpr* rmb_ast_expr_else(RmbSpan span, RmbAstExpr* value, RmbAstExpr* fallback);

RmbAstStmt* rmb_ast_stmt_var(RmbSpan span, rmb_string name, RmbAstTypeRef* type, RmbAstExpr* value);
RmbAstStmt* rmb_ast_stmt_assign(RmbSpan span, RmbAstExpr* target, RmbTokenKind op, RmbAstExpr* value);
RmbAstStmt* rmb_ast_stmt_return(RmbSpan span, RmbAstExpr* value);
RmbAstStmt* rmb_ast_stmt_if(RmbSpan span, RmbAstExpr* cond, RmbAstStmt** then_body, size_t then_count,
                           RmbAstStmt** else_body, size_t else_count);
RmbAstStmt* rmb_ast_stmt_while(RmbSpan span, RmbAstExpr* cond, RmbAstStmt** body, size_t body_count);
RmbAstStmt* rmb_ast_stmt_for(RmbSpan span, RmbAstStmt* init, RmbAstExpr* cond, RmbAstExpr* step,
                            RmbAstStmt** body, size_t body_count);
RmbAstStmt* rmb_ast_stmt_match(RmbSpan span, RmbAstExpr* value);
RmbAstStmt* rmb_ast_stmt_defer(RmbSpan span, RmbAstExpr* expr);
RmbAstStmt* rmb_ast_stmt_expr(RmbSpan span, RmbAstExpr* expr);

RmbAstItem* rmb_ast_item_use(RmbSpan span, rmb_string path);
RmbAstItem* rmb_ast_item_fn(RmbSpan span, bool is_pub, rmb_string name, RmbAstParam* params,
                           RmbAstTypeRef* return_type, RmbAstTypeRef* error_type,
                           RmbAstStmt** body, size_t body_count);
RmbAstItem* rmb_ast_item_struct(RmbSpan span, bool is_pub, rmb_string name, RmbAstField* fields);
RmbAstItem* rmb_ast_item_enum(RmbSpan span, bool is_pub, rmb_string name, RmbAstVariant* variants);

RmbAstFile* rmb_ast_file_create(const char* path, RmbAstItem* items, size_t item_count);

// AST summary printing
void rmb_ast_file_print_summary(RmbAstFile* file);

// AST memory management (allocation via arena)
void rmb_ast_init(rmb_arena* arena);
rmb_arena* rmb_ast_get_arena(void);

#endif // RMB_AST_H