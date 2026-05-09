// RauMa Bootstrap Compiler - Parser Implementation
// v0.0.3: Recursive descent parser

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "rmb/parser.h"
#include "rmb/diag.h"
#include "rmb/lexer.h"

// Helper function to combine two spans
static RmbSpan span_union(RmbSpan a, RmbSpan b) {
    return rmb_span_merge(a, b);
}

// Forward declarations for expression parsing functions
static RmbAstExpr* rmb_parse_assignment(RmbParser* parser);
static RmbAstExpr* rmb_parse_logical_or(RmbParser* parser);
static RmbAstExpr* rmb_parse_logical_and(RmbParser* parser);
static RmbAstExpr* rmb_parse_equality(RmbParser* parser);
static RmbAstExpr* rmb_parse_comparison(RmbParser* parser);
static RmbAstExpr* rmb_parse_term(RmbParser* parser);
static RmbAstExpr* rmb_parse_factor(RmbParser* parser);
static RmbAstExpr* rmb_parse_unary(RmbParser* parser);
static RmbAstExpr* rmb_parse_postfix(RmbParser* parser);
static RmbAstExpr* rmb_parse_primary(RmbParser* parser);

// Token vector helper
typedef struct {
    RmbToken* items;
    size_t capacity;
    size_t count;
} TokenVec;

static void token_vec_init(TokenVec* vec, size_t initial_capacity) {
    vec->items = NULL;
    vec->capacity = 0;
    vec->count = 0;
    if (initial_capacity > 0) {
        vec->items = malloc(initial_capacity * sizeof(RmbToken));
        if (vec->items) {
            vec->capacity = initial_capacity;
        }
    }
}

static void token_vec_push(TokenVec* vec, RmbToken token) {
    if (vec->count >= vec->capacity) {
        size_t new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
        RmbToken* new_items = realloc(vec->items, new_capacity * sizeof(RmbToken));
        if (!new_items) {
            return;
        }
        vec->items = new_items;
        vec->capacity = new_capacity;
    }
    vec->items[vec->count++] = token;
}

static void token_vec_free(TokenVec* vec) {
    free(vec->items);
    vec->items = NULL;
    vec->capacity = 0;
    vec->count = 0;
}

// Lex entire source into tokens
bool rmb_lex_all(RmbSource* source, rmb_arena* arena, RmbToken** out_tokens, size_t* out_len, bool* out_had_error) {
    TokenVec vec;
    token_vec_init(&vec, 64);

    RmbLexer lexer;
    rmb_lexer_init(&lexer, source);

    while (true) {
        RmbToken token = rmb_lexer_next(&lexer);
        token_vec_push(&vec, token);

        if (token.kind == RMB_TOKEN_EOF) {
            break;
        }
    }

    *out_had_error = rmb_lexer_had_error(&lexer);

    if (*out_had_error) {
        token_vec_free(&vec);
        return false;
    }

    // Copy tokens to arena
    size_t token_size = vec.count * sizeof(RmbToken);
    RmbToken* tokens = rmb_arena_alloc(arena, token_size);
    if (!tokens) {
        token_vec_free(&vec);
        return false;
    }

    memcpy(tokens, vec.items, token_size);
    *out_tokens = tokens;
    *out_len = vec.count;

    token_vec_free(&vec);
    return true;
}

// Initialize parser
void rmb_parser_init(RmbParser* parser, RmbToken* tokens, size_t len, rmb_arena* arena) {
    parser->tokens = tokens;
    parser->len = len;
    parser->pos = 0;
    parser->had_error = false;
    parser->arena = arena;
}

// Parser helper functions
static RmbToken* parser_current(RmbParser* parser) {
    if (parser->pos >= parser->len) {
        return &parser->tokens[parser->len - 1]; // Should be EOF
    }
    return &parser->tokens[parser->pos];
}

static RmbToken* parser_peek(RmbParser* parser, size_t offset) {
    size_t idx = parser->pos + offset;
    if (idx >= parser->len) {
        return &parser->tokens[parser->len - 1];
    }
    return &parser->tokens[idx];
}

static RmbToken* parser_advance(RmbParser* parser) {
    if (parser->pos < parser->len) {
        parser->pos++;
    }
    return parser_current(parser);
}

static bool parser_check(RmbParser* parser, RmbTokenKind kind) {
    if (parser->pos >= parser->len) {
        return false;
    }
    return parser_current(parser)->kind == kind;
}

static bool parser_match(RmbParser* parser, RmbTokenKind kind) {
    if (parser_check(parser, kind)) {
        parser_advance(parser);
        return true;
    }
    return false;
}

static RmbToken* parser_consume(RmbParser* parser, RmbTokenKind kind, const char* error_msg) {
    if (parser_check(parser, kind)) {
        RmbToken* token = parser_current(parser);
        parser_advance(parser);
        return token;
    }

    RmbToken* token = parser_current(parser);
    rmb_diag_error_at(token->span, "expected %s", error_msg);
    parser->had_error = true;
    return token;
}

static void parser_synchronize(RmbParser* parser) {
    // Skip tokens until we find a statement boundary or top-level item
    while (parser->pos < parser->len) {
        RmbToken* token = parser_current(parser);

        // Statement boundaries
        if (token->kind == RMB_TOKEN_SEMI ||
            token->kind == RMB_TOKEN_R_BRACE ||
            token->kind == RMB_TOKEN_R_PAREN ||
            token->kind == RMB_TOKEN_EOF) {
            return;
        }

        // Top-level item keywords
        if (token->kind == RMB_TOKEN_KW_FN ||
            token->kind == RMB_TOKEN_KW_PUB ||
            token->kind == RMB_TOKEN_KW_STRUCT ||
            token->kind == RMB_TOKEN_KW_ENUM ||
            token->kind == RMB_TOKEN_KW_USE) {
            return;
        }

        parser_advance(parser);
    }
}

// Helper: join multiple strings with dot separator
static rmb_string join_with_dots(RmbParser* parser, RmbToken** tokens, size_t start, size_t count) {
    if (count == 0) {
        return rmb_string_from_cstr("");
    }

    // Calculate total length
    size_t total_len = 0;
    for (size_t i = 0; i < count; i++) {
        total_len += tokens[start + i]->lexeme.len;
    }
    // Add dots between (count - 1)
    total_len += (count - 1);

    // Allocate buffer
    char* buf = rmb_arena_alloc(parser->arena, total_len + 1);
    if (!buf) {
        return rmb_string_from_cstr("");
    }

    char* pos = buf;
    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            *pos++ = '.';
        }
        rmb_string part = tokens[start + i]->lexeme;
        memcpy(pos, part.ptr, part.len);
        pos += part.len;
    }
    *pos = '\0';

    return (rmb_string){buf, total_len};
}

// Parse a type reference
RmbAstTypeRef* rmb_parse_type(RmbParser* parser) {
    RmbToken* start = parser_current(parser);

    // Pointer type
    if (parser_match(parser, RMB_TOKEN_STAR)) {
        RmbAstTypeRef* elem = rmb_parse_type(parser);
        return rmb_ast_type_pointer(span_union(start->span,parser_current(parser)->span), elem);
    }

    // Slice type
    if (parser_match(parser, RMB_TOKEN_L_BRACKET)) {
        if (parser_match(parser, RMB_TOKEN_R_BRACKET)) {
            RmbAstTypeRef* elem = rmb_parse_type(parser);
            return rmb_ast_type_slice(span_union(start->span,parser_current(parser)->span), elem);
        } else {
            // Array type [N]T
            RmbAstExpr* size = rmb_parse_expr(parser);
            parser_consume(parser, RMB_TOKEN_R_BRACKET, "]");
            RmbAstTypeRef* elem = rmb_parse_type(parser);
            return rmb_ast_type_array(span_union(start->span,parser_current(parser)->span), size, elem);
        }
    }

    // Qualified or simple type
    if (parser_check(parser, RMB_TOKEN_IDENT)) {
        // Collect identifier chain: a.b.c
        RmbToken* tokens[8];  // reasonable limit
        size_t token_count = 0;

        while (parser_check(parser, RMB_TOKEN_IDENT) && token_count < 8) {
            tokens[token_count++] = parser_current(parser);
            parser_advance(parser);

            if (!parser_match(parser, RMB_TOKEN_DOT)) {
                break;
            }
        }

        if (token_count == 0) {
            // Should not happen
            rmb_diag_error_at(start->span, "expected type identifier");
            parser->had_error = true;
            return rmb_ast_type_simple(start->span, rmb_string_from_cstr("unknown"));
        }

        RmbAstTypeRef* type = NULL;
        if (token_count > 1) {
            // Qualified type: module.name
            rmb_string module = tokens[0]->lexeme;
            rmb_string name = join_with_dots(parser, tokens, 1, token_count - 1);
            type = rmb_ast_type_qualified(
                span_union(tokens[0]->span, tokens[token_count-1]->span),
                module,
                name
            );
        } else {
            // Simple type
            type = rmb_ast_type_simple(tokens[0]->span, tokens[0]->lexeme);
        }

        // Optional type T?
        if (parser_match(parser, RMB_TOKEN_QUESTION)) {
            type = rmb_ast_type_optional(span_union(tokens[0]->span,parser_current(parser)->span), type);
        }

        return type;
    }

    // Error: expected a type
    rmb_diag_error_at(start->span, "expected type");
    parser->had_error = true;
    parser_synchronize(parser);
    return rmb_ast_type_simple(start->span, rmb_string_from_cstr("unknown"));
}

// Parse an expression (simplified precedence)
RmbAstExpr* rmb_parse_expr(RmbParser* parser) {
    return rmb_parse_assignment(parser);
}

// Parse assignment expression (lowest precedence)
static RmbAstExpr* rmb_parse_assignment(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_logical_or(parser);

    // Note: Assignment is a statement in RauMa, not an expression
    // So we don't parse assignment expressions here

    return expr;
}

// Parse logical OR (||)
static RmbAstExpr* rmb_parse_logical_or(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_logical_and(parser);

    while (parser_match(parser, RMB_TOKEN_OR_OR)) {
        RmbAstExpr* right = rmb_parse_logical_and(parser);
        expr = rmb_ast_expr_binary(
            span_union(expr->span,right->span),
            RMB_TOKEN_OR_OR,
            expr,
            right
        );
    }

    return expr;
}

// Parse logical AND (&&)
static RmbAstExpr* rmb_parse_logical_and(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_equality(parser);

    while (parser_match(parser, RMB_TOKEN_AND_AND)) {
        RmbAstExpr* right = rmb_parse_equality(parser);
        expr = rmb_ast_expr_binary(
            span_union(expr->span,right->span),
            RMB_TOKEN_AND_AND,
            expr,
            right
        );
    }

    return expr;
}

// Parse equality (==, !=)
static RmbAstExpr* rmb_parse_equality(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_comparison(parser);

    while (parser_check(parser, RMB_TOKEN_EQ_EQ) || parser_check(parser, RMB_TOKEN_BANG_EQ)) {
        RmbTokenKind op = parser_current(parser)->kind;
        parser_advance(parser);
        RmbAstExpr* right = rmb_parse_comparison(parser);
        expr = rmb_ast_expr_binary(
            span_union(expr->span,right->span),
            op,
            expr,
            right
        );
    }

    return expr;
}

// Parse comparison (<, >, <=, >=)
static RmbAstExpr* rmb_parse_comparison(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_term(parser);

    while (parser_check(parser, RMB_TOKEN_LT) || parser_check(parser, RMB_TOKEN_GT) ||
           parser_check(parser, RMB_TOKEN_LT_EQ) || parser_check(parser, RMB_TOKEN_GT_EQ)) {
        RmbTokenKind op = parser_current(parser)->kind;
        parser_advance(parser);
        RmbAstExpr* right = rmb_parse_term(parser);
        expr = rmb_ast_expr_binary(
            span_union(expr->span,right->span),
            op,
            expr,
            right
        );
    }

    return expr;
}

// Parse term (+, -)
static RmbAstExpr* rmb_parse_term(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_factor(parser);

    while (parser_check(parser, RMB_TOKEN_PLUS) || parser_check(parser, RMB_TOKEN_MINUS)) {
        RmbTokenKind op = parser_current(parser)->kind;
        parser_advance(parser);
        RmbAstExpr* right = rmb_parse_factor(parser);
        expr = rmb_ast_expr_binary(
            span_union(expr->span,right->span),
            op,
            expr,
            right
        );
    }

    return expr;
}

// Parse factor (*, /, %)
static RmbAstExpr* rmb_parse_factor(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_unary(parser);

    while (parser_check(parser, RMB_TOKEN_STAR) || parser_check(parser, RMB_TOKEN_SLASH) ||
           parser_check(parser, RMB_TOKEN_PERCENT)) {
        RmbTokenKind op = parser_current(parser)->kind;
        parser_advance(parser);
        RmbAstExpr* right = rmb_parse_unary(parser);
        expr = rmb_ast_expr_binary(
            span_union(expr->span,right->span),
            op,
            expr,
            right
        );
    }

    return expr;
}

// Parse unary (!, -, *, &)
static RmbAstExpr* rmb_parse_unary(RmbParser* parser) {
    RmbToken* op_token = parser_current(parser);
    RmbTokenKind op = op_token->kind;
    if (parser_match(parser, RMB_TOKEN_BANG) || parser_match(parser, RMB_TOKEN_MINUS) ||
        parser_match(parser, RMB_TOKEN_STAR) || parser_match(parser, RMB_TOKEN_AMP)) {
        RmbAstExpr* operand = rmb_parse_unary(parser);
        return rmb_ast_expr_unary(
            span_union(op_token->span, operand->span),
            op,
            operand
        );
    }

    return rmb_parse_postfix(parser);
}

// Parse postfix expressions (field access, call, error operators, optional else)
static RmbAstExpr* rmb_parse_postfix(RmbParser* parser) {
    RmbAstExpr* expr = rmb_parse_primary(parser);

    while (true) {
        // Field access: expr.field
        if (parser_match(parser, RMB_TOKEN_DOT)) {
            RmbToken* field = parser_consume(parser, RMB_TOKEN_IDENT, "field name");
            expr = rmb_ast_expr_field(
                span_union(expr->span,field->span),
                expr,
                field->lexeme
            );
        }
        // Function call: expr(arg1, arg2, ...)
        else if (parser_match(parser, RMB_TOKEN_L_PAREN)) {
            RmbAstExpr** args = NULL;
            size_t arg_count = 0;

            if (!parser_check(parser, RMB_TOKEN_R_PAREN)) {
                do {
                    RmbAstExpr* arg = rmb_parse_expr(parser);
                    if (arg) {
                        // Simple dynamic array for args
                        size_t new_size = (arg_count + 1) * sizeof(RmbAstExpr*);
                        RmbAstExpr** new_args = rmb_arena_alloc(parser->arena, new_size);
                        if (new_args) {
                            if (args) {
                                memcpy(new_args, args, arg_count * sizeof(RmbAstExpr*));
                            }
                            new_args[arg_count] = arg;
                            args = new_args;
                            arg_count++;
                        }
                    }
                } while (parser_match(parser, RMB_TOKEN_COMMA));
            }

            parser_consume(parser, RMB_TOKEN_R_PAREN, ")");
            expr = rmb_ast_expr_call(
                span_union(expr->span,parser_current(parser)->span),
                expr,
                args,
                arg_count
            );
        }
        // Error propagation: expr?
        else if (parser_match(parser, RMB_TOKEN_QUESTION)) {
            expr = rmb_ast_expr_error_prop(
                span_union(expr->span,parser_current(parser)->span),
                expr
            );
        }
        // Error panic: expr!
        else if (parser_match(parser, RMB_TOKEN_BANG)) {
            expr = rmb_ast_expr_error_panic(
                span_union(expr->span,parser_current(parser)->span),
                expr
            );
        }
        // Optional else: expr else fallback
        else if (parser_match(parser, RMB_TOKEN_KW_ELSE)) {
            RmbAstExpr* fallback = rmb_parse_expr(parser);
            expr = rmb_ast_expr_else(
                span_union(expr->span,fallback->span),
                expr,
                fallback
            );
        } else {
            break;
        }
    }

    return expr;
}

// Parse primary expressions (identifiers, literals, parenthesized expressions)
static RmbAstExpr* rmb_parse_primary(RmbParser* parser) {
    RmbToken* token = parser_current(parser);

    // Identifier
    if (parser_match(parser, RMB_TOKEN_IDENT)) {
        return rmb_ast_expr_ident(token->span, token->lexeme);
    }

    // Integer literal
    if (parser_match(parser, RMB_TOKEN_INT)) {
        return rmb_ast_expr_int(token->span, token->lexeme);
    }

    // String literal
    if (parser_match(parser, RMB_TOKEN_STRING)) {
        return rmb_ast_expr_string(token->span, token->lexeme);
    }

    // Boolean literals
    if (parser_match(parser, RMB_TOKEN_KW_TRUE)) {
        return rmb_ast_expr_bool(token->span, true);
    }
    if (parser_match(parser, RMB_TOKEN_KW_FALSE)) {
        return rmb_ast_expr_bool(token->span, false);
    }

    // None literal
    if (parser_match(parser, RMB_TOKEN_KW_NONE)) {
        return rmb_ast_expr_none(token->span);
    }

    // Parenthesized expression
    if (parser_match(parser, RMB_TOKEN_L_PAREN)) {
        RmbAstExpr* expr = rmb_parse_expr(parser);
        parser_consume(parser, RMB_TOKEN_R_PAREN, ")");
        return expr;
    }

    // Error
    rmb_diag_error_at(token->span, "expected expression");
    parser->had_error = true;
    parser_synchronize(parser);
    return rmb_ast_expr_ident(token->span, rmb_string_from_cstr("error"));
}

// Parse a statement
RmbAstStmt* rmb_parse_stmt(RmbParser* parser) {
    RmbToken* start = parser_current(parser);

    // Variable declaration
    if (parser_check(parser, RMB_TOKEN_IDENT)) {
        RmbToken* lookahead = parser_peek(parser, 1);
        if (lookahead->kind == RMB_TOKEN_COLON_EQ || lookahead->kind == RMB_TOKEN_COLON) {
            // name := expr or name: Type = expr
            RmbToken* name = parser_consume(parser, RMB_TOKEN_IDENT, "variable name");

            if (parser_match(parser, RMB_TOKEN_COLON_EQ)) {
                // Inferred type: name := expr
                RmbAstExpr* value = rmb_parse_expr(parser);
                parser_consume(parser, RMB_TOKEN_SEMI, ";");
                return rmb_ast_stmt_var(
                    span_union(name->span,parser_current(parser)->span),
                    name->lexeme,
                    NULL,  // inferred type
                    value
                );
            } else if (parser_match(parser, RMB_TOKEN_COLON)) {
                // Explicit type: name: Type = expr
                RmbAstTypeRef* type = rmb_parse_type(parser);
                parser_consume(parser, RMB_TOKEN_EQ, "=");
                RmbAstExpr* value = rmb_parse_expr(parser);
                parser_consume(parser, RMB_TOKEN_SEMI, ";");
                return rmb_ast_stmt_var(
                    span_union(name->span,parser_current(parser)->span),
                    name->lexeme,
                    type,
                    value
                );
            }
        }
    }

    // Assignment statement
    if (parser_check(parser, RMB_TOKEN_IDENT) || parser_check(parser, RMB_TOKEN_STAR)) {
        // Try to parse as assignment
        // Note: This is simplified - real assignment parsing would handle lvalues
        RmbAstExpr* target = rmb_parse_expr(parser);

        if (parser_check(parser, RMB_TOKEN_EQ) || parser_check(parser, RMB_TOKEN_PLUS_EQ) ||
            parser_check(parser, RMB_TOKEN_MINUS_EQ) || parser_check(parser, RMB_TOKEN_STAR_EQ) ||
            parser_check(parser, RMB_TOKEN_SLASH_EQ)) {
            RmbTokenKind op = parser_current(parser)->kind;
            parser_advance(parser);
            RmbAstExpr* value = rmb_parse_expr(parser);
            parser_consume(parser, RMB_TOKEN_SEMI, ";");
            return rmb_ast_stmt_assign(
                span_union(target->span,parser_current(parser)->span),
                target,
                op,
                value
            );
        }

        // Not an assignment, rewind and try as expression statement
        parser->pos = (start - parser->tokens); // Reset to start
    }

    // Return statement
    if (parser_match(parser, RMB_TOKEN_KW_RETURN)) {
        RmbAstExpr* value = NULL;
        if (!parser_check(parser, RMB_TOKEN_SEMI)) {
            value = rmb_parse_expr(parser);
        }
        parser_consume(parser, RMB_TOKEN_SEMI, ";");
        return rmb_ast_stmt_return(
            span_union(start->span,parser_current(parser)->span),
            value
        );
    }

    // If statement
    if (parser_match(parser, RMB_TOKEN_KW_IF)) {
        parser_consume(parser, RMB_TOKEN_L_PAREN, "(");
        RmbAstExpr* cond = rmb_parse_expr(parser);
        parser_consume(parser, RMB_TOKEN_R_PAREN, ")");

        parser_consume(parser, RMB_TOKEN_L_BRACE, "{");
        RmbAstStmt** then_body = NULL;
        size_t then_count = 0;

        while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
            RmbAstStmt* stmt = rmb_parse_stmt(parser);
            if (stmt) {
                // Simple dynamic array for statements
                size_t new_size = (then_count + 1) * sizeof(RmbAstStmt*);
                RmbAstStmt** new_body = rmb_arena_alloc(parser->arena, new_size);
                if (new_body) {
                    if (then_body) {
                        memcpy(new_body, then_body, then_count * sizeof(RmbAstStmt*));
                    }
                    new_body[then_count] = stmt;
                    then_body = new_body;
                    then_count++;
                }
            }
        }
        parser_consume(parser, RMB_TOKEN_R_BRACE, "}");

        RmbAstStmt** else_body = NULL;
        size_t else_count = 0;

        if (parser_match(parser, RMB_TOKEN_KW_ELSE)) {
            if (parser_match(parser, RMB_TOKEN_KW_IF)) {
                // else if - parse as a single statement in else block
                parser->pos--; // Back up to 'else' token
                RmbAstStmt* else_if_stmt = rmb_parse_stmt(parser);
                if (else_if_stmt) {
                    else_body = rmb_arena_alloc(parser->arena, sizeof(RmbAstStmt*));
                    if (else_body) {
                        else_body[0] = else_if_stmt;
                        else_count = 1;
                    }
                }
            } else {
                parser_consume(parser, RMB_TOKEN_L_BRACE, "{");
                while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
                    RmbAstStmt* stmt = rmb_parse_stmt(parser);
                    if (stmt) {
                        size_t new_size = (else_count + 1) * sizeof(RmbAstStmt*);
                        RmbAstStmt** new_body = rmb_arena_alloc(parser->arena, new_size);
                        if (new_body) {
                            if (else_body) {
                                memcpy(new_body, else_body, else_count * sizeof(RmbAstStmt*));
                            }
                            new_body[else_count] = stmt;
                            else_body = new_body;
                            else_count++;
                        }
                    }
                }
                parser_consume(parser, RMB_TOKEN_R_BRACE, "}");
            }
        }

        return rmb_ast_stmt_if(
            span_union(start->span,parser_current(parser)->span),
            cond,
            then_body,
            then_count,
            else_body,
            else_count
        );
    }

    // While statement
    if (parser_match(parser, RMB_TOKEN_KW_WHILE)) {
        parser_consume(parser, RMB_TOKEN_L_PAREN, "(");
        RmbAstExpr* cond = rmb_parse_expr(parser);
        parser_consume(parser, RMB_TOKEN_R_PAREN, ")");

        parser_consume(parser, RMB_TOKEN_L_BRACE, "{");
        RmbAstStmt** body = NULL;
        size_t body_count = 0;

        while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
            RmbAstStmt* stmt = rmb_parse_stmt(parser);
            if (stmt) {
                size_t new_size = (body_count + 1) * sizeof(RmbAstStmt*);
                RmbAstStmt** new_body = rmb_arena_alloc(parser->arena, new_size);
                if (new_body) {
                    if (body) {
                        memcpy(new_body, body, body_count * sizeof(RmbAstStmt*));
                    }
                    new_body[body_count] = stmt;
                    body = new_body;
                    body_count++;
                }
            }
        }
        parser_consume(parser, RMB_TOKEN_R_BRACE, "}");

        return rmb_ast_stmt_while(
            span_union(start->span,parser_current(parser)->span),
            cond,
            body,
            body_count
        );
    }

    // For statement (simplified)
    if (parser_match(parser, RMB_TOKEN_KW_FOR)) {
        parser_consume(parser, RMB_TOKEN_L_PAREN, "(");

        RmbAstStmt* init = NULL;
        if (!parser_check(parser, RMB_TOKEN_SEMI)) {
            init = rmb_parse_stmt(parser);
        } else {
            parser_advance(parser); // Skip empty init
        }

        RmbAstExpr* cond = NULL;
        if (!parser_check(parser, RMB_TOKEN_SEMI)) {
            cond = rmb_parse_expr(parser);
        }
        parser_consume(parser, RMB_TOKEN_SEMI, ";");

        RmbAstExpr* step = NULL;
        if (!parser_check(parser, RMB_TOKEN_R_PAREN)) {
            step = rmb_parse_expr(parser);
        }
        parser_consume(parser, RMB_TOKEN_R_PAREN, ")");

        parser_consume(parser, RMB_TOKEN_L_BRACE, "{");
        RmbAstStmt** body = NULL;
        size_t body_count = 0;

        while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
            RmbAstStmt* stmt = rmb_parse_stmt(parser);
            if (stmt) {
                size_t new_size = (body_count + 1) * sizeof(RmbAstStmt*);
                RmbAstStmt** new_body = rmb_arena_alloc(parser->arena, new_size);
                if (new_body) {
                    if (body) {
                        memcpy(new_body, body, body_count * sizeof(RmbAstStmt*));
                    }
                    new_body[body_count] = stmt;
                    body = new_body;
                    body_count++;
                }
            }
        }
        parser_consume(parser, RMB_TOKEN_R_BRACE, "}");

        return rmb_ast_stmt_for(
            span_union(start->span,parser_current(parser)->span),
            init,
            cond,
            step,
            body,
            body_count
        );
    }

    // Match statement
    if (parser_match(parser, RMB_TOKEN_KW_MATCH)) {
        parser_consume(parser, RMB_TOKEN_L_PAREN, "expected '(' after 'match'");
        RmbAstExpr* value = rmb_parse_expr(parser);
        parser_consume(parser, RMB_TOKEN_R_PAREN, "expected ')' after match expression");

        parser_consume(parser, RMB_TOKEN_L_BRACE, "expected '{' before match cases");
        RmbAstMatchCase* cases = NULL;
        RmbAstMatchCase* last_case = NULL;

        while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
            if (!parser_match(parser, RMB_TOKEN_KW_CASE)) {
                rmb_diag_error_at(parser_current(parser)->span, "expected 'case'");
                parser->had_error = true;
                // Recover to next case or }
                while (!parser_check(parser, RMB_TOKEN_KW_CASE) &&
                       !parser_check(parser, RMB_TOKEN_R_BRACE) &&
                       !parser_check(parser, RMB_TOKEN_EOF)) {
                    parser_advance(parser);
                }
                continue;
            }

            RmbToken* case_name = parser_consume(parser, RMB_TOKEN_IDENT, "case name after 'case'");

            // Parse optional binding list: (a, b)
            RmbAstMatchCaseBinding* bindings = NULL;
            RmbAstMatchCaseBinding* last_binding = NULL;
            if (parser_match(parser, RMB_TOKEN_L_PAREN)) {
                do {
                    RmbToken* binding_name = parser_consume(parser, RMB_TOKEN_IDENT, "binding name in case pattern");
                    RmbAstMatchCaseBinding* binding = rmb_ast_match_case_binding(
                        binding_name->span, binding_name->lexeme);
                    if (!bindings) {
                        bindings = binding;
                        last_binding = binding;
                    } else {
                        last_binding->next = binding;
                        last_binding = binding;
                    }
                } while (parser_match(parser, RMB_TOKEN_COMMA));
                parser_consume(parser, RMB_TOKEN_R_PAREN, "expected ')' after case bindings");
            }

            parser_consume(parser, RMB_TOKEN_L_BRACE, "expected '{' before case body");

            // Parse case body statements
            RmbAstStmt** body = NULL;
            size_t body_count = 0;

            while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
                RmbAstStmt* stmt = rmb_parse_stmt(parser);
                if (stmt) {
                    size_t new_size = (body_count + 1) * sizeof(RmbAstStmt*);
                    RmbAstStmt** new_body = rmb_arena_alloc(parser->arena, new_size);
                    if (new_body) {
                        if (body) {
                            memcpy(new_body, body, body_count * sizeof(RmbAstStmt*));
                        }
                        new_body[body_count] = stmt;
                        body = new_body;
                        body_count++;
                    }
                }
            }
            parser_consume(parser, RMB_TOKEN_R_BRACE, "}");

            RmbAstMatchCase* case_node = rmb_ast_match_case(
                case_name->span, case_name->lexeme, bindings, body, body_count);
            if (case_node) {
                if (!cases) {
                    cases = case_node;
                    last_case = case_node;
                } else {
                    last_case->next = case_node;
                    last_case = case_node;
                }
            }
        }
        parser_consume(parser, RMB_TOKEN_R_BRACE, "}");

        return rmb_ast_stmt_match(
            span_union(start->span,parser_current(parser)->span),
            value, cases
        );
    }

    // Defer statement
    if (parser_match(parser, RMB_TOKEN_KW_DEFER)) {
        RmbAstExpr* expr = rmb_parse_expr(parser);
        parser_consume(parser, RMB_TOKEN_SEMI, ";");
        return rmb_ast_stmt_defer(
            span_union(start->span,parser_current(parser)->span),
            expr
        );
    }

    // Expression statement
    RmbAstExpr* expr = rmb_parse_expr(parser);
    if (expr) {
        parser_consume(parser, RMB_TOKEN_SEMI, ";");
        return rmb_ast_stmt_expr(
            span_union(expr->span,parser_current(parser)->span),
            expr
        );
    }

    // Error
    rmb_diag_error_at(start->span, "expected statement");
    parser->had_error = true;
    parser_synchronize(parser);
    return rmb_ast_stmt_expr(start->span, rmb_ast_expr_ident(start->span, rmb_string_from_cstr("error")));
}

// Parse function parameter list
static RmbAstParam* rmb_parse_params(RmbParser* parser) {
    RmbAstParam* first = NULL;
    RmbAstParam* last = NULL;

    if (!parser_check(parser, RMB_TOKEN_R_PAREN)) {
        do {
            RmbToken* name = parser_consume(parser, RMB_TOKEN_IDENT, "parameter name");

            RmbAstTypeRef* type = NULL;
            // Parameters can be: name (inferred) or name Type (explicit)
            // Check if next token could start a type (ident, *, [, etc.)
            if (parser_check(parser, RMB_TOKEN_IDENT) ||
                parser_check(parser, RMB_TOKEN_STAR) ||
                parser_check(parser, RMB_TOKEN_L_BRACKET)) {
                // Try to parse as type
                type = rmb_parse_type(parser);
            }

            RmbAstParam* param = rmb_arena_alloc(parser->arena, sizeof(RmbAstParam));
            if (param) {
                param->name = name->lexeme;
                param->type = type;
                param->span = name->span;
                param->next = NULL;

                if (!first) {
                    first = param;
                    last = param;
                } else {
                    last->next = param;
                    last = param;
                }
            }
        } while (parser_match(parser, RMB_TOKEN_COMMA));
    }

    return first;
}

// Parse struct field list
static RmbAstField* rmb_parse_fields(RmbParser* parser) {
    RmbAstField* first = NULL;
    RmbAstField* last = NULL;

    while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
        RmbToken* name = parser_consume(parser, RMB_TOKEN_IDENT, "field name");
        // No colon, just type name
        RmbAstTypeRef* type = rmb_parse_type(parser);
        parser_consume(parser, RMB_TOKEN_SEMI, ";");

        RmbAstField* field = rmb_arena_alloc(parser->arena, sizeof(RmbAstField));
        if (field) {
            field->name = name->lexeme;
            field->type = type;
            field->span = name->span;
            field->next = NULL;

            if (!first) {
                first = field;
                last = field;
            } else {
                last->next = field;
                last = field;
            }
        }
    }

    return first;
}

// Parse enum variant list
static RmbAstVariant* rmb_parse_variants(RmbParser* parser) {
    RmbAstVariant* first = NULL;
    RmbAstVariant* last = NULL;

    while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
        RmbToken* name = parser_consume(parser, RMB_TOKEN_IDENT, "variant name in enum");

        RmbAstVariantField* fields = NULL;
        RmbAstVariantField* last_field = NULL;
        if (parser_match(parser, RMB_TOKEN_L_PAREN)) {
            // Parse field list: (field_name type, field_name type)
            do {
                RmbToken* field_name = parser_consume(parser, RMB_TOKEN_IDENT, "field name in enum variant");
                RmbAstTypeRef* field_type = rmb_parse_type(parser);
                RmbAstVariantField* field = rmb_ast_variant_field(
                    span_union(field_name->span, field_type->span),
                    field_name->lexeme, field_type);

                if (!fields) {
                    fields = field;
                    last_field = field;
                } else {
                    last_field->next = field;
                    last_field = field;
                }
            } while (parser_match(parser, RMB_TOKEN_COMMA));

            parser_consume(parser, RMB_TOKEN_R_PAREN, "expected ')' after variant fields");
        }

        parser_consume(parser, RMB_TOKEN_SEMI, "expected ';' after variant");

        RmbAstVariant* variant = rmb_ast_variant(name->span, name->lexeme, fields);
        if (variant) {
            if (!first) {
                first = variant;
                last = variant;
            } else {
                last->next = variant;
                last = variant;
            }
        }
    }

    return first;
}

// Parse a top-level item
RmbAstItem* rmb_parse_item(RmbParser* parser) {
    RmbToken* start = parser_current(parser);

    // Use declaration
    if (parser_match(parser, RMB_TOKEN_KW_USE)) {
        RmbToken* parts[8];
        size_t part_count = 0;
        RmbToken* first = parser_consume(parser, RMB_TOKEN_IDENT, "module path");
        parts[part_count++] = first;
        while (parser_match(parser, RMB_TOKEN_DOT)) {
            if (part_count >= 8) {
                rmb_diag_error_at(parser_current(parser)->span, "module path is too deep");
                parser->had_error = true;
                break;
            }
            parts[part_count++] = parser_consume(parser, RMB_TOKEN_IDENT, "module path segment");
        }
        rmb_string path = part_count > 1
            ? join_with_dots(parser, parts, 0, part_count)
            : first->lexeme;
        parser_consume(parser, RMB_TOKEN_SEMI, ";");
        return rmb_ast_item_use(
            span_union(start->span,parser_current(parser)->span),
            path
        );
    }

    // Public modifier
    bool is_pub = false;
    if (parser_match(parser, RMB_TOKEN_KW_PUB)) {
        is_pub = true;
    }

    // Function declaration
    if (parser_match(parser, RMB_TOKEN_KW_FN)) {
        RmbToken* name = parser_consume(parser, RMB_TOKEN_IDENT, "function name after 'fn'");

        parser_consume(parser, RMB_TOKEN_L_PAREN, "'(' after function name");
        RmbAstParam* params = rmb_parse_params(parser);
        parser_consume(parser, RMB_TOKEN_R_PAREN, "')' after parameter list");

        RmbAstTypeRef* return_type = NULL;
        // Return type comes directly after parameters (no colon)
        if (!parser_check(parser, RMB_TOKEN_L_BRACE) && !parser_check(parser, RMB_TOKEN_BANG_BANG)) {
            return_type = rmb_parse_type(parser);
        }

        RmbAstTypeRef* error_type = NULL;
        if (parser_match(parser, RMB_TOKEN_BANG_BANG)) {
            error_type = rmb_parse_type(parser);
        }

        parser_consume(parser, RMB_TOKEN_L_BRACE, "{");
        RmbAstStmt** body = NULL;
        size_t body_count = 0;

        while (!parser_check(parser, RMB_TOKEN_R_BRACE) && !parser_check(parser, RMB_TOKEN_EOF)) {
            size_t pos_before = parser->pos;
            RmbAstStmt* stmt = rmb_parse_stmt(parser);
            if (stmt) {
                size_t new_size = (body_count + 1) * sizeof(RmbAstStmt*);
                RmbAstStmt** new_body = rmb_arena_alloc(parser->arena, new_size);
                if (new_body) {
                    if (body) {
                        memcpy(new_body, body, body_count * sizeof(RmbAstStmt*));
                    }
                    new_body[body_count] = stmt;
                    body = new_body;
                    body_count++;
                }
            }
            // Recovery guard
            if (parser->pos == pos_before && !parser_check(parser, RMB_TOKEN_R_BRACE) &&
                !parser_check(parser, RMB_TOKEN_EOF)) {
                parser_advance(parser);
            }
        }
        parser_consume(parser, RMB_TOKEN_R_BRACE, "}");

        return rmb_ast_item_fn(
            span_union(start->span,parser_current(parser)->span),
            is_pub,
            name->lexeme,
            params,
            return_type,
            error_type,
            body,
            body_count
        );
    }

    // Struct declaration
    if (parser_match(parser, RMB_TOKEN_KW_STRUCT)) {
        RmbToken* name = parser_consume(parser, RMB_TOKEN_IDENT, "struct name after 'struct'");

        parser_consume(parser, RMB_TOKEN_L_BRACE, "expected '{' before struct body");
        RmbAstField* fields = rmb_parse_fields(parser);
        parser_consume(parser, RMB_TOKEN_R_BRACE, "expected '}' after struct fields");

        return rmb_ast_item_struct(
            span_union(start->span,parser_current(parser)->span),
            is_pub,
            name->lexeme,
            fields
        );
    }

    // Enum declaration
    if (parser_match(parser, RMB_TOKEN_KW_ENUM)) {
        RmbToken* name = parser_consume(parser, RMB_TOKEN_IDENT, "enum name after 'enum'");

        parser_consume(parser, RMB_TOKEN_L_BRACE, "expected '{' before enum body");
        RmbAstVariant* variants = rmb_parse_variants(parser);
        parser_consume(parser, RMB_TOKEN_R_BRACE, "expected '}' after enum variants");

        return rmb_ast_item_enum(
            span_union(start->span,parser_current(parser)->span),
            is_pub,
            name->lexeme,
            variants
        );
    }

    // Error
    if (!is_pub) {
        rmb_diag_error_at(start->span, "expected top-level item (fn, struct, enum, use)");
        parser->had_error = true;
    } else {
        rmb_diag_error_at(start->span, "expected fn, struct, or enum after 'pub'");
        parser->had_error = true;
    }

    parser_synchronize(parser);
    return NULL;
}

// Parse entire file
RmbAstFile* rmb_parse_file(RmbParser* parser, const char* path) {
    RmbAstItem* first = NULL;
    RmbAstItem* last = NULL;
    size_t item_count = 0;

    while (!parser_check(parser, RMB_TOKEN_EOF)) {
        size_t pos_before = parser->pos;
        RmbAstItem* item = rmb_parse_item(parser);
        if (item) {
            if (!first) {
                first = item;
                last = item;
            } else {
                last->next = item;
                last = item;
            }
            item_count++;
        }

        // Skip any stray semicolons
        while (parser_match(parser, RMB_TOKEN_SEMI)) {}

        // Recovery guard: if no progress was made, force advance to avoid infinite loop
        if (parser->pos == pos_before && !parser_check(parser, RMB_TOKEN_EOF)) {
            parser_advance(parser);
        }
    }

    return rmb_ast_file_create(path, first, item_count);
}
