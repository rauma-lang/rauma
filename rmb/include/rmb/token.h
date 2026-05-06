// RauMa Bootstrap Compiler - Token Definitions
// v0.0.2: Token kinds and token structure

#ifndef RMB_TOKEN_H
#define RMB_TOKEN_H

#include "common.h"
#include "span.h"
#include "string.h"

// Token kind enumeration
typedef enum RmbTokenKind {
    RMB_TOKEN_EOF,
    RMB_TOKEN_INVALID,

    RMB_TOKEN_IDENT,
    RMB_TOKEN_INT,
    RMB_TOKEN_STRING,

    RMB_TOKEN_KW_FN,
    RMB_TOKEN_KW_PUB,
    RMB_TOKEN_KW_STRUCT,
    RMB_TOKEN_KW_ENUM,
    RMB_TOKEN_KW_USE,
    RMB_TOKEN_KW_RETURN,
    RMB_TOKEN_KW_IF,
    RMB_TOKEN_KW_ELSE,
    RMB_TOKEN_KW_WHILE,
    RMB_TOKEN_KW_FOR,
    RMB_TOKEN_KW_MATCH,
    RMB_TOKEN_KW_CASE,
    RMB_TOKEN_KW_CONST,
    RMB_TOKEN_KW_DEFER,
    RMB_TOKEN_KW_TRUE,
    RMB_TOKEN_KW_FALSE,
    RMB_TOKEN_KW_NONE,

    RMB_TOKEN_L_PAREN,
    RMB_TOKEN_R_PAREN,
    RMB_TOKEN_L_BRACE,
    RMB_TOKEN_R_BRACE,
    RMB_TOKEN_L_BRACKET,
    RMB_TOKEN_R_BRACKET,

    RMB_TOKEN_COMMA,
    RMB_TOKEN_DOT,
    RMB_TOKEN_SEMI,
    RMB_TOKEN_COLON,

    RMB_TOKEN_QUESTION,
    RMB_TOKEN_BANG,
    RMB_TOKEN_BANG_BANG,

    RMB_TOKEN_PLUS,
    RMB_TOKEN_MINUS,
    RMB_TOKEN_STAR,
    RMB_TOKEN_SLASH,
    RMB_TOKEN_PERCENT,

    RMB_TOKEN_EQ,
    RMB_TOKEN_EQ_EQ,
    RMB_TOKEN_BANG_EQ,

    RMB_TOKEN_LT,
    RMB_TOKEN_GT,
    RMB_TOKEN_LT_EQ,
    RMB_TOKEN_GT_EQ,

    RMB_TOKEN_AND_AND,
    RMB_TOKEN_OR_OR,

    RMB_TOKEN_PLUS_EQ,
    RMB_TOKEN_MINUS_EQ,
    RMB_TOKEN_STAR_EQ,
    RMB_TOKEN_SLASH_EQ,
    RMB_TOKEN_COLON_EQ,

    RMB_TOKEN_AMP
} RmbTokenKind;

// Token structure
typedef struct RmbToken {
    RmbTokenKind kind;
    rmb_string lexeme;
    RmbSpan span;
} RmbToken;

// Get human-readable name for token kind
const char* rmb_token_kind_name(RmbTokenKind kind);

// Create a token
static RMB_INLINE RmbToken rmb_token_create(RmbTokenKind kind,
                                            rmb_string lexeme,
                                            RmbSpan span) {
    RmbToken token;
    token.kind = kind;
    token.lexeme = lexeme;
    token.span = span;
    return token;
}

// Check if token is a keyword
bool rmb_token_is_keyword(RmbTokenKind kind);

#endif // RMB_TOKEN_H