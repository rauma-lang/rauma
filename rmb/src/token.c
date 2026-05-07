// RauMa Bootstrap Compiler - Token Implementation
// v0.0.2: Token kinds and token structure

#include <string.h>
#include "rmb/token.h"

// Token kind names (must match RmbTokenKind enum order)
static const char* TOKEN_NAMES[] = {
    "eof",
    "invalid",
    "ident",
    "int",
    "string",
    "kw_fn",
    "kw_pub",
    "kw_struct",
    "kw_enum",
    "kw_use",
    "kw_return",
    "kw_if",
    "kw_else",
    "kw_while",
    "kw_for",
    "kw_match",
    "kw_case",
    "kw_const",
    "kw_defer",
    "kw_true",
    "kw_false",
    "kw_none",
    "l_paren",
    "r_paren",
    "l_brace",
    "r_brace",
    "l_bracket",
    "r_bracket",
    "comma",
    "dot",
    "semi",
    "colon",
    "question",
    "bang",
    "bang_bang",
    "plus",
    "minus",
    "star",
    "slash",
    "percent",
    "eq",
    "eq_eq",
    "bang_eq",
    "lt",
    "gt",
    "lt_eq",
    "gt_eq",
    "and_and",
    "or_or",
    "plus_eq",
    "minus_eq",
    "star_eq",
    "slash_eq",
    "colon_eq",
    "amp"
};

// Get human-readable name for token kind
const char* rmb_token_kind_name(RmbTokenKind kind) {
    if (kind >= 0 && kind < (int)(sizeof(TOKEN_NAMES) / sizeof(TOKEN_NAMES[0]))) {
        return TOKEN_NAMES[kind];
    }
    return "unknown";
}

// Check if token is a keyword
bool rmb_token_is_keyword(RmbTokenKind kind) {
    return kind >= RMB_TOKEN_KW_FN && kind <= RMB_TOKEN_KW_NONE;
}