// RauMa Bootstrap Compiler - Main Entry Point
// v0.0.3: Parser implementation

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

// Print help message
static void print_help(void) {
    printf("RauMa Bootstrap Compiler (rmb)\n");
    printf("usage: rmb <command> [options]\n");
    printf("\n");
    printf("commands:\n");
    printf("  lex      tokenize RauMa source\n");
    printf("  parse    parse RauMa source and print AST summary\n");
    printf("  version  print compiler version\n");
    printf("  help     print this help message\n");
    printf("\n");
    printf("options:\n");
    printf("  --help     show this help\n");
}

// Print version information
static void print_version(void) {
    printf("rmb 0.0.3\n");
}

// Handle lex command
static int handle_lex(int argc, char** argv) {
    if (argc < 1) {
        rmb_diag_error("missing file argument for lex command");
        return 1;
    }

    const char* filename = argv[0];

    // Read source file
    RmbSource source;
    if (!rmb_source_read(filename, &source)) {
        return 1;
    }

    // Initialize lexer
    RmbLexer lexer;
    rmb_lexer_init(&lexer, &source);

    // Tokenize and print tokens
    while (true) {
        RmbToken token = rmb_lexer_next(&lexer);

        // Print token
        printf("%s \"%.*s\" %d:%d\n",
               rmb_token_kind_name(token.kind),
               (int)token.lexeme.len, token.lexeme.ptr,
               token.span.line, token.span.col);

        if (token.kind == RMB_TOKEN_EOF) {
            break;
        }
    }

    // Free source
    rmb_source_free(&source);

    // Return error status
    return rmb_lexer_had_error(&lexer) ? 1 : 0;
}

// Handle parse command
static int handle_parse(int argc, char** argv) {
    if (argc < 1) {
        rmb_diag_error("missing file argument for parse command");
        return 1;
    }

    const char* filename = argv[0];

    // Read source file
    RmbSource source;
    if (!rmb_source_read(filename, &source)) {
        return 1;
    }

    // Create arena for AST allocation
    rmb_arena* arena = rmb_arena_create(1024 * 1024); // 1MB initial size
    if (!arena) {
        rmb_diag_error("failed to create memory arena");
        rmb_source_free(&source);
        return 1;
    }
    rmb_ast_init(arena);

    // Tokenize entire file
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

    // Initialize parser
    RmbParser parser;
    rmb_parser_init(&parser, tokens, token_count, arena);

    // Parse file
    RmbAstFile* ast = rmb_parse_file(&parser, filename);
    if (!ast) {
        rmb_source_free(&source);
        rmb_arena_destroy(arena);
        return 1;
    }

    // Print AST summary
    rmb_ast_file_print_summary(ast);

    // Cleanup
    rmb_source_free(&source);
    rmb_arena_destroy(arena);

    // Return error status
    return rmb_parser_had_error(&parser) ? 1 : 0;
}

// Main entry point
int main(int argc, char** argv) {
    // Initialize diagnostics
    rmb_diag_init();
    rmb_diag_set_color(false);  // No color output for now

    // Handle no arguments
    if (argc < 2) {
        print_help();
        rmb_diag_shutdown();
        return 0;
    }

    // Get command
    const char* command = argv[1];
    int result = 0;

    // Dispatch to command handler
    if (strcmp(command, "lex") == 0) {
        result = handle_lex(argc - 2, argv + 2);
    }
    else if (strcmp(command, "parse") == 0) {
        result = handle_parse(argc - 2, argv + 2);
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

    // Cleanup
    rmb_diag_shutdown();

    return result;
}