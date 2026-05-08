// RauMa Bootstrap Compiler - Main Entry Point
// v0.0.6: C codegen (rmb build)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "rmb/common.h"
#include "rmb/diag.h"
#include "rmb/source.h"
#include "rmb/lexer.h"
#include "rmb/token.h"
#include "rmb/ast.h"
#include "rmb/parser.h"
#include "rmb/checker.h"
#include "rmb/cgen.h"

// Print help message
static void print_help(void) {
    printf("RauMa Bootstrap Compiler (rmb)\n");
    printf("usage: rmb <command> [options]\n");
    printf("\n");
    printf("commands:\n");
    printf("  lex      tokenize RauMa source\n");
    printf("  parse    parse RauMa source and print AST summary\n");
    printf("  check    parse and type-check RauMa source\n");
    printf("  build    build RauMa source to executable\n");
    printf("  version  print compiler version\n");
    printf("  help     print this help message\n");
}

// Print version information
static void print_version(void) {
    printf("rmb 0.0.6\n");
}

// Handle lex command
static int handle_lex(int argc, char** argv) {
    if (argc < 1) {
        rmb_diag_error("missing file argument for lex command");
        return 1;
    }

    const char* filename = argv[0];

    RmbSource source;
    if (!rmb_source_read(filename, &source)) {
        return 1;
    }

    RmbLexer lexer;
    rmb_lexer_init(&lexer, &source);

    while (true) {
        RmbToken token = rmb_lexer_next(&lexer);

        printf("%s \"%.*s\" %d:%d\n",
               rmb_token_kind_name(token.kind),
               (int)token.lexeme.len, token.lexeme.ptr,
               token.span.line, token.span.col);

        if (token.kind == RMB_TOKEN_EOF) {
            break;
        }
    }

    rmb_source_free(&source);
    return rmb_lexer_had_error(&lexer) ? 1 : 0;
}

// Handle parse command
static int handle_parse(int argc, char** argv) {
    if (argc < 1) {
        rmb_diag_error("missing file argument for parse command");
        return 1;
    }

    const char* filename = argv[0];

    RmbSource source;
    if (!rmb_source_read(filename, &source)) {
        return 1;
    }

    rmb_arena* arena = rmb_arena_create(1024 * 1024);
    if (!arena) {
        rmb_diag_error("failed to create memory arena");
        rmb_source_free(&source);
        return 1;
    }
    rmb_ast_init(arena);

    RmbToken* tokens = NULL;
    size_t token_count = 0;
    bool had_lex_error = false;
    if (!rmb_lex_all(&source, arena, &tokens, &token_count, &had_lex_error)) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    if (had_lex_error) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    RmbParser parser;
    rmb_parser_init(&parser, tokens, token_count, arena);

    RmbAstFile* ast = rmb_parse_file(&parser, filename);
    if (!ast) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    rmb_ast_file_print_summary(ast);

    rmb_source_free(&source);
    rmb_arena_destroy(arena);

    return rmb_parser_had_error(&parser) ? 1 : 0;
}

// Handle check command
static int handle_check(int argc, char** argv) {
    if (argc < 1) {
        rmb_diag_error("missing file argument for check command");
        return 1;
    }

    const char* filename = argv[0];

    RmbSource source;
    if (!rmb_source_read(filename, &source)) {
        return 1;
    }

    rmb_arena* arena = rmb_arena_create(1024 * 1024);
    if (!arena) {
        rmb_diag_error("failed to create memory arena");
        rmb_source_free(&source);
        return 1;
    }
    rmb_ast_init(arena);

    RmbToken* tokens = NULL;
    size_t token_count = 0;
    bool had_lex_error = false;
    if (!rmb_lex_all(&source, arena, &tokens, &token_count, &had_lex_error)) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }
    if (had_lex_error) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    RmbParser parser;
    rmb_parser_init(&parser, tokens, token_count, arena);

    RmbAstFile* ast = rmb_parse_file(&parser, filename);
    if (!ast || rmb_parser_had_error(&parser)) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    RmbChecker checker;
    rmb_checker_init(&checker, arena, ast);
    bool ok = rmb_check_file(&checker);

    rmb_source_free(&source);
    rmb_arena_destroy(arena);

    if (!ok) return 1;

    printf("check ok: %s\n", filename);
    return 0;
}

// ----- build helpers -----

// Find the basename of a path (strip any directory and extension).
// Returns a malloc'd string. Caller frees.
static char* path_stem(const char* path) {
    size_t len = strlen(path);
    size_t start = 0;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/' || path[i] == '\\') start = i + 1;
    }
    size_t end = len;
    for (size_t i = len; i > start; i--) {
        if (path[i - 1] == '.') {
            end = i - 1;
            break;
        }
    }
    size_t out_len = (end > start) ? end - start : 0;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    memcpy(out, path + start, out_len);
    out[out_len] = '\0';
    return out;
}

// Ensure the build/ directory exists. Best-effort: ignores errors if already exists.
static void ensure_build_dir(void) {
#ifdef _WIN32
    (void)_mkdir("build");
#else
    (void)mkdir("build", 0755);
#endif
}

static int run_gcc(const char* c_path, const char* exe_path) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "gcc -std=c11 -O0 -g -o \"%s\" \"%s\"",
        exe_path, c_path);
    int rc = system(cmd);
    return rc;
}

// Handle build command
static int handle_build(int argc, char** argv) {
    if (argc < 1) {
        rmb_diag_error("missing file argument for build command");
        return 1;
    }

    const char* filename = argv[0];

    RmbSource source;
    if (!rmb_source_read(filename, &source)) {
        return 1;
    }

    rmb_arena* arena = rmb_arena_create(1024 * 1024);
    if (!arena) {
        rmb_diag_error("failed to create memory arena");
        rmb_source_free(&source);
        return 1;
    }
    rmb_ast_init(arena);

    RmbToken* tokens = NULL;
    size_t token_count = 0;
    bool had_lex_error = false;
    if (!rmb_lex_all(&source, arena, &tokens, &token_count, &had_lex_error)) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }
    if (had_lex_error) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    RmbParser parser;
    rmb_parser_init(&parser, tokens, token_count, arena);

    RmbAstFile* ast = rmb_parse_file(&parser, filename);
    if (!ast || rmb_parser_had_error(&parser)) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    // Codegen v0.0.6 pre-check: reject features the C backend cannot
    // handle yet, before the type-checker rejects them with its own (less
    // codegen-flavoured) message. Keeps the diagnostic crisp for users.
    bool cg_precheck_ok = true;
    for (RmbAstItem* it = ast->items; it; it = it->next) {
        if (it->kind == RMB_AST_ITEM_FN && it->fn_item.error_type != NULL) {
            rmb_diag_error_at(it->span,
                "error-returning functions are not supported by codegen v0.0.6");
            cg_precheck_ok = false;
        }
    }
    if (!cg_precheck_ok) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    RmbChecker checker;
    rmb_checker_init(&checker, arena, ast);
    if (!rmb_check_file(&checker)) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    // Build output paths under build/.
    char* stem = path_stem(filename);
    if (!stem || stem[0] == '\0') {
        rmb_diag_error("could not derive output name from: %s", filename);
        free(stem);
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    ensure_build_dir();

    char c_path[1024];
    char exe_path[1024];
    snprintf(c_path, sizeof(c_path), "build/%s.c", stem);
    snprintf(exe_path, sizeof(exe_path), "build/%s", stem);

    RmbCGen cgen;
    rmb_cgen_init(&cgen, arena, ast);
    bool emit_ok = rmb_cgen_emit_file(&cgen, c_path);

    if (!emit_ok) {
        free(stem);
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    int gcc_rc = run_gcc(c_path, exe_path);
    if (gcc_rc != 0) {
        rmb_diag_error("gcc failed for %s (exit %d)", c_path, gcc_rc);
        free(stem);
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    printf("build ok: %s -> %s\n", filename, exe_path);

    free(stem);
    rmb_source_free(&source);
    rmb_arena_destroy(arena);
    return 0;
}

// Main entry point
int main(int argc, char** argv) {
    rmb_diag_init();
    rmb_diag_set_color(false);

    if (argc < 2) {
        print_help();
        rmb_diag_shutdown();
        return 0;
    }

    const char* command = argv[1];
    int result = 0;

    if (strcmp(command, "lex") == 0) {
        result = handle_lex(argc - 2, argv + 2);
    }
    else if (strcmp(command, "parse") == 0) {
        result = handle_parse(argc - 2, argv + 2);
    }
    else if (strcmp(command, "check") == 0) {
        result = handle_check(argc - 2, argv + 2);
    }
    else if (strcmp(command, "build") == 0) {
        result = handle_build(argc - 2, argv + 2);
    }
    else if (strcmp(command, "version") == 0) {
        print_version();
    }
    else if (strcmp(command, "help") == 0 ||
             strcmp(command, "--help") == 0 ||
             strcmp(command, "-h") == 0) {
        print_help();
    }
    else {
        rmb_diag_error("Unknown command: %s", command);
        print_help();
        result = 1;
    }

    rmb_diag_shutdown();
    return result;
}
