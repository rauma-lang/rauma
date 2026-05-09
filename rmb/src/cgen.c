// RauMa Bootstrap Compiler - C Code Generator
// v0.0.7: Emit portable C99/C11 from checked RauMa AST chunks.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "rmb/cgen.h"
#include "rmb/diag.h"
#include "rmb/token.h"
#include "rmb/type.h"

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
    const char* c_name;
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
    const char* module_path;
    const char* c_name;
    CgFnParam* params;
    size_t param_count;
    RmbType* return_type;
    bool error_capable;
    bool is_main;
    bool is_external;
    RmbAstItem* ast;
    CgFn* next;
};

static void cg_error(RmbCGen* g, RmbSpan span, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    rmb_diag_error_at(span, "%s", buf);
    g->had_error = true;
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

static void emit_c_ident_part(RmbCGen* g, const char* s) {
    for (const char* p = s; *p; p++) {
        char ch = *p;
        fputc((ch == '/' || ch == '\\' || ch == '.' || ch == '-') ? '_' : ch, g->out);
    }
}

static void emit_c_rmb_string_part(RmbCGen* g, rmb_string s) {
    for (size_t i = 0; i < s.len; i++) {
        char ch = s.ptr[i];
        fputc((ch == '/' || ch == '\\' || ch == '.' || ch == '-') ? '_' : ch, g->out);
    }
}

static char* arena_c_name(RmbCGen* g, const char* prefix, rmb_string name) {
    size_t plen = strlen(prefix);
    char* out = rmb_arena_alloc(g->arena, plen + 1 + name.len + 1);
    if (!out) return NULL;
    memcpy(out, prefix, plen);
    out[plen] = '_';
    for (size_t i = 0; i < name.len; i++) {
        char ch = name.ptr[i];
        out[plen + 1 + i] =
            (ch == '/' || ch == '\\' || ch == '.' || ch == '-') ? '_' : ch;
    }
    out[plen + 1 + name.len] = '\0';
    return out;
}

static bool str_ends_module(const char* full, const char* short_name) {
    size_t flen = strlen(full);
    size_t slen = strlen(short_name);
    if (flen == slen && memcmp(full, short_name, slen) == 0) return true;
    if (flen <= slen) return false;
    return full[flen - slen - 1] == '.' &&
           memcmp(full + flen - slen, short_name, slen) == 0;
}

static CgStruct* find_struct(RmbCGen* g, rmb_string name) {
    for (CgStruct* s = (CgStruct*)g->struct_table; s; s = s->next) {
        if (rmb_string_equal(s->name, name)) return s;
    }
    return NULL;
}

static CgFn* find_fn(RmbCGen* g, rmb_string name) {
    for (CgFn* f = (CgFn*)g->fn_table; f; f = f->next) {
        if (!f->is_external && rmb_string_equal(f->name, name)) return f;
    }
    return NULL;
}

static CgFn* find_qualified_fn(RmbCGen* g, const char* module_path, rmb_string name) {
    for (CgFn* f = (CgFn*)g->fn_table; f; f = f->next) {
        if (!f->module_path || !rmb_string_equal(f->name, name)) continue;
        if (str_ends_module(f->module_path, module_path)) return f;
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

static RmbType* resolve_type_ref(RmbCGen* g, RmbAstTypeRef* ref) {
    if (!ref) return rmb_type_unknown();
    switch (ref->kind) {
        case RMB_AST_TYPE_SIMPLE: {
            RmbType* prim = rmb_type_lookup_primitive(ref->simple.name);
            if (prim) return prim;
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
            return rmb_type_make_named(g->arena, (rmb_string){buf, total});
        }
        default:
            return rmb_type_unknown();
    }
}

static void emit_c_type(RmbCGen* g, RmbType* t) {
    if (!t) { emit_str(g, "int64_t"); return; }
    switch (t->kind) {
        case RMB_TYPE_VOID:  emit_str(g, "void"); break;
        case RMB_TYPE_INT:   emit_str(g, "int64_t"); break;
        case RMB_TYPE_UINT:  emit_str(g, "uint64_t"); break;
        case RMB_TYPE_FLOAT: emit_str(g, "double"); break;
        case RMB_TYPE_BYTE:  emit_str(g, "uint8_t"); break;
        case RMB_TYPE_BOOL:  emit_str(g, "bool"); break;
        case RMB_TYPE_STR:   emit_str(g, "RmStr"); break;
        case RMB_TYPE_ARGS:  emit_str(g, "RmArgs"); break;
        case RMB_TYPE_NAMED: {
            CgStruct* s = find_struct(g, t->name);
            if (s) emit_str(g, s->c_name);
            else emit_str(g, "int64_t");
            break;
        }
        case RMB_TYPE_POINTER:
            emit_c_type(g, t->inner);
            emit_str(g, "*");
            break;
        default:
            emit_str(g, "int64_t");
            break;
    }
}

static void collect_struct_from_item(RmbCGen* g, RmbAstItem* item, const char* module_prefix) {
    rmb_string name = item->struct_item.name;
    CgStruct* s = rmb_arena_alloc(g->arena, sizeof(CgStruct));
    s->name = name;
    s->c_name = arena_c_name(g, module_prefix, name);
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

static void collect_fn_from_item(RmbCGen* g, RmbAstItem* item, const char* module_path,
                                 const char* module_prefix, bool is_external) {
    rmb_string name = item->fn_item.name;
    CgFn* f = rmb_arena_alloc(g->arena, sizeof(CgFn));
    f->name = name;
    f->module_path = module_path;
    f->c_name = arena_c_name(g, module_prefix, name);
    f->params = NULL;
    f->param_count = 0;
    f->return_type = item->fn_item.return_type
        ? resolve_type_ref(g, item->fn_item.return_type)
        : NULL;
    f->error_capable = item->fn_item.error_type != NULL;
    f->is_main = !is_external && g->options.is_entry &&
        rmb_string_equal(name, rmb_string_from_cstr("main"));
    f->is_external = is_external;
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

static RmbType* expr_type(RmbCGen* g, RmbAstExpr* e);

static bool qualified_name_from_expr(RmbCGen* g, RmbAstExpr* e, char* module,
                                     size_t module_cap, rmb_string* name) {
    if (!e || e->kind != RMB_AST_EXPR_FIELD) return false;
    *name = e->field.field;
    module[0] = '\0';
    RmbAstExpr* cur = e->field.object;
    while (cur && cur->kind == RMB_AST_EXPR_FIELD) {
        char part[128];
        size_t n = cur->field.field.len < sizeof(part) - 1 ? cur->field.field.len : sizeof(part) - 1;
        memcpy(part, cur->field.field.ptr, n);
        part[n] = '\0';
        char old[256];
        snprintf(old, sizeof(old), "%s", module);
        snprintf(module, module_cap, "%s%s%s", part, old[0] ? "." : "", old);
        cur = cur->field.object;
    }
    if (!cur || cur->kind != RMB_AST_EXPR_IDENT) return false;
    char root[128];
    size_t n = cur->ident.name.len < sizeof(root) - 1 ? cur->ident.name.len : sizeof(root) - 1;
    memcpy(root, cur->ident.name.ptr, n);
    root[n] = '\0';
    char old[256];
    snprintf(old, sizeof(old), "%s", module);
    snprintf(module, module_cap, "%s%s%s", root, old[0] ? "." : "", old);
    (void)g;
    return true;
}

static RmbType* call_type(RmbCGen* g, RmbAstExpr* e) {
    RmbAstExpr* callee = e->call.callee;
    if (callee && callee->kind == RMB_AST_EXPR_IDENT) {
        rmb_string name = callee->ident.name;
        if (rmb_string_equal(name, rmb_string_from_cstr("print"))) return rmb_type_void();
        if (rmb_string_equal(name, rmb_string_from_cstr("str_len")) ||
            rmb_string_equal(name, rmb_string_from_cstr("str_byte")) ||
            rmb_string_equal(name, rmb_string_from_cstr("args_len"))) {
            return rmb_type_int();
        }
        if (rmb_string_equal(name, rmb_string_from_cstr("str_eq"))) return rmb_type_bool();
        if (rmb_string_equal(name, rmb_string_from_cstr("args_get"))) return rmb_type_str();
        CgFn* fn = find_fn(g, name);
        if (fn) return fn->return_type ? fn->return_type : rmb_type_void();
    }
    char module[256];
    rmb_string name;
    if (qualified_name_from_expr(g, callee, module, sizeof(module), &name)) {
        CgFn* fn = find_qualified_fn(g, module, name);
        if (fn) return fn->return_type ? fn->return_type : rmb_type_void();
    }
    return rmb_type_unknown();
}

static RmbType* expr_type(RmbCGen* g, RmbAstExpr* e) {
    if (!e) return rmb_type_unknown();
    switch (e->kind) {
        case RMB_AST_EXPR_INT: return rmb_type_int();
        case RMB_AST_EXPR_STRING: return rmb_type_str();
        case RMB_AST_EXPR_BOOL: return rmb_type_bool();
        case RMB_AST_EXPR_NONE: return rmb_type_make_optional(g->arena, rmb_type_unknown());
        case RMB_AST_EXPR_IDENT: {
            CgVar* v = find_var(g, e->ident.name);
            return v ? v->type : rmb_type_unknown();
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
                        if (rmb_string_equal(f->name, e->field.field)) return f->type;
                    }
                }
            }
            return rmb_type_unknown();
        }
        case RMB_AST_EXPR_UNARY: {
            RmbType* inner = expr_type(g, e->unary.operand);
            if (e->unary.op == RMB_TOKEN_BANG) return rmb_type_bool();
            if (e->unary.op == RMB_TOKEN_STAR && inner && inner->kind == RMB_TYPE_POINTER) return inner->inner;
            if (e->unary.op == RMB_TOKEN_AMP) return rmb_type_make_pointer(g->arena, inner);
            return inner ? inner : rmb_type_int();
        }
        case RMB_AST_EXPR_BINARY:
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
        case RMB_AST_EXPR_ERROR_PROP: return expr_type(g, e->error_prop.operand);
        case RMB_AST_EXPR_ERROR_PANIC: return expr_type(g, e->error_panic.operand);
        case RMB_AST_EXPR_ELSE: return expr_type(g, e->else_expr.value);
        default: return rmb_type_unknown();
    }
}

static void emit_indent(RmbCGen* g) {
    for (int i = 0; i < g->indent; i++) fputs("    ", g->out);
}

static void emit_expr(RmbCGen* g, RmbAstExpr* e);
static void emit_stmt(RmbCGen* g, RmbAstStmt* s);
static void emit_block(RmbCGen* g, RmbAstStmt** body, size_t count);

static void emit_print_call(RmbCGen* g, RmbAstExpr* e) {
    if (e->call.arg_count != 1) {
        cg_error(g, e->span, "print expects exactly 1 argument");
        emit_str(g, "0");
        return;
    }
    RmbAstExpr* arg = e->call.args[0];
    if (arg->kind == RMB_AST_EXPR_STRING) {
        emit_str(g, "rm_print(rm_str(");
        emit_rmb_string(g, arg->string_lit.value);
        emit_str(g, "))");
        return;
    }
    RmbType* t = expr_type(g, arg);
    if (!t || rmb_type_is_unknown(t)) {
        cg_error(g, e->span, "cannot determine type of print argument in codegen v0.0.7");
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
            cg_error(g, e->span, "print of this type is not supported by codegen v0.0.7");
            emit_str(g, "0");
            break;
    }
}

static const char* binary_op_str(RmbTokenKind op) {
    switch (op) {
        case RMB_TOKEN_PLUS: return "+";
        case RMB_TOKEN_MINUS: return "-";
        case RMB_TOKEN_STAR: return "*";
        case RMB_TOKEN_SLASH: return "/";
        case RMB_TOKEN_PERCENT: return "%";
        case RMB_TOKEN_LT: return "<";
        case RMB_TOKEN_GT: return ">";
        case RMB_TOKEN_LT_EQ: return "<=";
        case RMB_TOKEN_GT_EQ: return ">=";
        case RMB_TOKEN_EQ_EQ: return "==";
        case RMB_TOKEN_BANG_EQ: return "!=";
        case RMB_TOKEN_AND_AND: return "&&";
        case RMB_TOKEN_OR_OR: return "||";
        default: return "?";
    }
}

static const char* unary_op_str(RmbTokenKind op) {
    switch (op) {
        case RMB_TOKEN_MINUS: return "-";
        case RMB_TOKEN_BANG: return "!";
        case RMB_TOKEN_STAR: return "*";
        case RMB_TOKEN_AMP: return "&";
        default: return "?";
    }
}

static void emit_call_args(RmbCGen* g, RmbAstExpr* e) {
    emit_str(g, "(");
    for (size_t i = 0; i < e->call.arg_count; i++) {
        if (i > 0) emit_str(g, ", ");
        emit_expr(g, e->call.args[i]);
    }
    emit_str(g, ")");
}

static void emit_call(RmbCGen* g, RmbAstExpr* e) {
    RmbAstExpr* callee = e->call.callee;
    if (callee && callee->kind == RMB_AST_EXPR_IDENT) {
        rmb_string name = callee->ident.name;
        if (rmb_string_equal(name, rmb_string_from_cstr("print"))) {
            emit_print_call(g, e);
            return;
        }
        if (rmb_string_equal(name, rmb_string_from_cstr("str_len"))) {
            if (e->call.arg_count != 1) {
                cg_error(g, e->span, "str_len expects exactly 1 argument");
                emit_str(g, "0");
                return;
            }
            emit_str(g, "rm_str_len(");
            emit_expr(g, e->call.args[0]);
            emit_str(g, ")");
            return;
        }
        if (rmb_string_equal(name, rmb_string_from_cstr("str_byte"))) {
            if (e->call.arg_count != 2) {
                cg_error(g, e->span, "str_byte expects exactly 2 arguments");
                emit_str(g, "0");
                return;
            }
            emit_str(g, "rm_str_byte(");
            emit_expr(g, e->call.args[0]);
            emit_str(g, ", ");
            emit_expr(g, e->call.args[1]);
            emit_str(g, ")");
            return;
        }
        if (rmb_string_equal(name, rmb_string_from_cstr("str_eq"))) {
            if (e->call.arg_count != 2) {
                cg_error(g, e->span, "str_eq expects exactly 2 arguments");
                emit_str(g, "false");
                return;
            }
            emit_str(g, "rm_str_eq(");
            emit_expr(g, e->call.args[0]);
            emit_str(g, ", ");
            emit_expr(g, e->call.args[1]);
            emit_str(g, ")");
            return;
        }
        if (rmb_string_equal(name, rmb_string_from_cstr("args_len"))) {
            if (e->call.arg_count != 1) {
                cg_error(g, e->span, "args_len expects exactly 1 argument");
                emit_str(g, "0");
                return;
            }
            emit_str(g, "rm_args_len(");
            emit_expr(g, e->call.args[0]);
            emit_str(g, ")");
            return;
        }
        if (rmb_string_equal(name, rmb_string_from_cstr("args_get"))) {
            if (e->call.arg_count != 2) {
                cg_error(g, e->span, "args_get expects exactly 2 arguments");
                emit_str(g, "rm_str(\"\")");
                return;
            }
            emit_str(g, "rm_args_get(");
            emit_expr(g, e->call.args[0]);
            emit_str(g, ", ");
            emit_expr(g, e->call.args[1]);
            emit_str(g, ")");
            return;
        }
        CgFn* fn = find_fn(g, name);
        if (fn && fn->error_capable) {
            cg_error(g, e->span, "error-returning functions are not supported by codegen v0.0.7");
            emit_str(g, "0");
            return;
        }
        emit_str(g, fn ? fn->c_name : "rm_fn_unknown");
        emit_call_args(g, e);
        return;
    }
    char module[256];
    rmb_string name;
    if (qualified_name_from_expr(g, callee, module, sizeof(module), &name)) {
        CgFn* fn = find_qualified_fn(g, module, name);
        if (!fn) {
            cg_error(g, e->span, "unknown imported function '%s.%.*s'",
                     module, (int)name.len, name.ptr);
            emit_str(g, "0");
            return;
        }
        emit_str(g, fn->c_name);
        emit_call_args(g, e);
        return;
    }
    cg_error(g, e->span, "call target not supported by codegen v0.0.7");
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
            cg_error(g, e->span, "error propagation '?' is not supported by codegen v0.0.7");
            emit_str(g, "0");
            break;
        case RMB_AST_EXPR_ERROR_PANIC:
            cg_error(g, e->span, "error panic '!' is not supported by codegen v0.0.7");
            emit_str(g, "0");
            break;
        case RMB_AST_EXPR_ELSE:
            cg_error(g, e->span, "'else' fallback expression is not supported by codegen v0.0.7");
            emit_str(g, "0");
            break;
        case RMB_AST_EXPR_NONE:
            cg_error(g, e->span, "'none' literal is not supported by codegen v0.0.7");
            emit_str(g, "0");
            break;
        default:
            cg_error(g, e->span, "unsupported expression in codegen v0.0.7");
            emit_str(g, "0");
            break;
    }
}

static const char* assign_op_str(RmbTokenKind op) {
    switch (op) {
        case RMB_TOKEN_EQ: return "=";
        case RMB_TOKEN_PLUS_EQ: return "+=";
        case RMB_TOKEN_MINUS_EQ: return "-=";
        case RMB_TOKEN_STAR_EQ: return "*=";
        case RMB_TOKEN_SLASH_EQ: return "/=";
        default: return "=";
    }
}

static void emit_var_stmt(RmbCGen* g, RmbAstStmt* s) {
    RmbType* t = s->var.type ? resolve_type_ref(g, s->var.type) : expr_type(g, s->var.value);
    if (!t || rmb_type_is_unknown(t)) t = rmb_type_int();
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
        emit_str(g, (fn && fn->is_main) ? "return 0;\n" : "return;\n");
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
    if (s->for_stmt.cond) emit_expr(g, s->for_stmt.cond);
    else emit_str(g, "1");
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
        case RMB_AST_STMT_VAR: emit_var_stmt(g, s); break;
        case RMB_AST_STMT_ASSIGN: emit_assign_stmt(g, s); break;
        case RMB_AST_STMT_RETURN: emit_return_stmt(g, s); break;
        case RMB_AST_STMT_IF: emit_if_stmt(g, s); break;
        case RMB_AST_STMT_WHILE: emit_while_stmt(g, s); break;
        case RMB_AST_STMT_FOR: emit_for_stmt(g, s); break;
        case RMB_AST_STMT_MATCH:
            cg_error(g, s->span, "match is not supported by codegen v0.0.7");
            break;
        case RMB_AST_STMT_DEFER:
            cg_error(g, s->span, "defer is not supported by codegen v0.0.7");
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
    for (size_t i = 0; i < count; i++) emit_stmt(g, body[i]);
    scope_pop(g);
}

static void emit_struct_decl(RmbCGen* g, CgStruct* s) {
    emit_str(g, "typedef struct ");
    emit_str(g, s->c_name);
    emit_str(g, " {\n");
    for (CgField* f = s->fields; f; f = f->next) {
        emit_str(g, "    ");
        emit_c_type(g, f->type);
        emit_str(g, " ");
        emit_rmb_string(g, f->name);
        emit_str(g, ";\n");
    }
    emit_str(g, "} ");
    emit_str(g, s->c_name);
    emit_str(g, ";\n\n");
}

static void emit_fn_signature(RmbCGen* g, CgFn* fn, bool prototype) {
    if (fn->is_main) {
        if (fn->param_count == 1 && fn->params && fn->params->type &&
            fn->params->type->kind == RMB_TYPE_ARGS) {
            emit_str(g, "int main(int argc, char **argv)");
        } else {
            emit_str(g, "int main(void)");
        }
        return;
    }
    if (fn->return_type && fn->return_type->kind != RMB_TYPE_VOID) emit_c_type(g, fn->return_type);
    else emit_str(g, "void");
    emit_str(g, " ");
    emit_str(g, fn->c_name);
    emit_str(g, "(");
    if (!fn->params) {
        emit_str(g, "void");
    } else {
        bool first = true;
        for (CgFnParam* p = fn->params; p; p = p->next) {
            if (!first) emit_str(g, ", ");
            first = false;
            emit_c_type(g, p->type);
            if (!prototype) {
                emit_str(g, " ");
                emit_rmb_string(g, p->name);
            }
        }
    }
    emit_str(g, ")");
}

static void emit_fn_decl(RmbCGen* g, CgFn* fn) {
    emit_fn_signature(g, fn, true);
    emit_str(g, ";\n");
}

static bool body_ends_with_return(RmbAstItem* item) {
    if (item->fn_item.body_count == 0) return false;
    RmbAstStmt* last = item->fn_item.body[item->fn_item.body_count - 1];
    return last && last->kind == RMB_AST_STMT_RETURN;
}

static void emit_fn_def(RmbCGen* g, CgFn* fn) {
    if (fn->is_external) return;
    bool main_has_args = fn->is_main && fn->param_count == 1 && fn->params &&
        fn->params->type && fn->params->type->kind == RMB_TYPE_ARGS;
    if (fn->is_main && fn->params && !main_has_args) {
        cg_error(g, fn->ast->span, "main only supports zero parameters or one Args parameter");
    }
    if (fn->error_capable) {
        cg_error(g, fn->ast->span, "error-returning functions are not supported by codegen v0.0.7");
        return;
    }
    g->current_fn = fn;
    emit_fn_signature(g, fn, false);
    emit_str(g, " {\n");
    g->indent++;
    scope_push(g);
    if (main_has_args) {
        emit_indent(g);
        emit_str(g, "RmArgs ");
        emit_rmb_string(g, fn->params->name);
        emit_str(g, ";\n");
        emit_indent(g);
        emit_rmb_string(g, fn->params->name);
        emit_str(g, ".argc = argc;\n");
        emit_indent(g);
        emit_rmb_string(g, fn->params->name);
        emit_str(g, ".argv = argv;\n");
    }
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

static void emit_prelude(RmbCGen* g) {
    emit_str(g,
        "// Generated by rmb 0.0.7 - do not edit.\n"
        "#include <stdint.h>\n"
        "#include <stddef.h>\n"
        "#include <stdbool.h>\n"
        "#include <stdio.h>\n"
        "#include <string.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "#if defined(__GNUC__)\n"
        "#define RM_UNUSED __attribute__((unused))\n"
        "#else\n"
        "#define RM_UNUSED\n"
        "#endif\n"
        "\n"
        "typedef struct {\n"
        "    const char *ptr;\n"
        "    int64_t len;\n"
        "} RmStr;\n"
        "\n"
        "typedef struct {\n"
        "    int argc;\n"
        "    char **argv;\n"
        "} RmArgs;\n"
        "\n"
        "static RM_UNUSED RmStr rm_str(const char *s) {\n"
        "    RmStr out;\n"
        "    out.ptr = s;\n"
        "    out.len = (int64_t)strlen(s);\n"
        "    return out;\n"
        "}\n"
        "\n"
        "static RM_UNUSED int64_t rm_str_len(RmStr s) {\n"
        "    return s.len;\n"
        "}\n"
        "\n"
        "static RM_UNUSED int64_t rm_str_byte(RmStr s, int64_t index) {\n"
        "    return (int64_t)((unsigned char)s.ptr[index]);\n"
        "}\n"
        "\n"
        "static RM_UNUSED bool rm_str_eq(RmStr a, RmStr b) {\n"
        "    if (a.len != b.len) return false;\n"
        "    return memcmp(a.ptr, b.ptr, (size_t)a.len) == 0;\n"
        "}\n"
        "\n"
        "static RM_UNUSED int64_t rm_args_len(RmArgs args) {\n"
        "    return (int64_t)args.argc;\n"
        "}\n"
        "\n"
        "static RM_UNUSED RmStr rm_args_get(RmArgs args, int64_t index) {\n"
        "    return rm_str(args.argv[index]);\n"
        "}\n"
        "\n"
        "static RM_UNUSED void rm_print(RmStr s) {\n"
        "    fwrite(s.ptr, 1, (size_t)s.len, stdout);\n"
        "}\n"
        "\n"
        "static RM_UNUSED void rm_print_int(int64_t v) {\n"
        "    printf(\"%lld\", (long long)v);\n"
        "}\n"
        "\n"
        "static RM_UNUSED void rm_print_bool(bool b) {\n"
        "    if (b) fwrite(\"true\", 1, 4, stdout);\n"
        "    else fwrite(\"false\", 1, 5, stdout);\n"
        "}\n"
        "\n");
}

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
    memset(&g->options, 0, sizeof(g->options));
}

bool rmb_cgen_emit_file(RmbCGen* g, const char* out_path, RmbCGenOptions options) {
    g->options = options;

    for (size_t i = 0; i < options.external_struct_count; i++) {
        collect_struct_from_item(g, options.external_structs[i].item,
                                 options.external_structs[i].module_prefix);
    }
    for (RmbAstItem* it = g->file->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_STRUCT) collect_struct_from_item(g, it, options.module_prefix);
    }
    for (size_t i = 0; i < options.external_fn_count; i++) {
        collect_fn_from_item(g, options.external_fns[i].item,
                             options.external_fns[i].module_path,
                             options.external_fns[i].module_prefix,
                             true);
    }
    for (RmbAstItem* it = g->file->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_FN) {
            collect_fn_from_item(g, it, options.module_path, options.module_prefix, false);
        }
    }

    g->out = fopen(out_path, "wb");
    if (!g->out) {
        rmb_diag_error("failed to open output file: %s", out_path);
        return false;
    }

    emit_prelude(g);

    emit_str(g, "/* module ");
    emit_c_ident_part(g, options.module_path ? options.module_path : "");
    emit_str(g, " */\n\n");

    for (CgStruct* s = (CgStruct*)g->struct_table; s; s = s->next) {
        emit_struct_decl(g, s);
    }
    for (CgFn* fn = (CgFn*)g->fn_table; fn; fn = fn->next) {
        if (fn->is_main || fn->error_capable) continue;
        emit_fn_decl(g, fn);
    }
    emit_str(g, "\n");
    for (CgFn* fn = (CgFn*)g->fn_table; fn; fn = fn->next) {
        emit_fn_def(g, fn);
    }

    fclose(g->out);
    g->out = NULL;
    (void)emit_c_rmb_string_part;
    return !g->had_error;
}
