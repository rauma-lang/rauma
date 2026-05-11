// RauMa Bootstrap Compiler - Chunk Build Driver
// v0.0.7: File/chunk build layout for small local projects.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "rmb/build.h"
#include "rmb/arena.h"
#include "rmb/ast.h"
#include "rmb/cgen.h"
#include "rmb/checker.h"
#include "rmb/diag.h"
#include "rmb/lexer.h"
#include "rmb/parser.h"
#include "rmb/source.h"

typedef struct BuildChunk BuildChunk;
struct BuildChunk {
    char* path;
    char* dir;
    char* stem;
    char* module_path;
    char* module_prefix;
    char* c_path;
    char* o_path;
    char* rmi_path;
    int state;
    rmb_arena* arena;
    RmbSource source;
    RmbAstFile* ast;
};

typedef struct BuildContext {
    char* entry_path;
    char* entry_dir;
    char* entry_stem;
    BuildChunk* chunks;
    size_t count;
    size_t cap;
    BuildChunk** order;
    size_t order_count;
    size_t order_cap;
} BuildContext;

static char* xstrdup(const char* s) {
    size_t n = strlen(s);
    char* out = malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

static char* normalize_path(const char* path) {
    char* out = xstrdup(path);
    if (!out) return NULL;
    for (char* p = out; *p; p++) {
        if (*p == '\\') *p = '/';
    }
    while (out[0] == '.' && out[1] == '/') {
        memmove(out, out + 2, strlen(out + 2) + 1);
    }
    return out;
}

static char* path_dir(const char* path) {
    const char* last = strrchr(path, '/');
    if (!last) return xstrdup(".");
    size_t n = (size_t)(last - path);
    char* out = malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, path, n);
    out[n] = '\0';
    return out;
}

static char* path_stem(const char* path) {
    const char* base = strrchr(path, '/');
    base = base ? base + 1 : path;
    const char* dot = strrchr(base, '.');
    size_t n = dot ? (size_t)(dot - base) : strlen(base);
    char* out = malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, base, n);
    out[n] = '\0';
    return out;
}

static char* module_from_source_path(const char* path) {
    while (path[0] == '.' && path[1] == '.' && path[2] == '/') {
        path += 3;
    }
    char* out = xstrdup(path);
    if (!out) return NULL;
    char* dot = strrchr(out, '.');
    if (dot && strcmp(dot, ".rm") == 0) *dot = '\0';
    for (char* p = out; *p; p++) {
        if (*p == '/' || *p == '\\') *p = '.';
    }
    return out;
}

static const char* artifact_source_dir(const char* dir) {
    while (dir[0] == '.' && dir[1] == '.' && dir[2] == '/') {
        dir += 3;
    }
    return dir;
}

static char* prefix_from_module(const char* module) {
    size_t n = strlen(module);
    char* out = malloc(n + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < n; i++) {
        char ch = module[i];
        out[i] = (ch == '.' || ch == '/' || ch == '\\' || ch == '-') ? '_' : ch;
    }
    out[n] = '\0';
    return out;
}

static char* resolve_use_path(BuildContext* ctx, rmb_string use_path) {
    size_t base_len = strlen(ctx->entry_dir);
    size_t total = base_len + 1 + use_path.len + 3;
    char* out = malloc(total + 1);
    if (!out) return NULL;
    memcpy(out, ctx->entry_dir, base_len);
    out[base_len] = '/';
    for (size_t i = 0; i < use_path.len; i++) {
        out[base_len + 1 + i] = use_path.ptr[i] == '.' ? '/' : use_path.ptr[i];
    }
    memcpy(out + base_len + 1 + use_path.len, ".rm", 4);
    return out;
}

static bool file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

static bool mkdir_one(const char* path) {
#ifdef _WIN32
    if (_mkdir(path) == 0) return true;
#else
    if (mkdir(path, 0755) == 0) return true;
#endif
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static bool mkdir_p(const char* path) {
    char tmp[1024];
    size_t n = strlen(path);
    if (n >= sizeof(tmp)) return false;
    memcpy(tmp, path, n + 1);
    for (size_t i = 1; i < n; i++) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char save = tmp[i];
            tmp[i] = '\0';
            if (tmp[0] != '\0' && !mkdir_one(tmp)) return false;
            tmp[i] = save;
        }
    }
    return mkdir_one(tmp);
}

static bool ensure_parent_dir(const char* path) {
    char* dir = path_dir(path);
    if (!dir) return false;
    bool ok = mkdir_p(dir);
    free(dir);
    return ok;
}

static int find_chunk(BuildContext* ctx, const char* path) {
    for (size_t i = 0; i < ctx->count; i++) {
        if (strcmp(ctx->chunks[i].path, path) == 0) return (int)i;
    }
    return -1;
}

static BuildChunk* add_chunk(BuildContext* ctx, const char* path) {
    int existing = find_chunk(ctx, path);
    if (existing >= 0) return &ctx->chunks[existing];
    if (ctx->count >= ctx->cap) {
        size_t cap = ctx->cap ? ctx->cap * 2 : 32;
        BuildChunk* chunks = realloc(ctx->chunks, cap * sizeof(BuildChunk));
        if (!chunks) return NULL;
        ctx->chunks = chunks;
        ctx->cap = cap;
    }
    BuildChunk* c = &ctx->chunks[ctx->count++];
    memset(c, 0, sizeof(*c));
    c->path = xstrdup(path);
    c->dir = path_dir(path);
    c->stem = path_stem(path);
    c->module_path = module_from_source_path(path);
    c->module_prefix = prefix_from_module(c->module_path);
    if (!c->path || !c->dir || !c->stem || !c->module_path || !c->module_prefix) return NULL;
    return c;
}

static bool order_push(BuildContext* ctx, BuildChunk* chunk) {
    if (ctx->order_count >= ctx->order_cap) {
        size_t cap = ctx->order_cap ? ctx->order_cap * 2 : 8;
        BuildChunk** order = realloc(ctx->order, cap * sizeof(BuildChunk*));
        if (!order) return false;
        ctx->order = order;
        ctx->order_cap = cap;
    }
    ctx->order[ctx->order_count++] = chunk;
    return true;
}

static bool parse_chunk(BuildChunk* chunk) {
    if (chunk->ast) return true;
    if (!rmb_source_read(chunk->path, &chunk->source)) return false;
    chunk->arena = rmb_arena_create(4 * 1024 * 1024);
    if (!chunk->arena) {
        rmb_diag_error("failed to create memory arena");
        return false;
    }
    rmb_ast_init(chunk->arena);
    RmbToken* tokens = NULL;
    size_t token_count = 0;
    bool had_lex_error = false;
    if (!rmb_lex_all(&chunk->source, chunk->arena, &tokens, &token_count, &had_lex_error)) {
        return false;
    }
    if (had_lex_error) return false;
    RmbParser parser;
    rmb_parser_init(&parser, tokens, token_count, chunk->arena);
    chunk->ast = rmb_parse_file(&parser, chunk->path);
    return chunk->ast && !rmb_parser_had_error(&parser);
}

static bool visit_chunk(BuildContext* ctx, BuildChunk* chunk) {
    if (chunk->state == 2) return true;
    if (chunk->state == 1) {
        rmb_diag_error("cyclic use dependency involving %s", chunk->path);
        return false;
    }
    chunk->state = 1;
    if (!parse_chunk(chunk)) return false;
    for (RmbAstItem* it = chunk->ast->items; it; it = it->next) {
        if (it->kind != RMB_AST_ITEM_USE) continue;
        char* dep_path = resolve_use_path(ctx, it->use_item.path);
        if (!dep_path) return false;
        if (!file_exists(dep_path)) {
            rmb_diag_error_at(it->span, "unresolved use target: %s", dep_path);
            free(dep_path);
            return false;
        }
        BuildChunk* dep = add_chunk(ctx, dep_path);
        free(dep_path);
        if (!dep || !visit_chunk(ctx, dep)) return false;
    }
    chunk->state = 2;
    return order_push(ctx, chunk);
}

static bool precheck_codegen_scope(BuildChunk* chunk) {
    bool ok = true;
    for (RmbAstItem* it = chunk->ast->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_FN && it->fn_item.error_type != NULL) {
            rmb_diag_error_at(it->span,
                "error-returning functions are not supported by codegen v0.0.7");
            ok = false;
        }
    }
    return ok;
}

static bool check_chunk(BuildChunk* chunk) {
    if (!precheck_codegen_scope(chunk)) return false;
    RmbChecker checker;
    rmb_checker_init(&checker, chunk->arena, chunk->ast);
    return rmb_check_file(&checker);
}

static char* chunk_artifact_path(BuildChunk* chunk, const char* ext) {
    const char* root = "build/debug/native/c/chunk";
    const char* dir = artifact_source_dir(chunk->dir);
    size_t n = strlen(root) + 1 + strlen(dir) + 1 + strlen(chunk->stem) +
               1 + strlen(chunk->stem) + strlen(ext) + 1;
    char* out = malloc(n);
    if (!out) return NULL;
    snprintf(out, n, "%s/%s/%s/%s%s", root, dir, chunk->stem, chunk->stem, ext);
    return out;
}

static char* bin_path(BuildContext* ctx) {
    size_t n = strlen("build/debug/native/bin/") + strlen(ctx->entry_stem) + 1;
    char* out = malloc(n);
    if (!out) return NULL;
    snprintf(out, n, "build/debug/native/bin/%s", ctx->entry_stem);
    return out;
}

static bool emit_rmi(BuildChunk* chunk) {
    FILE* f = fopen(chunk->rmi_path, "wb");
    if (!f) {
        rmb_diag_error("failed to open interface file: %s", chunk->rmi_path);
        return false;
    }
    fprintf(f, "module %s\n", chunk->module_path);
    for (RmbAstItem* it = chunk->ast->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_FN && it->fn_item.is_pub) {
            fprintf(f, "fn %.*s(", (int)it->fn_item.name.len, it->fn_item.name.ptr);
            bool first = true;
            for (RmbAstParam* p = it->fn_item.params; p; p = p->next) {
                if (!first) fprintf(f, ", ");
                first = false;
                fprintf(f, "%.*s", (int)p->name.len, p->name.ptr);
            }
            fprintf(f, ")\n");
        } else if (it->kind == RMB_AST_ITEM_STRUCT && it->struct_item.is_pub) {
            size_t fields = 0;
            for (RmbAstField* field = it->struct_item.fields; field; field = field->next) fields++;
            fprintf(f, "struct %.*s fields=%zu\n",
                    (int)it->struct_item.name.len, it->struct_item.name.ptr, fields);
        }
    }
    fclose(f);
    return true;
}

static size_t count_external_fns(BuildContext* ctx, BuildChunk* current) {
    size_t n = 0;
    for (size_t i = 0; i < ctx->order_count; i++) {
        BuildChunk* c = ctx->order[i];
        if (c == current) continue;
        for (RmbAstItem* it = c->ast->items; it; it = it->next) {
            if (it->kind == RMB_AST_ITEM_FN) n++;
        }
    }
    return n;
}

static size_t count_external_structs(BuildContext* ctx, BuildChunk* current) {
    size_t n = 0;
    for (size_t i = 0; i < ctx->order_count; i++) {
        BuildChunk* c = ctx->order[i];
        if (c == current) continue;
        for (RmbAstItem* it = c->ast->items; it; it = it->next) {
            if (it->kind == RMB_AST_ITEM_STRUCT) n++;
        }
    }
    return n;
}

static void fill_cgen_options(BuildContext* ctx, BuildChunk* chunk, RmbCGenOptions* options) {
    memset(options, 0, sizeof(*options));
    options->source_path = chunk->path;
    options->module_path = chunk->module_path;
    options->module_prefix = chunk->module_prefix;
    options->is_entry = strcmp(chunk->path, ctx->entry_path) == 0;
    options->external_fn_count = count_external_fns(ctx, chunk);
    options->external_struct_count = count_external_structs(ctx, chunk);
    if (options->external_fn_count > 0) {
        options->external_fns = calloc(options->external_fn_count, sizeof(RmbCGenExternalFn));
    }
    if (options->external_struct_count > 0) {
        options->external_structs = calloc(options->external_struct_count, sizeof(RmbCGenExternalStruct));
    }
    size_t fn_i = 0;
    size_t st_i = 0;
    for (size_t i = 0; i < ctx->order_count; i++) {
        BuildChunk* c = ctx->order[i];
        if (c == chunk) continue;
        for (RmbAstItem* it = c->ast->items; it; it = it->next) {
            if (it->kind == RMB_AST_ITEM_FN) {
                options->external_fns[fn_i++] = (RmbCGenExternalFn){
                    c->module_path, c->module_prefix, it
                };
            } else if (it->kind == RMB_AST_ITEM_STRUCT) {
                options->external_structs[st_i++] = (RmbCGenExternalStruct){
                    c->module_path, c->module_prefix, it
                };
            }
        }
    }
}

static bool emit_c(BuildContext* ctx, BuildChunk* chunk) {
    RmbCGenOptions options;
    fill_cgen_options(ctx, chunk, &options);
    RmbCGen cgen;
    rmb_cgen_init(&cgen, chunk->arena, chunk->ast);
    bool ok = rmb_cgen_emit_file(&cgen, chunk->c_path, options);
    free(options.external_fns);
    free(options.external_structs);
    return ok;
}

static int run_command(const char* cmd) {
    return system(cmd);
}

static const char* c_compiler(void) {
    const char* cc = getenv("CC");
    return (cc && cc[0] != '\0') ? cc : "gcc";
}

static bool compile_chunk(BuildChunk* chunk) {
    const char* cc = c_compiler();
    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
        "%s -std=c11 -Wall -Wextra -Werror -pedantic -fno-strict-aliasing -c \"%s\" -o \"%s\"",
        cc, chunk->c_path, chunk->o_path);
    int rc = run_command(cmd);
    if (rc != 0) {
        rmb_diag_error("%s failed for %s (exit %d)", cc, chunk->c_path, rc);
        return false;
    }
    return true;
}

static bool link_chunks(BuildContext* ctx, const char* exe_path) {
    const char* cc = c_compiler();
    size_t cap = strlen(cc) + 1024;
    for (size_t i = 0; i < ctx->order_count; i++) cap += strlen(ctx->order[i]->o_path) + 4;
    char* cmd = malloc(cap);
    if (!cmd) return false;
    snprintf(cmd, cap, "%s", cc);
    for (size_t i = 0; i < ctx->order_count; i++) {
        strncat(cmd, " \"", cap - strlen(cmd) - 1);
        strncat(cmd, ctx->order[i]->o_path, cap - strlen(cmd) - 1);
        strncat(cmd, "\"", cap - strlen(cmd) - 1);
    }
    strncat(cmd, " -o \"", cap - strlen(cmd) - 1);
    strncat(cmd, exe_path, cap - strlen(cmd) - 1);
    strncat(cmd, "\"", cap - strlen(cmd) - 1);
    int rc = run_command(cmd);
    free(cmd);
    if (rc != 0) {
        rmb_diag_error("%s link failed (exit %d)", cc, rc);
        return false;
    }
    return true;
}

static bool prepare_paths(BuildContext* ctx) {
    for (size_t i = 0; i < ctx->order_count; i++) {
        BuildChunk* c = ctx->order[i];
        c->c_path = chunk_artifact_path(c, ".c");
        c->o_path = chunk_artifact_path(c, ".o");
        c->rmi_path = chunk_artifact_path(c, ".rmi");
        if (!c->c_path || !c->o_path || !c->rmi_path) return false;
        if (!ensure_parent_dir(c->c_path)) {
            rmb_diag_error("failed to create output directory for %s", c->c_path);
            return false;
        }
    }
    return mkdir_p("build/debug/native/bin");
}

static void free_context(BuildContext* ctx) {
    for (size_t i = 0; i < ctx->count; i++) {
        BuildChunk* c = &ctx->chunks[i];
        if (c->ast) rmb_source_free(&c->source);
        if (c->arena) rmb_arena_destroy(c->arena);
        free(c->path);
        free(c->dir);
        free(c->stem);
        free(c->module_path);
        free(c->module_prefix);
        free(c->c_path);
        free(c->o_path);
        free(c->rmi_path);
    }
    free(ctx->chunks);
    free(ctx->order);
    free(ctx->entry_path);
    free(ctx->entry_dir);
    free(ctx->entry_stem);
}

bool rmb_build_project(const char* entry_path_raw) {
    BuildContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.entry_path = normalize_path(entry_path_raw);
    ctx.entry_dir = path_dir(ctx.entry_path);
    ctx.entry_stem = path_stem(ctx.entry_path);
    if (!ctx.entry_path || !ctx.entry_dir || !ctx.entry_stem) {
        rmb_diag_error("failed to initialize build paths");
        free_context(&ctx);
        return false;
    }
    BuildChunk* entry = add_chunk(&ctx, ctx.entry_path);
    bool ok = entry && visit_chunk(&ctx, entry) && prepare_paths(&ctx);
    for (size_t i = 0; ok && i < ctx.order_count; i++) {
        ok = check_chunk(ctx.order[i]);
    }
    for (size_t i = 0; ok && i < ctx.order_count; i++) {
        BuildChunk* c = ctx.order[i];
        ok = emit_rmi(c) && emit_c(&ctx, c) && compile_chunk(c);
    }
    char* exe_path = NULL;
    if (ok) {
        exe_path = bin_path(&ctx);
        ok = exe_path && link_chunks(&ctx, exe_path);
    }
    if (ok) {
        printf("build ok: %s -> %s\n", ctx.entry_path, exe_path);
    }
    free(exe_path);
    free_context(&ctx);
    return ok;
}
