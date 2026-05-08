// RauMa Bootstrap Compiler - Type Checker
// v0.0.5: Basic type checking

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "rmb/checker.h"
#include "rmb/diag.h"
#include "rmb/token.h"

// ----- Internal symbol tables -----

typedef struct RmbVarSymbol RmbVarSymbol;
struct RmbVarSymbol {
    rmb_string name;
    RmbType* type;
    RmbVarSymbol* next;
};

typedef struct RmbScope RmbScope;
struct RmbScope {
    RmbVarSymbol* vars;
    RmbScope* parent;
};

typedef struct RmbFieldSymbol RmbFieldSymbol;
struct RmbFieldSymbol {
    rmb_string name;
    RmbType* type;
    RmbFieldSymbol* next;
};

typedef struct RmbStructSymbol RmbStructSymbol;
struct RmbStructSymbol {
    rmb_string name;
    RmbFieldSymbol* fields;
    RmbStructSymbol* next;
};

typedef struct RmbVariantSymbol RmbVariantSymbol;
struct RmbVariantSymbol {
    rmb_string name;
    RmbFieldSymbol* fields;
    RmbVariantSymbol* next;
};

typedef struct RmbEnumSymbol RmbEnumSymbol;
struct RmbEnumSymbol {
    rmb_string name;
    RmbVariantSymbol* variants;
    RmbEnumSymbol* next;
};

typedef struct RmbFnParamSymbol RmbFnParamSymbol;
struct RmbFnParamSymbol {
    rmb_string name;
    RmbType* type;
    RmbFnParamSymbol* next;
};

typedef struct RmbFnSymbol RmbFnSymbol;
struct RmbFnSymbol {
    rmb_string name;
    bool is_pub;
    RmbFnParamSymbol* params;
    size_t param_count;
    RmbType* return_type;
    bool error_capable;
    RmbType* error_type;
    RmbAstItem* ast;
    RmbFnSymbol* next;
};

// ----- Helpers -----

static void emit_error(RmbChecker* c, RmbSpan span, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    rmb_diag_error_at(span, "%s", buf);
    c->had_error = true;
}

static RmbStructSymbol* find_struct(RmbChecker* c, rmb_string name) {
    for (RmbStructSymbol* s = (RmbStructSymbol*)c->struct_symbols; s; s = s->next) {
        if (rmb_string_equal(s->name, name)) return s;
    }
    return NULL;
}

static RmbEnumSymbol* find_enum(RmbChecker* c, rmb_string name) {
    for (RmbEnumSymbol* e = (RmbEnumSymbol*)c->enum_symbols; e; e = e->next) {
        if (rmb_string_equal(e->name, name)) return e;
    }
    return NULL;
}

static RmbFnSymbol* find_fn(RmbChecker* c, rmb_string name) {
    for (RmbFnSymbol* f = (RmbFnSymbol*)c->fn_symbols; f; f = f->next) {
        if (rmb_string_equal(f->name, name)) return f;
    }
    return NULL;
}

static RmbVarSymbol* find_var(RmbChecker* c, rmb_string name) {
    for (RmbScope* s = (RmbScope*)c->current_scope; s; s = s->parent) {
        for (RmbVarSymbol* v = s->vars; v; v = v->next) {
            if (rmb_string_equal(v->name, name)) return v;
        }
    }
    return NULL;
}

static void scope_push(RmbChecker* c) {
    RmbScope* s = rmb_arena_alloc(c->arena, sizeof(RmbScope));
    s->vars = NULL;
    s->parent = (RmbScope*)c->current_scope;
    c->current_scope = s;
}

static void scope_pop(RmbChecker* c) {
    RmbScope* s = (RmbScope*)c->current_scope;
    if (s) c->current_scope = s->parent;
}

static void scope_add_var(RmbChecker* c, rmb_string name, RmbType* type) {
    RmbScope* s = (RmbScope*)c->current_scope;
    if (!s) return;
    RmbVarSymbol* v = rmb_arena_alloc(c->arena, sizeof(RmbVarSymbol));
    v->name = name;
    v->type = type;
    v->next = s->vars;
    s->vars = v;
}

// ----- Type resolution from AST type refs -----

static RmbType* resolve_type_ref(RmbChecker* c, RmbAstTypeRef* ref);

static RmbType* resolve_simple_type(RmbChecker* c, rmb_string name, RmbSpan span) {
    RmbType* prim = rmb_type_lookup_primitive(name);
    if (prim) return prim;

    if (find_struct(c, name) || find_enum(c, name)) {
        return rmb_type_make_named(c->arena, name);
    }

    // Allow but mark unknown for now (e.g., external types like HttpError)
    // We do not error on unresolved named types because they may come from
    // external modules that v0.0.5 does not load.
    (void)span;
    return rmb_type_make_named(c->arena, name);
}

static RmbType* resolve_type_ref(RmbChecker* c, RmbAstTypeRef* ref) {
    if (!ref) return rmb_type_unknown();
    switch (ref->kind) {
        case RMB_AST_TYPE_SIMPLE:
            return resolve_simple_type(c, ref->simple.name, ref->span);
        case RMB_AST_TYPE_POINTER:
            return rmb_type_make_pointer(c->arena, resolve_type_ref(c, ref->pointer.elem));
        case RMB_AST_TYPE_SLICE:
            return rmb_type_make_slice(c->arena, resolve_type_ref(c, ref->slice.elem));
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
            return rmb_type_make_array(c->arena, resolve_type_ref(c, ref->array.elem), len);
        }
        case RMB_AST_TYPE_OPTIONAL:
            return rmb_type_make_optional(c->arena, resolve_type_ref(c, ref->optional.elem));
        case RMB_AST_TYPE_QUALIFIED: {
            // Treat qualified as external named: module.name
            // Build a single label: module.name
            size_t total = ref->qualified.module.len + 1 + ref->qualified.name.len;
            char* buf = rmb_arena_alloc(c->arena, total + 1);
            memcpy(buf, ref->qualified.module.ptr, ref->qualified.module.len);
            buf[ref->qualified.module.len] = '.';
            memcpy(buf + ref->qualified.module.len + 1,
                   ref->qualified.name.ptr, ref->qualified.name.len);
            buf[total] = '\0';
            rmb_string name = {buf, total};
            return rmb_type_make_named(c->arena, name);
        }
        default:
            return rmb_type_unknown();
    }
}

// ----- Assignability -----

static bool is_assignable(RmbType* dst, RmbType* src) {
    if (!dst || !src) return true;
    if (rmb_type_is_unknown(dst) || rmb_type_is_unknown(src)) return true;
    if (rmb_type_equal(dst, src)) return true;

    // Allow T -> T?
    if (dst->kind == RMB_TYPE_OPTIONAL && rmb_type_equal(dst->inner, src)) return true;

    // Allow optional<X> -> optional<Y> if X assignable to Y (covers none -> T?)
    if (dst->kind == RMB_TYPE_OPTIONAL && src->kind == RMB_TYPE_OPTIONAL) {
        return is_assignable(dst->inner, src->inner);
    }

    return false;
}

// ----- Symbol collection -----

static RmbType* maybe_unknown(RmbType* t) { return t ? t : rmb_type_unknown(); }

static void collect_struct(RmbChecker* c, RmbAstItem* item) {
    rmb_string name = item->struct_item.name;
    if (find_struct(c, name) || find_enum(c, name) || find_fn(c, name)) {
        emit_error(c, item->span, "duplicate top-level name '%.*s'",
                   (int)name.len, name.ptr);
        return;
    }
    RmbStructSymbol* s = rmb_arena_alloc(c->arena, sizeof(RmbStructSymbol));
    s->name = name;
    s->fields = NULL;
    s->next = (RmbStructSymbol*)c->struct_symbols;
    c->struct_symbols = s;

    RmbFieldSymbol* last = NULL;
    for (RmbAstField* f = item->struct_item.fields; f; f = f->next) {
        // Duplicate field check
        for (RmbFieldSymbol* existing = s->fields; existing; existing = existing->next) {
            if (rmb_string_equal(existing->name, f->name)) {
                emit_error(c, f->span, "duplicate field '%.*s' in struct '%.*s'",
                           (int)f->name.len, f->name.ptr,
                           (int)name.len, name.ptr);
                break;
            }
        }
        RmbFieldSymbol* fs = rmb_arena_alloc(c->arena, sizeof(RmbFieldSymbol));
        fs->name = f->name;
        fs->type = maybe_unknown(resolve_type_ref(c, f->type));
        fs->next = NULL;
        if (!last) s->fields = fs; else last->next = fs;
        last = fs;
    }
}

static void collect_enum(RmbChecker* c, RmbAstItem* item) {
    rmb_string name = item->enum_item.name;
    if (find_struct(c, name) || find_enum(c, name) || find_fn(c, name)) {
        emit_error(c, item->span, "duplicate top-level name '%.*s'",
                   (int)name.len, name.ptr);
        return;
    }
    RmbEnumSymbol* e = rmb_arena_alloc(c->arena, sizeof(RmbEnumSymbol));
    e->name = name;
    e->variants = NULL;
    e->next = (RmbEnumSymbol*)c->enum_symbols;
    c->enum_symbols = e;

    RmbVariantSymbol* vlast = NULL;
    for (RmbAstVariant* v = item->enum_item.variants; v; v = v->next) {
        for (RmbVariantSymbol* existing = e->variants; existing; existing = existing->next) {
            if (rmb_string_equal(existing->name, v->name)) {
                emit_error(c, v->span, "duplicate variant '%.*s' in enum '%.*s'",
                           (int)v->name.len, v->name.ptr,
                           (int)name.len, name.ptr);
                break;
            }
        }
        RmbVariantSymbol* vs = rmb_arena_alloc(c->arena, sizeof(RmbVariantSymbol));
        vs->name = v->name;
        vs->fields = NULL;
        vs->next = NULL;

        RmbFieldSymbol* last = NULL;
        for (RmbAstVariantField* f = v->fields; f; f = f->next) {
            for (RmbFieldSymbol* existing = vs->fields; existing; existing = existing->next) {
                if (rmb_string_equal(existing->name, f->name)) {
                    emit_error(c, f->span, "duplicate field '%.*s' in variant '%.*s'",
                               (int)f->name.len, f->name.ptr,
                               (int)v->name.len, v->name.ptr);
                    break;
                }
            }
            RmbFieldSymbol* fs = rmb_arena_alloc(c->arena, sizeof(RmbFieldSymbol));
            fs->name = f->name;
            fs->type = maybe_unknown(resolve_type_ref(c, f->type));
            fs->next = NULL;
            if (!last) vs->fields = fs; else last->next = fs;
            last = fs;
        }

        if (!vlast) e->variants = vs; else vlast->next = vs;
        vlast = vs;
    }
}

static void collect_fn(RmbChecker* c, RmbAstItem* item) {
    rmb_string name = item->fn_item.name;
    if (find_struct(c, name) || find_enum(c, name) || find_fn(c, name)) {
        emit_error(c, item->span, "duplicate top-level name '%.*s'",
                   (int)name.len, name.ptr);
        return;
    }
    RmbFnSymbol* f = rmb_arena_alloc(c->arena, sizeof(RmbFnSymbol));
    f->name = name;
    f->is_pub = item->fn_item.is_pub;
    f->params = NULL;
    f->param_count = 0;
    f->return_type = item->fn_item.return_type
        ? resolve_type_ref(c, item->fn_item.return_type)
        : NULL;
    f->error_capable = item->fn_item.error_type != NULL;
    f->error_type = item->fn_item.error_type
        ? resolve_type_ref(c, item->fn_item.error_type)
        : NULL;
    f->ast = item;
    f->next = (RmbFnSymbol*)c->fn_symbols;
    c->fn_symbols = f;

    RmbFnParamSymbol* last = NULL;
    for (RmbAstParam* p = item->fn_item.params; p; p = p->next) {
        RmbFnParamSymbol* ps = rmb_arena_alloc(c->arena, sizeof(RmbFnParamSymbol));
        ps->name = p->name;
        if (p->type) {
            ps->type = resolve_type_ref(c, p->type);
        } else {
            if (f->is_pub) {
                emit_error(c, p->span,
                    "parameter '%.*s' of public function must have explicit type",
                    (int)p->name.len, p->name.ptr);
            }
            ps->type = rmb_type_unknown();
        }
        ps->next = NULL;
        if (!last) f->params = ps; else last->next = ps;
        last = ps;
        f->param_count++;
    }
}

// ----- Expression checking -----

typedef struct {
    RmbType* type;
    bool error_capable;   // expression yields an error-capable value (e.g., call to fn !! E)
    RmbType* error_type;  // populated if error_capable
} ExprResult;

static ExprResult err_unknown(void) {
    ExprResult r = { rmb_type_unknown(), false, NULL };
    return r;
}

static ExprResult err_ty(RmbType* t) {
    ExprResult r = { t, false, NULL };
    return r;
}

static ExprResult check_expr(RmbChecker* c, RmbAstExpr* expr);

static ExprResult check_call(RmbChecker* c, RmbAstExpr* expr) {
    RmbAstExpr* callee = expr->call.callee;
    // Identifier callee -> function lookup
    if (callee && callee->kind == RMB_AST_EXPR_IDENT) {
        rmb_string name = callee->ident.name;
        RmbFnSymbol* fn = find_fn(c, name);
        // Check args first regardless
        for (size_t i = 0; i < expr->call.arg_count; i++) {
            check_expr(c, expr->call.args[i]);
        }
        if (!fn) {
            // Tolerated if name is a known external (we don't have those yet);
            // tolerate built-ins like print as well.
            if (rmb_string_equal(name, rmb_string_from_cstr("print"))) {
                ExprResult r = err_ty(rmb_type_void());
                return r;
            }
            // Allow: result is unknown
            return err_unknown();
        }
        if (expr->call.arg_count != fn->param_count) {
            emit_error(c, expr->span,
                "function '%.*s' expects %zu arguments, got %zu",
                (int)name.len, name.ptr, fn->param_count, expr->call.arg_count);
        } else {
            RmbFnParamSymbol* p = fn->params;
            for (size_t i = 0; i < expr->call.arg_count && p; i++, p = p->next) {
                ExprResult arg = check_expr(c, expr->call.args[i]);
                if (!is_assignable(p->type, arg.type)) {
                    emit_error(c, expr->call.args[i]->span,
                        "argument %zu type mismatch in call to '%.*s'",
                        i + 1, (int)name.len, name.ptr);
                }
            }
        }
        ExprResult r;
        r.type = fn->return_type ? fn->return_type : rmb_type_void();
        r.error_capable = fn->error_capable;
        r.error_type = fn->error_type;
        return r;
    }
    // Other callees: just check args and yield unknown
    for (size_t i = 0; i < expr->call.arg_count; i++) {
        check_expr(c, expr->call.args[i]);
    }
    if (callee) check_expr(c, callee);
    return err_unknown();
}

static ExprResult check_field(RmbChecker* c, RmbAstExpr* expr) {
    ExprResult base = check_expr(c, expr->field.object);
    RmbType* bt = base.type;
    if (bt && bt->kind == RMB_TYPE_POINTER) bt = bt->inner;
    if (bt && bt->kind == RMB_TYPE_NAMED) {
        RmbStructSymbol* s = find_struct(c, bt->name);
        if (s) {
            for (RmbFieldSymbol* f = s->fields; f; f = f->next) {
                if (rmb_string_equal(f->name, expr->field.field)) {
                    return err_ty(f->type);
                }
            }
            emit_error(c, expr->span, "field '%.*s' does not exist on struct '%.*s'",
                (int)expr->field.field.len, expr->field.field.ptr,
                (int)bt->name.len, bt->name.ptr);
            return err_unknown();
        }
    }
    return err_unknown();
}

static ExprResult check_unary(RmbChecker* c, RmbAstExpr* expr) {
    ExprResult inner = check_expr(c, expr->unary.operand);
    switch (expr->unary.op) {
        case RMB_TOKEN_BANG:
            if (!rmb_type_is_unknown(inner.type) && inner.type->kind != RMB_TYPE_BOOL) {
                emit_error(c, expr->span, "operator '!' requires bool operand");
            }
            return err_ty(rmb_type_bool());
        case RMB_TOKEN_MINUS:
            if (!rmb_type_is_unknown(inner.type) && !rmb_type_is_numeric(inner.type)) {
                emit_error(c, expr->span, "operator '-' requires numeric operand");
            }
            return err_ty(inner.type ? inner.type : rmb_type_int());
        case RMB_TOKEN_STAR:
            if (inner.type && inner.type->kind == RMB_TYPE_POINTER) {
                return err_ty(inner.type->inner);
            }
            return err_unknown();
        case RMB_TOKEN_AMP:
            return err_ty(rmb_type_make_pointer(c->arena, inner.type));
        default:
            return inner;
    }
}

static ExprResult check_binary(RmbChecker* c, RmbAstExpr* expr) {
    ExprResult left = check_expr(c, expr->binary.left);
    ExprResult right = check_expr(c, expr->binary.right);
    RmbTokenKind op = expr->binary.op;

    switch (op) {
        case RMB_TOKEN_PLUS:
        case RMB_TOKEN_MINUS:
        case RMB_TOKEN_STAR:
        case RMB_TOKEN_SLASH:
        case RMB_TOKEN_PERCENT: {
            // Allow int+int, str+str (concat) only if both sides are str.
            bool l_str = left.type && left.type->kind == RMB_TYPE_STR;
            bool r_str = right.type && right.type->kind == RMB_TYPE_STR;
            if (op == RMB_TOKEN_PLUS && l_str && r_str) {
                return err_ty(rmb_type_str());
            }
            if (!rmb_type_is_unknown(left.type) && !rmb_type_is_numeric(left.type)) {
                emit_error(c, expr->span, "left operand of arithmetic must be numeric");
            }
            if (!rmb_type_is_unknown(right.type) && !rmb_type_is_numeric(right.type)) {
                emit_error(c, expr->span, "right operand of arithmetic must be numeric");
            }
            // Result is left type if known else right type
            RmbType* r = !rmb_type_is_unknown(left.type) ? left.type : right.type;
            return err_ty(r ? r : rmb_type_int());
        }
        case RMB_TOKEN_LT:
        case RMB_TOKEN_GT:
        case RMB_TOKEN_LT_EQ:
        case RMB_TOKEN_GT_EQ:
            if (!rmb_type_is_unknown(left.type) && !rmb_type_is_numeric(left.type) &&
                left.type->kind != RMB_TYPE_STR) {
                emit_error(c, expr->span, "comparison requires numeric or string operands");
            }
            return err_ty(rmb_type_bool());
        case RMB_TOKEN_EQ_EQ:
        case RMB_TOKEN_BANG_EQ:
            return err_ty(rmb_type_bool());
        case RMB_TOKEN_AND_AND:
        case RMB_TOKEN_OR_OR:
            if (!rmb_type_is_unknown(left.type) && left.type->kind != RMB_TYPE_BOOL) {
                emit_error(c, expr->span, "logical operator requires bool operands");
            }
            if (!rmb_type_is_unknown(right.type) && right.type->kind != RMB_TYPE_BOOL) {
                emit_error(c, expr->span, "logical operator requires bool operands");
            }
            return err_ty(rmb_type_bool());
        default:
            return err_unknown();
    }
}

static ExprResult check_error_prop(RmbChecker* c, RmbAstExpr* expr) {
    ExprResult inner = check_expr(c, expr->error_prop.operand);
    RmbFnSymbol* current = (RmbFnSymbol*)c->current_fn;
    if (!inner.error_capable && !rmb_type_is_unknown(inner.type)) {
        emit_error(c, expr->span, "cannot use '?' on non-error value");
    }
    if (current && !current->error_capable && inner.error_capable) {
        emit_error(c, expr->span,
            "cannot use '?' in function that does not return an error");
    }
    return err_ty(inner.type);
}

static ExprResult check_error_panic(RmbChecker* c, RmbAstExpr* expr) {
    ExprResult inner = check_expr(c, expr->error_panic.operand);
    if (rmb_type_is_unknown(inner.type)) return err_unknown();
    if (inner.error_capable) {
        return err_ty(inner.type);
    }
    if (rmb_type_is_optional(inner.type)) {
        return err_ty(inner.type->inner);
    }
    emit_error(c, expr->span, "cannot use '!' on non-error/non-optional value");
    return err_ty(inner.type);
}

static ExprResult check_else_expr(RmbChecker* c, RmbAstExpr* expr) {
    ExprResult val = check_expr(c, expr->else_expr.value);
    ExprResult fb = check_expr(c, expr->else_expr.fallback);
    if (rmb_type_is_optional(val.type)) {
        RmbType* inner = val.type->inner;
        if (!is_assignable(inner, fb.type)) {
            emit_error(c, expr->span, "fallback type does not match optional inner type");
        }
        return err_ty(inner);
    }
    if (val.error_capable) {
        return err_ty(val.type);
    }
    if (rmb_type_is_unknown(val.type)) return err_unknown();
    emit_error(c, expr->span, "'else' fallback requires optional or error-capable value");
    return err_ty(val.type);
}

static ExprResult check_expr(RmbChecker* c, RmbAstExpr* expr) {
    if (!expr) return err_unknown();
    switch (expr->kind) {
        case RMB_AST_EXPR_INT:
            return err_ty(rmb_type_int());
        case RMB_AST_EXPR_STRING:
            return err_ty(rmb_type_str());
        case RMB_AST_EXPR_BOOL:
            return err_ty(rmb_type_bool());
        case RMB_AST_EXPR_NONE:
            // Has unknown inner; assignable to any optional.
            return err_ty(rmb_type_make_optional(c->arena, rmb_type_unknown()));
        case RMB_AST_EXPR_IDENT: {
            rmb_string name = expr->ident.name;
            RmbVarSymbol* v = find_var(c, name);
            if (v) return err_ty(v->type);
            RmbFnSymbol* fn = find_fn(c, name);
            if (fn) {
                // function reference; treat as unknown for now
                return err_unknown();
            }
            // Allow built-in 'print' identifier silently by returning unknown.
            if (rmb_string_equal(name, rmb_string_from_cstr("print"))) {
                return err_unknown();
            }
            emit_error(c, expr->span, "unknown variable '%.*s'",
                       (int)name.len, name.ptr);
            return err_unknown();
        }
        case RMB_AST_EXPR_CALL:
            return check_call(c, expr);
        case RMB_AST_EXPR_FIELD:
            return check_field(c, expr);
        case RMB_AST_EXPR_UNARY:
            return check_unary(c, expr);
        case RMB_AST_EXPR_BINARY:
            return check_binary(c, expr);
        case RMB_AST_EXPR_ERROR_PROP:
            return check_error_prop(c, expr);
        case RMB_AST_EXPR_ERROR_PANIC:
            return check_error_panic(c, expr);
        case RMB_AST_EXPR_ELSE:
            return check_else_expr(c, expr);
        default:
            return err_unknown();
    }
}

// ----- Statement checking -----

static void check_stmt(RmbChecker* c, RmbAstStmt* stmt);

static void check_block(RmbChecker* c, RmbAstStmt** body, size_t count) {
    scope_push(c);
    for (size_t i = 0; i < count; i++) {
        check_stmt(c, body[i]);
    }
    scope_pop(c);
}

static void check_var_stmt(RmbChecker* c, RmbAstStmt* stmt) {
    ExprResult val = check_expr(c, stmt->var.value);
    RmbType* t;
    if (stmt->var.type) {
        t = resolve_type_ref(c, stmt->var.type);
        if (stmt->var.value) {
            // Special case: none -> needs optional
            if (stmt->var.value->kind == RMB_AST_EXPR_NONE &&
                t->kind != RMB_TYPE_OPTIONAL) {
                emit_error(c, stmt->span,
                    "cannot assign 'none' to non-optional type");
            } else if (!is_assignable(t, val.type)) {
                emit_error(c, stmt->span, "type mismatch in variable declaration");
            }
        }
    } else {
        t = val.type ? val.type : rmb_type_unknown();
    }
    scope_add_var(c, stmt->var.name, t);
}

static void check_assign_stmt(RmbChecker* c, RmbAstStmt* stmt) {
    ExprResult target = check_expr(c, stmt->assign.target);
    ExprResult value = check_expr(c, stmt->assign.value);
    if (stmt->assign.op == RMB_TOKEN_EQ) {
        if (!is_assignable(target.type, value.type)) {
            emit_error(c, stmt->span, "type mismatch in assignment");
        }
    } else {
        // compound: numeric only
        if (!rmb_type_is_unknown(target.type) && !rmb_type_is_numeric(target.type)) {
            emit_error(c, stmt->span, "compound assignment requires numeric target");
        }
    }
}

static void check_return_stmt(RmbChecker* c, RmbAstStmt* stmt) {
    RmbFnSymbol* fn = (RmbFnSymbol*)c->current_fn;
    if (!fn) return;
    if (!stmt->ret.value) {
        if (fn->return_type && fn->return_type->kind != RMB_TYPE_VOID) {
            emit_error(c, stmt->span, "return without value in non-void function");
        } else if (!fn->return_type) {
            fn->return_type = rmb_type_void();
        }
        return;
    }
    ExprResult val = check_expr(c, stmt->ret.value);
    if (fn->return_type) {
        if (!is_assignable(fn->return_type, val.type)) {
            emit_error(c, stmt->span, "return type mismatch");
        }
    } else {
        // Infer
        fn->return_type = val.type ? val.type : rmb_type_unknown();
    }
}

static void check_match_stmt(RmbChecker* c, RmbAstStmt* stmt) {
    check_expr(c, stmt->match_stmt.value);
    for (RmbAstMatchCase* mc = stmt->match_stmt.cases; mc; mc = mc->next) {
        scope_push(c);
        for (RmbAstMatchCaseBinding* b = mc->bindings; b; b = b->next) {
            scope_add_var(c, b->name, rmb_type_unknown());
        }
        for (size_t i = 0; i < mc->body_count; i++) {
            check_stmt(c, mc->body[i]);
        }
        scope_pop(c);
    }
}

static void check_stmt(RmbChecker* c, RmbAstStmt* stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
        case RMB_AST_STMT_VAR:
            check_var_stmt(c, stmt);
            break;
        case RMB_AST_STMT_ASSIGN:
            check_assign_stmt(c, stmt);
            break;
        case RMB_AST_STMT_RETURN:
            check_return_stmt(c, stmt);
            break;
        case RMB_AST_STMT_IF: {
            ExprResult cond = check_expr(c, stmt->if_stmt.cond);
            if (!rmb_type_is_unknown(cond.type) && cond.type->kind != RMB_TYPE_BOOL) {
                emit_error(c, stmt->span, "if condition must be bool");
            }
            check_block(c, stmt->if_stmt.then_body, stmt->if_stmt.then_count);
            check_block(c, stmt->if_stmt.else_body, stmt->if_stmt.else_count);
            break;
        }
        case RMB_AST_STMT_WHILE: {
            ExprResult cond = check_expr(c, stmt->while_stmt.cond);
            if (!rmb_type_is_unknown(cond.type) && cond.type->kind != RMB_TYPE_BOOL) {
                emit_error(c, stmt->span, "while condition must be bool");
            }
            check_block(c, stmt->while_stmt.body, stmt->while_stmt.body_count);
            break;
        }
        case RMB_AST_STMT_FOR: {
            scope_push(c);
            if (stmt->for_stmt.init) check_stmt(c, stmt->for_stmt.init);
            if (stmt->for_stmt.cond) check_expr(c, stmt->for_stmt.cond);
            if (stmt->for_stmt.step) check_expr(c, stmt->for_stmt.step);
            for (size_t i = 0; i < stmt->for_stmt.body_count; i++) {
                check_stmt(c, stmt->for_stmt.body[i]);
            }
            scope_pop(c);
            break;
        }
        case RMB_AST_STMT_MATCH:
            check_match_stmt(c, stmt);
            break;
        case RMB_AST_STMT_DEFER:
            check_expr(c, stmt->defer_stmt.expr);
            break;
        case RMB_AST_STMT_EXPR:
            check_expr(c, stmt->expr_stmt.expr);
            break;
        default:
            break;
    }
}

// ----- Function body checking -----

static void check_fn_body(RmbChecker* c, RmbFnSymbol* fn) {
    c->current_fn = fn;
    scope_push(c);
    for (RmbFnParamSymbol* p = fn->params; p; p = p->next) {
        scope_add_var(c, p->name, p->type);
    }
    for (size_t i = 0; i < fn->ast->fn_item.body_count; i++) {
        check_stmt(c, fn->ast->fn_item.body[i]);
    }
    scope_pop(c);
    if (!fn->return_type) fn->return_type = rmb_type_void();
    c->current_fn = NULL;
}

// ----- Public API -----

void rmb_checker_init(RmbChecker* checker, rmb_arena* arena, RmbAstFile* file) {
    checker->arena = arena;
    checker->file = file;
    checker->had_error = false;
    checker->fn_symbols = NULL;
    checker->struct_symbols = NULL;
    checker->enum_symbols = NULL;
    checker->current_fn = NULL;
    checker->current_scope = NULL;
}

bool rmb_check_file(RmbChecker* c) {
    // Pass 1: collect top-level symbols (structs/enums first so fns can reference them)
    for (RmbAstItem* it = c->file->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_STRUCT) collect_struct(c, it);
        else if (it->kind == RMB_AST_ITEM_ENUM) collect_enum(c, it);
    }
    for (RmbAstItem* it = c->file->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_FN) collect_fn(c, it);
    }

    // Pass 2: check function bodies
    for (RmbFnSymbol* fn = (RmbFnSymbol*)c->fn_symbols; fn; fn = fn->next) {
        check_fn_body(c, fn);
    }

    return !c->had_error;
}
