// RauMa Bootstrap Compiler - Parser
// v0.0.3: Recursive descent parser

#ifndef RMB_PARSER_H
#define RMB_PARSER_H

#include "common.h"
#include "source.h"
#include "token.h"
#include "ast.h"
#include "arena.h"

// Parser state
typedef struct RmbParser {
    RmbToken* tokens;
    size_t len;
    size_t pos;
    bool had_error;
    rmb_arena* arena;
} RmbParser;

// Initialize parser
void rmb_parser_init(RmbParser* parser, RmbToken* tokens, size_t len, rmb_arena* arena);

// Parse entire file
RmbAstFile* rmb_parse_file(RmbParser* parser, const char* path);

// Parse specific constructs (for internal use)
RmbAstItem* rmb_parse_item(RmbParser* parser);
RmbAstStmt* rmb_parse_stmt(RmbParser* parser);
RmbAstExpr* rmb_parse_expr(RmbParser* parser);
RmbAstTypeRef* rmb_parse_type(RmbParser* parser);

// Token collection helper (lex entire source into tokens)
bool rmb_lex_all(RmbSource* source, rmb_arena* arena, RmbToken** out_tokens, size_t* out_len, bool* out_had_error);

// Check if parser encountered errors
static RMB_INLINE bool rmb_parser_had_error(const RmbParser* parser) {
    return parser->had_error;
}

#endif // RMB_PARSER_H