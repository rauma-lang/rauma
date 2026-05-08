// RauMa Bootstrap Compiler - C Code Generator
// v0.0.6: Emit portable C99/C11 from a checked RauMa AST.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "rmb/cgen.h"
#include "rmb/diag.h"
#include "rmb/token.h"
#include "rmb/type.h"

// ----- Internal symbol tables -----

typedef struct CgVar CgVar;
struct CgVar {
    rmb_string name;
    RmbType* type;
    CgVar* next;
};

typedef struct CgScope CgScope;
struct CgScope {
    CgVar* vars;
    CgScope* parent;
};

typedef struct CgField CgField;
struct CgField {
    rmb_string name;
    RmbType* type;
    CgField* next;
};

typedef struct CgStruct CgStruct;
struct CgStruct {
    rmb_string name;
    CgField* fields;
    CgStruct* next;
};

typedef struct CgFnParam CgFnParam;
struct CgFnParam {
    rmb_string name;
    RmbType* type;
    CgFnParam* next;
};

typedef struct CgFn CgFn;
struct CgFn {
    rmb_string name;
    CgFnParam* params;
    size_t param_count;
    RmbType* return_type;
    bool error_capable;
    bool is_main;
    RmbAstItem* ast;
    CgFn* next;
};

// ----- Diagnostics -----

static void cg_error(RmbCGen* g, RmbSpan span, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    rmb_diag_error_at(span, "%s", buf);
    g->had_error = true;
}

// ----- Symbol lookup -----

static CgStruct* find_struct(RmbCGen* g, rmb_string name) {
    for (CgStruct* s = (CgStruct*)g->struct_table; s; s = s->next) {
        if (rmb_string_equal(s->name, name)) return s;
    }
    return NULL;
}

static CgFn* find_fn(RmbCGen* g, rmb_string name) {
    for (CgFn* f = (CgFn*)g->fn_table; f; f = f->next) {
        if (rmb_string_equal(f->name, name)) return f;
    }
    return NULL;
}

static CgVar* find_var(RmbCGen* g, rmb_string name) {
    for (CgScope* s = (CgScope*)g->current_scope; s; s = s->parent) {
        for (CgVar* v = s->vars; v; v = v->next) {
            if (rmb_string_equal(v->name, name)) return v;
        }
    }
    return NULL;
}

static void scope_push(RmbCGen* g) {
    CgScope* s = rmb_arena_alloc(g->arena, sizeof(CgScope));
    s->vars = NULL;
    s->parent = (CgScope*)g->current_scope;
    g->current_scope = s;
}

static void scope_pop(RmbCGen* g) {
    CgScope* s = (CgScope*)g->current_scope;
    if (s) g->current_scope = s->parent;
}

static void scope_add_var(RmbCGen* g, rmb_string name, RmbType* type) {
    CgScope* s = (CgScope*)g->current_scope;
    if (!s) return;
    CgVar* v = rmb_arena_alloc(g->arena, sizeof(CgVar));
    v->name = name;
    v->type = type;
    v->next = s->vars;
    s->vars = v;
}

// ----- Type resolution from AST type refs -----

static RmbType* resolve_type_ref(RmbCGen* g, RmbAstTypeRef* ref) {
    if (!ref) return rmb_type_unknown();
    switch (ref->kind) {
        case RMB_AST_TYPE_SIMPLE: {
            RmbType* prim = rmb_type_lookup_primitive(ref->simple.name);
            if (prim) return prim;
            if (find_struct(g, ref->simple.name)) {
                return rmb_type_make_named(g->arena, ref->simple.name);
            }
            return rmb_type_make_named(g->arena, ref->simple.name);
        }
        case RMB_AST_TYPE_POINTER:
            return rmb_type_make_pointer(g->arena, resolve_type_ref(g, ref->pointer.elem));
        case RMB_AST_TYPE_SLICE:
            return rmb_type_make_slice(g->arena, resolve_type_ref(g, ref->slice.elem));
        case RMB_AST_TYPE_ARRAY: {
            size_t len = 0;
            if (ref->array.size && ref->array.size->kind == RMB_AST_EXPR_INT) {
                rmb_string v = ref->array.size->int_lit.value;
                for (size_t i = 0; i < v.len; i++) {
                    char ch = v.ptr[i];
                    if (ch < '0' || ch > '9') break;
                    len = len * 10 + (size_t)(ch - '0');
                }
            }
            return rmb_type_make_array(g->arena, resolve_type_ref(g, ref->array.elem), len);
        }
        case RMB_AST_TYPE_OPTIONAL:
            return rmb_type_make_optional(g->arena, resolve_type_ref(g, ref->optional.elem));
        case RMB_AST_TYPE_QUALIFIED: {
            size_t total = ref->qualified.module.len + 1 + ref->qualified.name.len;
            char* buf = rmb_arena_alloc(g->arena, total + 1);
            memcpy(buf, ref->qualified.module.ptr, ref->qualified.module.len);
            buf[ref->qualified.module.len] = '.';
            memcpy(buf + ref->qualified.module.len + 1,
                   ref->qualified.name.ptr, ref->qualified.name.len);
            buf[total] = '\0';
            rmb_string name = { buf, total };
            return rmb_type_make_named(g->arena, name);
        }
        default:
            return rmb_type_unknown();
    }
}

// ----- Expression type inference -----

static RmbType* expr_type(RmbCGen* g, RmbAstExpr* e);

static RmbType* call_type(RmbCGen* g, RmbAstExpr* e) {
    RmbAstExpr* callee = e->call.callee;
    if (callee && callee->kind == RMB_AST_EXPR_IDENT) {
        rmb_string name = callee->ident.name;
        if (rmb_string_equal(name, rmb_string_from_cstr("print"))) {
            return rmb_type_void();
        }
        CgFn* fn = find_fn(g, name);
        if (fn) return fn->return_type ? fn->return_type : rmb_type_void();
    }
    return rmb_type_unknown();
}

static RmbType* expr_type(RmbCGen* g, RmbAstExpr* e) {
    if (!e) return rmb_type_unknown();
    switch (e->kind) {
        case RMB_AST_EXPR_INT:
            return rmb_type_int();
        case RMB_AST_EXPR_STRING:
            return rmb_type_str();
        case RMB_AST_EXPR_BOOL:
            return rmb_type_bool();
        case RMB_AST_EXPR_NONE:
            return rmb_type_make_optional(g->arena, rmb_type_unknown());
        case RMB_AST_EXPR_IDENT: {
            CgVar* v = find_var(g, e->ident.name);
            if (v) return v->type;
            return rmb_type_unknown();
        }
        case RMB_AST_EXPR_CALL:
            return call_type(g, e);
        case RMB_AST_EXPR_FIELD: {
            RmbType* base = expr_type(g, e->field.object);
            if (base && base->kind == RMB_TYPE_POINTER) base = base->inner;
            if (base && base->kind == RMB_TYPE_NAMED) {
                CgStruct* s = find_struct(g, base->name);
                if (s) {
                    for (CgField* f = s->fields; f; f = f->next) {
                        if (rmb_string_equal(f->name, e->field.field)) {
                            return f->type;
                        }
                    }
                }
            }
            return rmb_type_unknown();
        }
        case RMB_AST_EXPR_UNARY: {
            RmbType* inner = expr_type(g, e->unary.operand);
            switch (e->unary.op) {
                case RMB_TOKEN_BANG:
                    return rmb_type_bool();
                case RMB_TOKEN_MINUS:
                    return inner ? inner : rmb_type_int();
                case RMB_TOKEN_STAR:
                    if (inner && inner->kind == RMB_TYPE_POINTER) return inner->inner;
                    return rmb_type_unknown();
                case RMB_TOKEN_AMP:
                    return rmb_type_make_pointer(g->arena, inner);
                default:
                    return inner;
            }
        }
        case RMB_AST_EXPR_BINARY: {
            switch (e->binary.op) {
                case RMB_TOKEN_LT:
                case RMB_TOKEN_GT:
                case RMB_TOKEN_LT_EQ:
                case RMB_TOKEN_GT_EQ:
                case RMB_TOKEN_EQ_EQ:
                case RMB_TOKEN_BANG_EQ:
                case RMB_TOKEN_AND_AND:
                case RMB_TOKEN_OR_OR:
                    return rmb_type_bool();
                default: {
                    RmbType* lt = expr_type(g, e->binary.left);
                    if (lt && !rmb_type_is_unknown(lt)) return lt;
                    return expr_type(g, e->binary.right);
                }
            }
        }
        case RMB_AST_EXPR_ERROR_PROP:
            return expr_type(g, e->error_prop.operand);
        case RMB_AST_EXPR_ERROR_PANIC:
            return expr_type(g, e->error_panic.operand);
        case RMB_AST_EXPR_ELSE: {
            RmbType* vt = expr_type(g, e->else_expr.value);
            if (vt && vt->kind == RMB_TYPE_OPTIONAL) return vt->inner;
            return vt;
        }
        default:
            return rmb_type_unknown();
    }
}

// ----- Output helpers -----

static void emit_indent(RmbCGen* g) {
    for (int i = 0; i < g->indent; i++) {
        fputs("    ", g->out);
    }
}

static void emit_str(RmbCGen* g, const char* s) {
    fputs(s, g->out);
}

static void emit_fmt(RmbCGen* g, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g->out, fmt, ap);
    va_end(ap);
}

static void emit_rmb_string(RmbCGen* g, rmb_string s) {
    fwrite(s.ptr, 1, s.len, g->out);
}

// ----- Type emission -----

static void emit_c_type(RmbCGen* g, RmbType* t) {
    if (!t) { emit_str(g, "int64_t"); return; }
    switch (t->kind) {
        case RMB_TYPE_VOID:    emit_str(g, "void"); break;
        case RMB_TYPE_INT:     emit_str(g, "int64_t"); break;
        case RMB_TYPE_UINT:    emit_str(g, "uint64_t"); break;
        case RMB_TYPE_FLOAT:   emit_str(g, "double"); break;
        case RMB_TYPE_BYTE:    emit_str(g, "uint8_t"); break;
        case RMB_TYPE_BOOL:    emit_str(g, "bool"); break;
        case RMB_TYPE_STR:     emit_str(g, "RmStr"); break;
        case RMB_TYPE_NAMED:
            if (find_struct(g, t->name)) {
                emit_str(g, "Rm_");
                emit_rmb_string(g, t->name);
            } else {
                emit_str(g, "int64_t"); // unknown named -> fallback
            }
            break;
        case RMB_TYPE_POINTER:
            emit_c_type(g, t->inner);
            emit_str(g, "*");
            break;
        default:
            emit_str(g, "int64_t");
            break;
    }
}

// ----- Symbol collection -----

static void collect_struct(RmbCGen* g, RmbAstItem* item) {
    rmb_string name = item->struct_item.name;
    if (find_struct(g, name)) return;
    CgStruct* s = rmb_arena_alloc(g->arena, sizeof(CgStruct));
    s->name = name;
    s->fields = NULL;
    s->next = (CgStruct*)g->struct_table;
    g->struct_table = s;
    CgField* last = NULL;
    for (RmbAstField* f = item->struct_item.fields; f; f = f->next) {
        CgField* fs = rmb_arena_alloc(g->arena, sizeof(CgField));
        fs->name = f->name;
        fs->type = resolve_type_ref(g, f->type);
        fs->next = NULL;
        if (!last) s->fields = fs; else last->next = fs;
        last = fs;
    }
}

static void collect_fn(RmbCGen* g, RmbAstItem* item) {
    rmb_string name = item->fn_item.name;
    if (find_fn(g, name)) return;
    CgFn* f = rmb_arena_alloc(g->arena, sizeof(CgFn));
    f->name = name;
    f->params = NULL;
    f->param_count = 0;
    f->return_type = item->fn_item.return_type
        ? resolve_type_ref(g, item->fn_item.return_type)
        : NULL;
    f->error_capable = item->fn_item.error_type != NULL;
    f->is_main = rmb_string_equal(name, rmb_string_from_cstr("main"));
    f->ast = item;
    f->next = (CgFn*)g->fn_table;
    g->fn_table = f;
    CgFnParam* last = NULL;
    for (RmbAstParam* p = item->fn_item.params; p; p = p->next) {
        CgFnParam* ps = rmb_arena_alloc(g->arena, sizeof(CgFnParam));
        ps->name = p->name;
        ps->type = p->type ? resolve_type_ref(g, p->type) : rmb_type_unknown();
        ps->next = NULL;
        if (!last) f->params = ps; else last->next = ps;
        last = ps;
        f->param_count++;
    }
}

// ----- Forward declarations for emission -----

static void emit_expr(RmbCGen* g, RmbAstExpr* e);
static void emit_stmt(RmbCGen* g, RmbAstStmt* s);
static void emit_block(RmbCGen* g, RmbAstStmt** body, size_t count);

// ----- Print built-in -----

static void emit_print_call(RmbCGen* g, RmbAstExpr* e) {
    if (e->call.arg_count != 1) {
        cg_error(g, e->span, "print expects exactly 1 argument");
        emit_str(g, "0");
        return;
    }
    RmbAstExpr* arg = e->call.args[0];

    // String literal short-cut.
    if (arg->kind == RMB_AST_EXPR_STRING) {
        emit_str(g, "rm_print(rm_str(");
        emit_rmb_string(g, arg->string_lit.value);
        emit_str(g, "))");
        return;
    }

    RmbType* t = expr_type(g, arg);
    if (!t || rmb_type_is_unknown(t)) {
        cg_error(g, e->span,
            "cannot determine type of print argument in codegen v0.0.6");
        emit_str(g, "0");
        return;
    }

    switch (t->kind) {
        case RMB_TYPE_STR:
            emit_str(g, "rm_print(");
            emit_expr(g, arg);
            emit_str(g, ")");
            break;
        case RMB_TYPE_INT:
        case RMB_TYPE_UINT:
        case RMB_TYPE_BYTE:
            emit_str(g, "rm_print_int((int64_t)(");
            emit_expr(g, arg);
            emit_str(g, "))");
            break;
        case RMB_TYPE_BOOL:
            emit_str(g, "rm_print_bool(");
            emit_expr(g, arg);
            emit_str(g, ")");
            break;
        default:
            cg_error(g, e->span,
                "print of this type is not supported by codegen v0.0.6");
            emit_str(g, "0");
            break;
    }
}

// ----- Expression emission -----

static const char* binary_op_str(RmbTokenKind op) {
    switch (op) {
        case RMB_TOKEN_PLUS:    return "+";
        case RMB_TOKEN_MINUS:   return "-";
        case RMB_TOKEN_STAR:    return "*";
        case RMB_TOKEN_SLASH:   return "/";
        case RMB_TOKEN_PERCENT: return "%";
        case RMB_TOKEN_LT:      return "<";
        case RMB_TOKEN_GT:      return ">";
        case RMB_TOKEN_LT_EQ:   return "<=";
        case RMB_TOKEN_GT_EQ:   return ">=";
        case RMB_TOKEN_EQ_EQ:   return "==";
        case RMB_TOKEN_BANG_EQ: return "!=";
        case RMB_TOKEN_AND_AND: return "&&";
        case RMB_TOKEN_OR_OR:   return "||";
        default:                return "?";
    }
}

static const char* unary_op_str(RmbTokenKind op) {
    switch (op) {
        case RMB_TOKEN_MINUS: return "-";
        case RMB_TOKEN_BANG:  return "!";
        case RMB_TOKEN_STAR:  return "*";
        case RMB_TOKEN_AMP:   return "&";
        default:              return "?";
    }
}

static void emit_call(RmbCGen* g, RmbAstExpr* e) {
    RmbAstExpr* callee = e->call.callee;
    if (callee && callee->kind == RMB_AST_EXPR_IDENT) {
        rmb_string name = callee->ident.name;
        if (rmb_string_equal(name, rmb_string_from_cstr("print"))) {
            emit_print_call(g, e);
            return;
        }
        CgFn* fn = find_fn(g, name);
        if (fn && fn->error_capable) {
            cg_error(g, e->span,
                "error-returning functions are not supported by codegen v0.0.6");
            emit_str(g, "0");
            return;
        }
        emit_str(g, "rm_fn_");
        emit_rmb_string(g, name);
        emit_str(g, "(");
        for (size_t i = 0; i < e->call.arg_count; i++) {
            if (i > 0) emit_str(g, ", ");
            emit_expr(g, e->call.args[i]);
        }
        emit_str(g, ")");
        return;
    }
    cg_error(g, e->span, "call target not supported by codegen v0.0.6");
    emit_str(g, "0");
}

static void emit_expr(RmbCGen* g, RmbAstExpr* e) {
    if (!e) { emit_str(g, "0"); return; }
    switch (e->kind) {
        case RMB_AST_EXPR_INT:
            emit_rmb_string(g, e->int_lit.value);
            break;
        case RMB_AST_EXPR_STRING:
            emit_str(g, "rm_str(");
            emit_rmb_string(g, e->string_lit.value);
            emit_str(g, ")");
            break;
        case RMB_AST_EXPR_BOOL:
            emit_str(g, e->bool_lit.value ? "true" : "false");
            break;
        case RMB_AST_EXPR_IDENT:
            emit_rmb_string(g, e->ident.name);
            break;
        case RMB_AST_EXPR_CALL:
            emit_call(g, e);
            break;
        case RMB_AST_EXPR_FIELD:
            emit_str(g, "(");
            emit_expr(g, e->field.object);
            emit_str(g, ").");
            emit_rmb_string(g, e->field.field);
            break;
        case RMB_AST_EXPR_UNARY:
            emit_str(g, "(");
            emit_str(g, unary_op_str(e->unary.op));
            emit_expr(g, e->unary.operand);
            emit_str(g, ")");
            break;
        case RMB_AST_EXPR_BINARY:
            emit_str(g, "(");
            emit_expr(g, e->binary.left);
            emit_fmt(g, " %s ", binary_op_str(e->binary.op));
            emit_expr(g, e->binary.right);
            emit_str(g, ")");
            break;
        case RMB_AST_EXPR_ERROR_PROP:
            cg_error(g, e->span,
                "error propagation '?' is not supported by codegen v0.0.6");
            emit_str(g, "0");
            break;
        case RMB_AST_EXPR_ERROR_PANIC:
            cg_error(g, e->span,
                "error panic '!' is not supported by codegen v0.0.6");
            emit_str(g, "0");
            break;
        case RMB_AST_EXPR_ELSE:
            cg_error(g, e->span,
                "'else' fallback expression is not supported by codegen v0.0.6");
            emit_str(g, "0");
            break;
        case RMB_AST_EXPR_NONE:
            cg_error(g, e->span,
                "'none' literal is not supported by codegen v0.0.6");
            emit_str(g, "0");
            break;
        default:
            cg_error(g, e->span, "unsupported expression in codegen v0.0.6");
            emit_str(g, "0");
            break;
    }
}

static const char* assign_op_str(RmbTokenKind op) {
    switch (op) {
        case RMB_TOKEN_EQ:       return "=";
        case RMB_TOKEN_PLUS_EQ:  return "+=";
        case RMB_TOKEN_MINUS_EQ: return "-=";
        case RMB_TOKEN_STAR_EQ:  return "*=";
        case RMB_TOKEN_SLASH_EQ: return "/=";
        default:                 return "=";
    }
}

// ----- Statement emission -----

static void emit_var_stmt(RmbCGen* g, RmbAstStmt* s) {
    RmbType* t = NULL;
    if (s->var.type) {
        t = resolve_type_ref(g, s->var.type);
    } else if (s->var.value) {
        t = expr_type(g, s->var.value);
    }
    if (!t || rmb_type_is_unknown(t)) {
        // Fall back to int64_t for untyped declarations.
        t = rmb_type_int();
    }
    scope_add_var(g, s->var.name, t);
    emit_indent(g);
    emit_c_type(g, t);
    emit_str(g, " ");
    emit_rmb_string(g, s->var.name);
    if (s->var.value) {
        emit_str(g, " = ");
        emit_expr(g, s->var.value);
    }
    emit_str(g, ";\n");
}

static void emit_assign_stmt(RmbCGen* g, RmbAstStmt* s) {
    emit_indent(g);
    emit_expr(g, s->assign.target);
    emit_fmt(g, " %s ", assign_op_str(s->assign.op));
    emit_expr(g, s->assign.value);
    emit_str(g, ";\n");
}

static void emit_return_stmt(RmbCGen* g, RmbAstStmt* s) {
    emit_indent(g);
    if (s->ret.value) {
        emit_str(g, "return ");
        emit_expr(g, s->ret.value);
        emit_str(g, ";\n");
    } else {
        CgFn* fn = (CgFn*)g->current_fn;
        if (fn && fn->is_main) {
            emit_str(g, "return 0;\n");
        } else {
            emit_str(g, "return;\n");
        }
    }
}

static void emit_if_stmt(RmbCGen* g, RmbAstStmt* s) {
    emit_indent(g);
    emit_str(g, "if (");
    emit_expr(g, s->if_stmt.cond);
    emit_str(g, ") {\n");
    g->indent++;
    emit_block(g, s->if_stmt.then_body, s->if_stmt.then_count);
    g->indent--;
    if (s->if_stmt.else_count > 0 || s->if_stmt.else_body) {
        emit_indent(g);
        emit_str(g, "} else {\n");
        g->indent++;
        emit_block(g, s->if_stmt.else_body, s->if_stmt.else_count);
        g->indent--;
    }
    emit_indent(g);
    emit_str(g, "}\n");
}

static void emit_while_stmt(RmbCGen* g, RmbAstStmt* s) {
    emit_indent(g);
    emit_str(g, "while (");
    emit_expr(g, s->while_stmt.cond);
    emit_str(g, ") {\n");
    g->indent++;
    emit_block(g, s->while_stmt.body, s->while_stmt.body_count);
    g->indent--;
    emit_indent(g);
    emit_str(g, "}\n");
}

static void emit_for_stmt(RmbCGen* g, RmbAstStmt* s) {
    emit_indent(g);
    emit_str(g, "{\n");
    g->indent++;
    scope_push(g);
    if (s->for_stmt.init) emit_stmt(g, s->for_stmt.init);
    emit_indent(g);
    emit_str(g, "while (");
    if (s->for_stmt.cond) {
        emit_expr(g, s->for_stmt.cond);
    } else {
        emit_str(g, "1");
    }
    emit_str(g, ") {\n");
    g->indent++;
    emit_block(g, s->for_stmt.body, s->for_stmt.body_count);
    if (s->for_stmt.step) {
        emit_indent(g);
        emit_expr(g, s->for_stmt.step);
        emit_str(g, ";\n");
    }
    g->indent--;
    emit_indent(g);
    emit_str(g, "}\n");
    scope_pop(g);
    g->indent--;
    emit_indent(g);
    emit_str(g, "}\n");
}

static void emit_stmt(RmbCGen* g, RmbAstStmt* s) {
    if (!s) return;
    switch (s->kind) {
        case RMB_AST_STMT_VAR:
            emit_var_stmt(g, s);
            break;
        case RMB_AST_STMT_ASSIGN:
            emit_assign_stmt(g, s);
            break;
        case RMB_AST_STMT_RETURN:
            emit_return_stmt(g, s);
            break;
        case RMB_AST_STMT_IF:
            emit_if_stmt(g, s);
            break;
        case RMB_AST_STMT_WHILE:
            emit_while_stmt(g, s);
            break;
        case RMB_AST_STMT_FOR:
            emit_for_stmt(g, s);
            break;
        case RMB_AST_STMT_MATCH:
            cg_error(g, s->span,
                "match is not supported by codegen v0.0.6");
            break;
        case RMB_AST_STMT_DEFER:
            cg_error(g, s->span,
                "defer is not supported by codegen v0.0.6");
            break;
        case RMB_AST_STMT_EXPR:
            emit_indent(g);
            emit_expr(g, s->expr_stmt.expr);
            emit_str(g, ";\n");
            break;
        default:
            break;
    }
}

static void emit_block(RmbCGen* g, RmbAstStmt** body, size_t count) {
    scope_push(g);
    for (size_t i = 0; i < count; i++) {
        emit_stmt(g, body[i]);
    }
    scope_pop(g);
}

// ----- Top-level emission -----

static void emit_struct_decl(RmbCGen* g, CgStruct* s) {
    emit_str(g, "typedef struct Rm_");
    emit_rmb_string(g, s->name);
    emit_str(g, " {\n");
    for (CgField* f = s->fields; f; f = f->next) {
        emit_str(g, "    ");
        emit_c_type(g, f->type);
        emit_str(g, " ");
        emit_rmb_string(g, f->name);
        emit_str(g, ";\n");
    }
    emit_str(g, "} Rm_");
    emit_rmb_string(g, s->name);
    emit_str(g, ";\n\n");
}

static void emit_fn_signature(RmbCGen* g, CgFn* fn) {
    if (fn->is_main) {
        emit_str(g, "int main(void)");
        return;
    }
    emit_str(g, "static ");
    if (fn->return_type && fn->return_type->kind != RMB_TYPE_VOID) {
        emit_c_type(g, fn->return_type);
    } else {
        emit_str(g, "void");
    }
    emit_str(g, " rm_fn_");
    emit_rmb_string(g, fn->name);
    emit_str(g, "(");
    if (!fn->params) {
        emit_str(g, "void");
    } else {
        bool first = true;
        for (CgFnParam* p = fn->params; p; p = p->next) {
            if (!first) emit_str(g, ", ");
            first = false;
            emit_c_type(g, p->type);
            emit_str(g, " ");
            emit_rmb_string(g, p->name);
        }
    }
    emit_str(g, ")");
}

static void emit_fn_decl(RmbCGen* g, CgFn* fn) {
    emit_fn_signature(g, fn);
    emit_str(g, ";\n");
}

static bool body_ends_with_return(RmbAstItem* item) {
    if (item->fn_item.body_count == 0) return false;
    RmbAstStmt* last = item->fn_item.body[item->fn_item.body_count - 1];
    return last && last->kind == RMB_AST_STMT_RETURN;
}

static void emit_fn_def(RmbCGen* g, CgFn* fn) {
    if (fn->is_main && fn->params) {
        cg_error(g, fn->ast->span,
            "main arguments are not supported by codegen v0.0.6");
    }
    if (fn->error_capable) {
        cg_error(g, fn->ast->span,
            "error-returning functions are not supported by codegen v0.0.6");
        return;
    }

    g->current_fn = fn;
    emit_fn_signature(g, fn);
    emit_str(g, " {\n");
    g->indent++;
    scope_push(g);
    for (CgFnParam* p = fn->params; p; p = p->next) {
        scope_add_var(g, p->name, p->type);
    }
    for (size_t i = 0; i < fn->ast->fn_item.body_count; i++) {
        emit_stmt(g, fn->ast->fn_item.body[i]);
    }
    if (fn->is_main && !body_ends_with_return(fn->ast)) {
        emit_indent(g);
        emit_str(g, "return 0;\n");
    }
    scope_pop(g);
    g->indent--;
    emit_str(g, "}\n\n");
    g->current_fn = NULL;
}

// ----- Prelude -----

static void emit_prelude(RmbCGen* g) {
    emit_str(g,
        "// Generated by rmb 0.0.6 - do not edit.\n"
        "#include <stdint.h>\n"
        "#include <stddef.h>\n"
        "#include <stdbool.h>\n"
        "#include <stdio.h>\n"
        "#include <string.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "typedef struct {\n"
        "    const char *ptr;\n"
        "    int64_t len;\n"
        "} RmStr;\n"
        "\n"
        "static RmStr rm_str(const char *s) {\n"
        "    RmStr out;\n"
        "    out.ptr = s;\n"
        "    out.len = (int64_t)strlen(s);\n"
        "    return out;\n"
        "}\n"
        "\n"
        "static void rm_print(RmStr s) {\n"
        "    fwrite(s.ptr, 1, (size_t)s.len, stdout);\n"
        "}\n"
        "\n"
        "static void rm_print_int(int64_t v) {\n"
        "    printf(\"%lld\", (long long)v);\n"
        "}\n"
        "\n"
        "static void rm_print_bool(bool b) {\n"
        "    if (b) fwrite(\"true\", 1, 4, stdout);\n"
        "    else fwrite(\"false\", 1, 5, stdout);\n"
        "}\n"
        "\n");
}

// ----- Public API -----

void rmb_cgen_init(RmbCGen* g, rmb_arena* arena, RmbAstFile* file) {
    g->arena = arena;
    g->file = file;
    g->had_error = false;
    g->fn_table = NULL;
    g->struct_table = NULL;
    g->current_scope = NULL;
    g->current_fn = NULL;
    g->out = NULL;
    g->indent = 0;
}

bool rmb_cgen_emit_file(RmbCGen* g, const char* out_path) {
    // Pass 1: collect structs and functions.
    for (RmbAstItem* it = g->file->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_STRUCT) collect_struct(g, it);
    }
    for (RmbAstItem* it = g->file->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_FN) collect_fn(g, it);
    }

    g->out = fopen(out_path, "wb");
    if (!g->out) {
        rmb_diag_error("failed to open output file: %s", out_path);
        return false;
    }

    emit_prelude(g);

    // Struct typedefs.
    for (CgStruct* s = (CgStruct*)g->struct_table; s; s = s->next) {
        emit_struct_decl(g, s);
    }

    // Forward declarations for non-main functions.
    for (CgFn* fn = (CgFn*)g->fn_table; fn; fn = fn->next) {
        if (fn->is_main || fn->error_capable) continue;
        emit_fn_decl(g, fn);
    }
    emit_str(g, "\n");

    // Function definitions.
    for (CgFn* fn = (CgFn*)g->fn_table; fn; fn = fn->next) {
        emit_fn_def(g, fn);
    }

    fclose(g->out);
    g->out = NULL;
    return !g->had_error;
}
