// RauMa Bootstrap Compiler - Main Entry Point
// v0.0.7: Chunk build system (rmb build)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rmb/common.h"
#include "rmb/diag.h"
#include "rmb/source.h"
#include "rmb/lexer.h"
#include "rmb/token.h"
#include "rmb/ast.h"
#include "rmb/parser.h"
#include "rmb/checker.h"
#include "rmb/build.h"

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
    printf("rmb 0.0.7\n");
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

// Handle build command
static int handle_build(int argc, char** argv) {
    if (argc < 1) {
        rmb_diag_error("missing file argument for build command");
        return 1;
    }

    return rmb_build_project(argv[0]) ? 0 : 1;
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
