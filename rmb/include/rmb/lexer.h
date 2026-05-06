// RauMa Bootstrap Compiler - Lexer/Tokenizer
// v0.0.2: Token stream generation

#ifndef RMB_LEXER_H
#define RMB_LEXER_H

#include "common.h"
#include "source.h"
#include "token.h"

// Lexer state
typedef struct RmbLexer {
    RmbSource* source;     // Source being lexed (not owned)
    size_t pos;            // Current byte position
    int line;              // Current 1-based line number
    int col;               // Current 1-based column number
    bool had_error;        // Whether any errors occurred
} RmbLexer;

// Initialize lexer with source
void rmb_lexer_init(RmbLexer* lexer, RmbSource* source);

// Get next token from source
RmbToken rmb_lexer_next(RmbLexer* lexer);

// Check if lexer encountered errors
static RMB_INLINE bool rmb_lexer_had_error(const RmbLexer* lexer) {
    return lexer->had_error;
}

#endif // RMB_LEXER_H