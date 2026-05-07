// RauMa Bootstrap Compiler - AST Implementation
// v0.0.3: Parser foundation

#include <stdio.h>
#include <string.h>

#include "rmb/ast.h"

// Arena for AST allocation
static rmb_arena* g_ast_arena = NULL;

// Initialize AST arena
void rmb_ast_init(rmb_arena* arena) {
    g_ast_arena = arena;
}

// Get AST arena
rmb_arena* rmb_ast_get_arena(void) {
    return g_ast_arena;
}

// Helper: allocate AST node
static void* ast_alloc(size_t size) {
    RMB_ASSERT(g_ast_arena != NULL);
    return rmb_arena_alloc(g_ast_arena, size);
}

// Type reference creation functions
RmbAstTypeRef* rmb_ast_type_simple(RmbSpan span, rmb_string name) {
    RmbAstTypeRef* type = ast_alloc(sizeof(RmbAstTypeRef));
    if (!type) return NULL;

    type->kind = RMB_AST_TYPE_SIMPLE;
    type->span = span;
    type->simple.name = name;
    return type;
}

RmbAstTypeRef* rmb_ast_type_pointer(RmbSpan span, RmbAstTypeRef* elem) {
    RmbAstTypeRef* type = ast_alloc(sizeof(RmbAstTypeRef));
    if (!type) return NULL;

    type->kind = RMB_AST_TYPE_POINTER;
    type->span = span;
    type->pointer.elem = elem;
    return type;
}

RmbAstTypeRef* rmb_ast_type_slice(RmbSpan span, RmbAstTypeRef* elem) {
    RmbAstTypeRef* type = ast_alloc(sizeof(RmbAstTypeRef));
    if (!type) return NULL;

    type->kind = RMB_AST_TYPE_SLICE;
    type->span = span;
    type->slice.elem = elem;
    return type;
}

RmbAstTypeRef* rmb_ast_type_array(RmbSpan span, RmbAstExpr* size, RmbAstTypeRef* elem) {
    RmbAstTypeRef* type = ast_alloc(sizeof(RmbAstTypeRef));
    if (!type) return NULL;

    type->kind = RMB_AST_TYPE_ARRAY;
    type->span = span;
    type->array.size = size;
    type->array.elem = elem;
    return type;
}

RmbAstTypeRef* rmb_ast_type_optional(RmbSpan span, RmbAstTypeRef* elem) {
    RmbAstTypeRef* type = ast_alloc(sizeof(RmbAstTypeRef));
    if (!type) return NULL;

    type->kind = RMB_AST_TYPE_OPTIONAL;
    type->span = span;
    type->optional.elem = elem;
    return type;
}

RmbAstTypeRef* rmb_ast_type_qualified(RmbSpan span, rmb_string module, rmb_string name) {
    RmbAstTypeRef* type = ast_alloc(sizeof(RmbAstTypeRef));
    if (!type) return NULL;

    type->kind = RMB_AST_TYPE_QUALIFIED;
    type->span = span;
    type->qualified.module = module;
    type->qualified.name = name;
    return type;
}

// Expression creation functions
RmbAstExpr* rmb_ast_expr_ident(RmbSpan span, rmb_string name) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_IDENT;
    expr->span = span;
    expr->ident.name = name;
    return expr;
}

RmbAstExpr* rmb_ast_expr_int(RmbSpan span, rmb_string value) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_INT;
    expr->span = span;
    expr->int_lit.value = value;
    return expr;
}

RmbAstExpr* rmb_ast_expr_string(RmbSpan span, rmb_string value) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_STRING;
    expr->span = span;
    expr->string_lit.value = value;
    return expr;
}

RmbAstExpr* rmb_ast_expr_bool(RmbSpan span, bool value) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_BOOL;
    expr->span = span;
    expr->bool_lit.value = value;
    return expr;
}

RmbAstExpr* rmb_ast_expr_none(RmbSpan span) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_NONE;
    expr->span = span;
    return expr;
}

RmbAstExpr* rmb_ast_expr_call(RmbSpan span, RmbAstExpr* callee, RmbAstExpr** args, size_t arg_count) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_CALL;
    expr->span = span;
    expr->call.callee = callee;
    expr->call.args = args;
    expr->call.arg_count = arg_count;
    return expr;
}

RmbAstExpr* rmb_ast_expr_field(RmbSpan span, RmbAstExpr* object, rmb_string field) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_FIELD;
    expr->span = span;
    expr->field.object = object;
    expr->field.field = field;
    return expr;
}

RmbAstExpr* rmb_ast_expr_unary(RmbSpan span, RmbTokenKind op, RmbAstExpr* operand) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_UNARY;
    expr->span = span;
    expr->unary.op = op;
    expr->unary.operand = operand;
    return expr;
}

RmbAstExpr* rmb_ast_expr_binary(RmbSpan span, RmbTokenKind op, RmbAstExpr* left, RmbAstExpr* right) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_BINARY;
    expr->span = span;
    expr->binary.op = op;
    expr->binary.left = left;
    expr->binary.right = right;
    return expr;
}

RmbAstExpr* rmb_ast_expr_error_prop(RmbSpan span, RmbAstExpr* operand) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_ERROR_PROP;
    expr->span = span;
    expr->error_prop.operand = operand;
    return expr;
}

RmbAstExpr* rmb_ast_expr_error_panic(RmbSpan span, RmbAstExpr* operand) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_ERROR_PANIC;
    expr->span = span;
    expr->error_panic.operand = operand;
    return expr;
}

RmbAstExpr* rmb_ast_expr_else(RmbSpan span, RmbAstExpr* value, RmbAstExpr* fallback) {
    RmbAstExpr* expr = ast_alloc(sizeof(RmbAstExpr));
    if (!expr) return NULL;

    expr->kind = RMB_AST_EXPR_ELSE;
    expr->span = span;
    expr->else_expr.value = value;
    expr->else_expr.fallback = fallback;
    return expr;
}

// Statement creation functions
RmbAstStmt* rmb_ast_stmt_var(RmbSpan span, rmb_string name, RmbAstTypeRef* type, RmbAstExpr* value) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_VAR;
    stmt->span = span;
    stmt->var.name = name;
    stmt->var.type = type;
    stmt->var.value = value;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_assign(RmbSpan span, RmbAstExpr* target, RmbTokenKind op, RmbAstExpr* value) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_ASSIGN;
    stmt->span = span;
    stmt->assign.target = target;
    stmt->assign.op = op;
    stmt->assign.value = value;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_return(RmbSpan span, RmbAstExpr* value) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_RETURN;
    stmt->span = span;
    stmt->ret.value = value;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_if(RmbSpan span, RmbAstExpr* cond, RmbAstStmt** then_body, size_t then_count,
                           RmbAstStmt** else_body, size_t else_count) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_IF;
    stmt->span = span;
    stmt->if_stmt.cond = cond;
    stmt->if_stmt.then_body = then_body;
    stmt->if_stmt.then_count = then_count;
    stmt->if_stmt.else_body = else_body;
    stmt->if_stmt.else_count = else_count;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_while(RmbSpan span, RmbAstExpr* cond, RmbAstStmt** body, size_t body_count) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_WHILE;
    stmt->span = span;
    stmt->while_stmt.cond = cond;
    stmt->while_stmt.body = body;
    stmt->while_stmt.body_count = body_count;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_for(RmbSpan span, RmbAstStmt* init, RmbAstExpr* cond, RmbAstExpr* step,
                            RmbAstStmt** body, size_t body_count) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_FOR;
    stmt->span = span;
    stmt->for_stmt.init = init;
    stmt->for_stmt.cond = cond;
    stmt->for_stmt.step = step;
    stmt->for_stmt.body = body;
    stmt->for_stmt.body_count = body_count;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_match(RmbSpan span, RmbAstExpr* value, RmbAstMatchCase* cases) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_MATCH;
    stmt->span = span;
    stmt->match_stmt.value = value;
    stmt->match_stmt.cases = cases;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_defer(RmbSpan span, RmbAstExpr* expr) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_DEFER;
    stmt->span = span;
    stmt->defer_stmt.expr = expr;
    return stmt;
}

RmbAstStmt* rmb_ast_stmt_expr(RmbSpan span, RmbAstExpr* expr) {
    RmbAstStmt* stmt = ast_alloc(sizeof(RmbAstStmt));
    if (!stmt) return NULL;

    stmt->kind = RMB_AST_STMT_EXPR;
    stmt->span = span;
    stmt->expr_stmt.expr = expr;
    return stmt;
}

// Item creation functions
RmbAstItem* rmb_ast_item_use(RmbSpan span, rmb_string path) {
    RmbAstItem* item = ast_alloc(sizeof(RmbAstItem));
    if (!item) return NULL;

    item->kind = RMB_AST_ITEM_USE;
    item->span = span;
    item->use_item.path = path;
    item->next = NULL;
    return item;
}

RmbAstItem* rmb_ast_item_fn(RmbSpan span, bool is_pub, rmb_string name, RmbAstParam* params,
                           RmbAstTypeRef* return_type, RmbAstTypeRef* error_type,
                           RmbAstStmt** body, size_t body_count) {
    RmbAstItem* item = ast_alloc(sizeof(RmbAstItem));
    if (!item) return NULL;

    item->kind = RMB_AST_ITEM_FN;
    item->span = span;
    item->fn_item.is_pub = is_pub;
    item->fn_item.name = name;
    item->fn_item.params = params;
    item->fn_item.return_type = return_type;
    item->fn_item.error_type = error_type;
    item->fn_item.body = body;
    item->fn_item.body_count = body_count;
    item->next = NULL;
    return item;
}

RmbAstItem* rmb_ast_item_struct(RmbSpan span, bool is_pub, rmb_string name, RmbAstField* fields) {
    RmbAstItem* item = ast_alloc(sizeof(RmbAstItem));
    if (!item) return NULL;

    item->kind = RMB_AST_ITEM_STRUCT;
    item->span = span;
    item->struct_item.is_pub = is_pub;
    item->struct_item.name = name;
    item->struct_item.fields = fields;
    item->next = NULL;
    return item;
}

RmbAstItem* rmb_ast_item_enum(RmbSpan span, bool is_pub, rmb_string name, RmbAstVariant* variants) {
    RmbAstItem* item = ast_alloc(sizeof(RmbAstItem));
    if (!item) return NULL;

    item->kind = RMB_AST_ITEM_ENUM;
    item->span = span;
    item->enum_item.is_pub = is_pub;
    item->enum_item.name = name;
    item->enum_item.variants = variants;
    item->next = NULL;
    return item;
}

// Variant field creation
RmbAstVariantField* rmb_ast_variant_field(RmbSpan span, rmb_string name, RmbAstTypeRef* type) {
    RmbAstVariantField* field = ast_alloc(sizeof(RmbAstVariantField));
    if (!field) return NULL;

    field->name = name;
    field->type = type;
    field->span = span;
    field->next = NULL;
    return field;
}

// Variant creation
RmbAstVariant* rmb_ast_variant(RmbSpan span, rmb_string name, RmbAstVariantField* fields) {
    RmbAstVariant* variant = ast_alloc(sizeof(RmbAstVariant));
    if (!variant) return NULL;

    variant->name = name;
    variant->fields = fields;
    variant->span = span;
    variant->next = NULL;
    return variant;
}

// Match case binding creation
RmbAstMatchCaseBinding* rmb_ast_match_case_binding(RmbSpan span, rmb_string name) {
    RmbAstMatchCaseBinding* binding = ast_alloc(sizeof(RmbAstMatchCaseBinding));
    if (!binding) return NULL;

    binding->name = name;
    binding->span = span;
    binding->next = NULL;
    return binding;
}

// Match case creation
RmbAstMatchCase* rmb_ast_match_case(RmbSpan span, rmb_string name, RmbAstMatchCaseBinding* bindings,
                                   RmbAstStmt** body, size_t body_count) {
    RmbAstMatchCase* case_node = ast_alloc(sizeof(RmbAstMatchCase));
    if (!case_node) return NULL;

    case_node->name = name;
    case_node->bindings = bindings;
    case_node->body = body;
    case_node->body_count = body_count;
    case_node->span = span;
    case_node->next = NULL;
    return case_node;
}

// File creation
RmbAstFile* rmb_ast_file_create(const char* path, RmbAstItem* items, size_t item_count) {
    RmbAstFile* file = ast_alloc(sizeof(RmbAstFile));
    if (!file) return NULL;

    file->path = path;
    file->items = items;
    file->item_count = item_count;
    return file;
}

// Helper: count parameters
static size_t count_params(RmbAstParam* params) {
    size_t count = 0;
    while (params) {
        count++;
        params = params->next;
    }
    return count;
}

// Helper: count fields
static size_t count_fields(RmbAstField* fields) {
    size_t count = 0;
    while (fields) {
        count++;
        fields = fields->next;
    }
    return count;
}

// Helper: count variant fields
static size_t count_variant_fields(RmbAstVariantField* fields) {
    size_t count = 0;
    while (fields) {
        count++;
        fields = fields->next;
    }
    return count;
}

// Helper: count variants
static size_t count_variants(RmbAstVariant* variants) {
    size_t count = 0;
    while (variants) {
        count++;
        variants = variants->next;
    }
    return count;
}

// Helper: count match case bindings
static size_t count_match_case_bindings(RmbAstMatchCaseBinding* bindings) {
    size_t count = 0;
    while (bindings) {
        count++;
        bindings = bindings->next;
    }
    return count;
}

// Helper: count match cases
static size_t count_match_cases(RmbAstMatchCase* cases) {
    size_t count = 0;
    while (cases) {
        count++;
        cases = cases->next;
    }
    return count;
}

// Helper: print type reference without "type=" prefix
static void print_type_raw(RmbAstTypeRef* type) {
    if (!type) {
        printf("?");
        return;
    }
    switch (type->kind) {
        case RMB_AST_TYPE_SIMPLE:
            printf("%.*s", (int)type->simple.name.len, type->simple.name.ptr);
            break;
        case RMB_AST_TYPE_POINTER:
            printf("*");
            print_type_raw(type->pointer.elem);
            break;
        case RMB_AST_TYPE_SLICE:
            printf("[]");
            print_type_raw(type->slice.elem);
            break;
        case RMB_AST_TYPE_ARRAY:
            printf("[");
            if (type->array.size && type->array.size->kind == RMB_AST_EXPR_INT) {
                printf("%.*s", (int)type->array.size->int_lit.value.len, type->array.size->int_lit.value.ptr);
            } else {
                printf("?");
            }
            printf("]");
            print_type_raw(type->array.elem);
            break;
        case RMB_AST_TYPE_OPTIONAL:
            print_type_raw(type->optional.elem);
            printf("?");
            break;
        case RMB_AST_TYPE_QUALIFIED:
            printf("%.*s.%.*s", (int)type->qualified.module.len, type->qualified.module.ptr,
                   (int)type->qualified.name.len, type->qualified.name.ptr);
            break;
        default:
            printf("?");
            break;
    }
}

// Helper: print type reference summary
static void print_type_summary(RmbAstTypeRef* type) {
    printf("type=");
    print_type_raw(type);
}

// Helper: print function statement summary
static void print_stmt_summary(RmbAstStmt* stmt, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");

    switch (stmt->kind) {
        case RMB_AST_STMT_VAR:
            printf("stmt var %.*s ", (int)stmt->var.name.len, stmt->var.name.ptr);
            if (stmt->var.type) {
                print_type_summary(stmt->var.type);
            } else {
                printf("type=?");
            }
            printf("\n");
            break;
        case RMB_AST_STMT_ASSIGN:
            printf("stmt assign\n");
            break;
        case RMB_AST_STMT_RETURN:
            printf("stmt return\n");
            break;
        case RMB_AST_STMT_IF:
            printf("stmt if\n");
            break;
        case RMB_AST_STMT_WHILE:
            printf("stmt while\n");
            break;
        case RMB_AST_STMT_FOR:
            printf("stmt for\n");
            break;
        case RMB_AST_STMT_MATCH: {
            size_t case_count = count_match_cases(stmt->match_stmt.cases);
            printf("stmt match cases=%zu\n", case_count);
            RmbAstMatchCase* case_node = stmt->match_stmt.cases;
            while (case_node) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                size_t binding_count = count_match_case_bindings(case_node->bindings);
                printf("case %.*s bindings=%zu\n", (int)case_node->name.len, case_node->name.ptr, binding_count);
                case_node = case_node->next;
            }
            break;
        }
        case RMB_AST_STMT_DEFER:
            printf("stmt defer\n");
            break;
        case RMB_AST_STMT_EXPR:
            printf("stmt expr\n");
            break;
        default:
            printf("stmt unknown\n");
            break;
    }
}

// AST summary printing
void rmb_ast_file_print_summary(RmbAstFile* file) {
    printf("file %s\n", file->path);
    printf("items %zu\n", file->item_count);

    RmbAstItem* item = file->items;
    while (item) {
        switch (item->kind) {
            case RMB_AST_ITEM_USE:
                printf("use %.*s\n", (int)item->use_item.path.len, item->use_item.path.ptr);
                break;

            case RMB_AST_ITEM_FN: {
                const char* pub_str = item->fn_item.is_pub ? "pub " : "";
                size_t param_count = count_params(item->fn_item.params);
                printf("%sfn %.*s params=%zu", pub_str,
                       (int)item->fn_item.name.len, item->fn_item.name.ptr,
                       param_count);
                if (item->fn_item.return_type) {
                    printf(" return=");
                    print_type_summary(item->fn_item.return_type);
                }
                if (item->fn_item.error_type) {
                    printf(" error=");
                    print_type_summary(item->fn_item.error_type);
                }
                printf("\n");

                // Print parameters
                RmbAstParam* param = item->fn_item.params;
                while (param) {
                    printf("  param %.*s ", (int)param->name.len, param->name.ptr);
                    print_type_summary(param->type);
                    printf("\n");
                    param = param->next;
                }

                // Print statement summary
                for (size_t i = 0; i < item->fn_item.body_count; i++) {
                    print_stmt_summary(item->fn_item.body[i], 1);
                }
                break;
            }

            case RMB_AST_ITEM_STRUCT: {
                const char* pub_str = item->struct_item.is_pub ? "pub " : "";
                size_t field_count = count_fields(item->struct_item.fields);
                printf("%sstruct %.*s fields=%zu\n", pub_str,
                       (int)item->struct_item.name.len, item->struct_item.name.ptr,
                       field_count);

                // Print fields
                RmbAstField* field = item->struct_item.fields;
                while (field) {
                    printf("  field %.*s ", (int)field->name.len, field->name.ptr);
                    print_type_summary(field->type);
                    printf("\n");
                    field = field->next;
                }
                break;
            }

            case RMB_AST_ITEM_ENUM: {
                const char* pub_str = item->enum_item.is_pub ? "pub " : "";
                size_t variant_count = count_variants(item->enum_item.variants);
                printf("%senum %.*s variants=%zu\n", pub_str,
                       (int)item->enum_item.name.len, item->enum_item.name.ptr,
                       variant_count);

                // Print variants
                RmbAstVariant* variant = item->enum_item.variants;
                while (variant) {
                    size_t field_count = count_variant_fields(variant->fields);
                    printf("  variant %.*s fields=%zu\n", (int)variant->name.len, variant->name.ptr, field_count);

                    // Print variant fields
                    RmbAstVariantField* field = variant->fields;
                    while (field) {
                        printf("    field %.*s ", (int)field->name.len, field->name.ptr);
                        print_type_summary(field->type);
                        printf("\n");
                        field = field->next;
                    }
                    variant = variant->next;
                }
                break;
            }

            default:
                printf("item unknown\n");
                break;
        }

        item = item->next;
    }
}