// RauMa Bootstrap Compiler - Lexer Implementation
// v0.0.2: Token stream generation

#include <ctype.h>
#include <string.h>

#include "rmb/lexer.h"
#include "rmb/diag.h"

// Helper: check if character is whitespace
static bool is_whitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

// Helper: check if character can start an identifier
static bool is_ident_start(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z') ||
           ch == '_';
}

// Helper: check if character can continue an identifier
static bool is_ident_continue(char ch) {
    return is_ident_start(ch) || (ch >= '0' && ch <= '9');
}

// Helper: check if character is a digit
static bool is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

// Helper: check if at end of source
static bool is_at_end(const RmbLexer* lexer) {
    return lexer->pos >= lexer->source->len;
}

// Helper: peek character at current position
static char peek(const RmbLexer* lexer) {
    if (is_at_end(lexer)) {
        return '\0';
    }
    return lexer->source->data[lexer->pos];
}

// Helper: peek character at offset
static char peek_at(const RmbLexer* lexer, size_t offset) {
    size_t pos = lexer->pos + offset;
    if (pos >= lexer->source->len) {
        return '\0';
    }
    return lexer->source->data[pos];
}

// Helper: advance and return previous character
static char advance(RmbLexer* lexer) {
    if (is_at_end(lexer)) {
        return '\0';
    }

    char ch = lexer->source->data[lexer->pos];
    lexer->pos++;

    // Update line and column
    if (ch == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }

    return ch;
}

// Helper: create a token from current span
static RmbToken make_token(RmbLexer* lexer, RmbTokenKind kind, size_t start) {
    rmb_string lexeme = rmb_source_slice(lexer->source, start, lexer->pos);
    RmbSpan span = rmb_span_create(lexer->source->path, start, lexer->pos,
                                   lexer->line, lexer->col - (lexer->pos - start));
    return rmb_token_create(kind, lexeme, span);
}


// Helper: check if next character matches expected
static bool match_char(RmbLexer* lexer, char expected) {
    if (is_at_end(lexer) || peek(lexer) != expected) {
        return false;
    }
    advance(lexer);
    return true;
}

// Initialize lexer with source
void rmb_lexer_init(RmbLexer* lexer, RmbSource* source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
    lexer->had_error = false;
}

// Skip whitespace and comments
static void skip_whitespace_and_comments(RmbLexer* lexer) {
    while (!is_at_end(lexer)) {
        char ch = peek(lexer);

        // Skip whitespace
        if (is_whitespace(ch)) {
            advance(lexer);
            continue;
        }

        // Skip line comments
        if (ch == '/' && peek_at(lexer, 1) == '/') {
            while (!is_at_end(lexer) && peek(lexer) != '\n') {
                advance(lexer);
            }
            continue;
        }

        // Skip block comments
        if (ch == '/' && peek_at(lexer, 1) == '*') {
            size_t start = lexer->pos;
            int start_line = lexer->line;
            int start_col = lexer->col;

            // Skip "/*"
            advance(lexer);
            advance(lexer);

            while (!is_at_end(lexer)) {
                if (peek(lexer) == '*' && peek_at(lexer, 1) == '/') {
                    // Skip "*/"
                    advance(lexer);
                    advance(lexer);
                    break;
                }
                advance(lexer);
            }

            // Check for unterminated block comment
            if (is_at_end(lexer)) {
                RmbSpan span = rmb_span_create(lexer->source->path, start, lexer->pos,
                                               start_line, start_col);
                rmb_diag_error_at(span, "unterminated block comment");
                lexer->had_error = true;
            }
            continue;
        }

        // Not whitespace or comment
        break;
    }
}

// Helper: check if lexeme matches a string literal
static bool lexeme_equals(rmb_string lexeme, const char* str) {
    size_t len = strlen(str);
    if (lexeme.len != len) return false;
    return memcmp(lexeme.ptr, str, len) == 0;
}

// Lex identifier or keyword
static RmbToken lex_identifier(RmbLexer* lexer) {
    size_t start = lexer->pos;
    while (!is_at_end(lexer) && is_ident_continue(peek(lexer))) {
        advance(lexer);
    }

    rmb_string lexeme = rmb_source_slice(lexer->source, start, lexer->pos);

    // Check for keywords
    if (lexeme_equals(lexeme, "fn")) return make_token(lexer, RMB_TOKEN_KW_FN, start);
    if (lexeme_equals(lexeme, "pub")) return make_token(lexer, RMB_TOKEN_KW_PUB, start);
    if (lexeme_equals(lexeme, "struct")) return make_token(lexer, RMB_TOKEN_KW_STRUCT, start);
    if (lexeme_equals(lexeme, "enum")) return make_token(lexer, RMB_TOKEN_KW_ENUM, start);
    if (lexeme_equals(lexeme, "use")) return make_token(lexer, RMB_TOKEN_KW_USE, start);
    if (lexeme_equals(lexeme, "return")) return make_token(lexer, RMB_TOKEN_KW_RETURN, start);
    if (lexeme_equals(lexeme, "if")) return make_token(lexer, RMB_TOKEN_KW_IF, start);
    if (lexeme_equals(lexeme, "else")) return make_token(lexer, RMB_TOKEN_KW_ELSE, start);
    if (lexeme_equals(lexeme, "while")) return make_token(lexer, RMB_TOKEN_KW_WHILE, start);
    if (lexeme_equals(lexeme, "for")) return make_token(lexer, RMB_TOKEN_KW_FOR, start);
    if (lexeme_equals(lexeme, "match")) return make_token(lexer, RMB_TOKEN_KW_MATCH, start);
    if (lexeme_equals(lexeme, "case")) return make_token(lexer, RMB_TOKEN_KW_CASE, start);
    if (lexeme_equals(lexeme, "const")) return make_token(lexer, RMB_TOKEN_KW_CONST, start);
    if (lexeme_equals(lexeme, "defer")) return make_token(lexer, RMB_TOKEN_KW_DEFER, start);
    if (lexeme_equals(lexeme, "true")) return make_token(lexer, RMB_TOKEN_KW_TRUE, start);
    if (lexeme_equals(lexeme, "false")) return make_token(lexer, RMB_TOKEN_KW_FALSE, start);
    if (lexeme_equals(lexeme, "none")) return make_token(lexer, RMB_TOKEN_KW_NONE, start);

    return make_token(lexer, RMB_TOKEN_IDENT, start);
}

// Lex integer literal
static RmbToken lex_number(RmbLexer* lexer) {
    size_t start = lexer->pos;
    while (!is_at_end(lexer) && is_digit(peek(lexer))) {
        advance(lexer);
    }
    return make_token(lexer, RMB_TOKEN_INT, start);
}

// Lex string literal
static RmbToken lex_string(RmbLexer* lexer) {
    size_t start = lexer->pos;
    int start_line = lexer->line;
    int start_col = lexer->col;

    // Skip opening quote
    advance(lexer);

    while (!is_at_end(lexer)) {
        char ch = peek(lexer);

        if (ch == '\n') {
            // Newline in string is an error
            RmbSpan span = rmb_span_create(lexer->source->path, start, lexer->pos,
                                           start_line, start_col);
            rmb_diag_error_at(span, "unterminated string literal");
            lexer->had_error = true;
            return make_token(lexer, RMB_TOKEN_STRING, start);
        }

        if (ch == '"') {
            // Closing quote
            advance(lexer);
            return make_token(lexer, RMB_TOKEN_STRING, start);
        }

        if (ch == '\\') {
            // Escape sequence
            advance(lexer);  // Skip backslash

            if (is_at_end(lexer)) {
                break;
            }

            char esc = peek(lexer);
            switch (esc) {
                case 'n':
                case 'r':
                case 't':
                case '0':
                case '"':
                case '\\':
                    advance(lexer);
                    break;
                default: {
                    // Unknown escape sequence
                    RmbSpan span = rmb_span_at(lexer->source->path, lexer->pos,
                                              lexer->line, lexer->col);
                    rmb_diag_error_at(span, "unknown escape sequence: \\%c", esc);
                    lexer->had_error = true;
                    advance(lexer);
                    break;
                }
            }
            continue;
        }

        advance(lexer);
    }

    // Unterminated string
    RmbSpan span = rmb_span_create(lexer->source->path, start, lexer->pos,
                                   start_line, start_col);
    rmb_diag_error_at(span, "unterminated string literal");
    lexer->had_error = true;
    return make_token(lexer, RMB_TOKEN_STRING, start);
}

// Get next token from source
RmbToken rmb_lexer_next(RmbLexer* lexer) {
    skip_whitespace_and_comments(lexer);

    if (is_at_end(lexer)) {
        RmbSpan span = rmb_span_at(lexer->source->path, lexer->pos,
                                  lexer->line, lexer->col);
        return rmb_token_create(RMB_TOKEN_EOF, rmb_string_from_cstr(""), span);
    }

    size_t start = lexer->pos;
    char ch = advance(lexer);

    // Identifiers and keywords
    if (is_ident_start(ch)) {
        lexer->pos = start;  // Reset to start
        lexer->col -= 1;     // Adjust column
        return lex_identifier(lexer);
    }

    // Numbers
    if (is_digit(ch)) {
        lexer->pos = start;  // Reset to start
        lexer->col -= 1;     // Adjust column
        return lex_number(lexer);
    }

    // Strings
    if (ch == '"') {
        lexer->pos = start;  // Reset to start
        lexer->col -= 1;     // Adjust column
        return lex_string(lexer);
    }

    // Single-character tokens
    switch (ch) {
        case '(': return make_token(lexer, RMB_TOKEN_L_PAREN, start);
        case ')': return make_token(lexer, RMB_TOKEN_R_PAREN, start);
        case '{': return make_token(lexer, RMB_TOKEN_L_BRACE, start);
        case '}': return make_token(lexer, RMB_TOKEN_R_BRACE, start);
        case '[': return make_token(lexer, RMB_TOKEN_L_BRACKET, start);
        case ']': return make_token(lexer, RMB_TOKEN_R_BRACKET, start);
        case ',': return make_token(lexer, RMB_TOKEN_COMMA, start);
        case '.': return make_token(lexer, RMB_TOKEN_DOT, start);
        case ';': return make_token(lexer, RMB_TOKEN_SEMI, start);
        case ':':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_COLON_EQ, start);
            return make_token(lexer, RMB_TOKEN_COLON, start);
        case '?': return make_token(lexer, RMB_TOKEN_QUESTION, start);
        case '!':
            if (match_char(lexer, '!')) return make_token(lexer, RMB_TOKEN_BANG_BANG, start);
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_BANG_EQ, start);
            return make_token(lexer, RMB_TOKEN_BANG, start);
        case '+':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_PLUS_EQ, start);
            return make_token(lexer, RMB_TOKEN_PLUS, start);
        case '-':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_MINUS_EQ, start);
            return make_token(lexer, RMB_TOKEN_MINUS, start);
        case '*':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_STAR_EQ, start);
            return make_token(lexer, RMB_TOKEN_STAR, start);
        case '/':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_SLASH_EQ, start);
            return make_token(lexer, RMB_TOKEN_SLASH, start);
        case '%': return make_token(lexer, RMB_TOKEN_PERCENT, start);
        case '=':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_EQ_EQ, start);
            return make_token(lexer, RMB_TOKEN_EQ, start);
        case '<':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_LT_EQ, start);
            return make_token(lexer, RMB_TOKEN_LT, start);
        case '>':
            if (match_char(lexer, '=')) return make_token(lexer, RMB_TOKEN_GT_EQ, start);
            return make_token(lexer, RMB_TOKEN_GT, start);
        case '&':
            if (match_char(lexer, '&')) return make_token(lexer, RMB_TOKEN_AND_AND, start);
            return make_token(lexer, RMB_TOKEN_AMP, start);
        case '|':
            if (match_char(lexer, '|')) return make_token(lexer, RMB_TOKEN_OR_OR, start);
            break;  // Fall through to invalid token
    }

    // Invalid character
    RmbSpan span = rmb_span_at(lexer->source->path, start, lexer->line, lexer->col - 1);
    rmb_diag_error_at(span, "invalid character: '%c' (0x%02x)", ch, (unsigned char)ch);
    lexer->had_error = true;
    return make_token(lexer, RMB_TOKEN_INVALID, start);
}