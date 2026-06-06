#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char peek(Lexer* lx) {
    return lx->src[lx->pos];
}

static char peek2(Lexer* lx) {
    return lx->src[lx->pos] ? lx->src[lx->pos + 1] : '\0';
}

static char advance(Lexer* lx) {
    char c = lx->src[lx->pos];
    if (c == '\0') return '\0';

    lx->pos++;
    if (c == '\n') {
        lx->line++;
        lx->col = 1;
    } else {
        lx->col++;
    }
    return c;
}

static Token make_tok(TokenKind kind, const char* lex, int line, int col) {
    Token t;
    t.kind = kind;
    t.line = line;
    t.col = col;
    snprintf(t.lexeme, sizeof(t.lexeme), "%s", lex);
    return t;
}

void lexer_init(Lexer* lx, const char* source) {
    lx->src = source;
    lx->pos = 0;
    lx->line = 1;
    lx->col = 1;
    lx->error_line = 0;
    lx->error_col = 0;
    lx->error_message[0] = '\0';
}

static void skip_ws_and_comments(Lexer* lx) {
    for (;;) {
        char c = peek(lx);

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lx);
            continue;
        }

        if (c == '/' && peek2(lx) == '/') {
            advance(lx);
            advance(lx);
            while (peek(lx) != '\n' && peek(lx) != '\0') advance(lx);
            continue;
        }

        if (c == '/' && peek2(lx) == '*') {
            int comment_line = lx->line;
            int comment_col = lx->col;
            advance(lx);
            advance(lx);
            while (!(peek(lx) == '*' && peek2(lx) == '/') && peek(lx) != '\0') {
                advance(lx);
            }
            if (peek(lx) == '\0') {
                lx->error_line = comment_line;
                lx->error_col = comment_col;
                snprintf(lx->error_message, sizeof(lx->error_message),
                         "unterminated block comment");
                return;
            }
            advance(lx);
            advance(lx);
            continue;
        }

        break;
    }
}

static TokenKind keyword_or_id(const char* s) {
    if (strcmp(s, "if") == 0) return TK_IF;
    if (strcmp(s, "else") == 0) return TK_ELSE;
    if (strcmp(s, "while") == 0) return TK_WHILE;
    return TK_ID;
}

Token lexer_next(Lexer* lx) {
    skip_ws_and_comments(lx);

    if (lx->error_message[0] != '\0') {
        Token t = make_tok(TK_ERR, lx->error_message, lx->error_line, lx->error_col);
        lx->error_message[0] = '\0';
        return t;
    }

    int start_line = lx->line;
    int start_col = lx->col;
    char c = peek(lx);

    if (c == '\0') return make_tok(TK_EOF, "", start_line, start_col);

    if (isalpha((unsigned char)c) || c == '_') {
        char buf[128];
        size_t k = 0;
        while (isalnum((unsigned char)peek(lx)) || peek(lx) == '_') {
            if (k + 1 < sizeof(buf)) buf[k++] = advance(lx);
            else advance(lx);
        }
        buf[k] = '\0';
        return make_tok(keyword_or_id(buf), buf, start_line, start_col);
    }

    if (isdigit((unsigned char)c)) {
        char buf[128];
        size_t k = 0;
        while (isdigit((unsigned char)peek(lx))) {
            if (k + 1 < sizeof(buf)) buf[k++] = advance(lx);
            else advance(lx);
        }
        buf[k] = '\0';
        return make_tok(TK_INT, buf, start_line, start_col);
    }

    if (c == '=' && peek2(lx) == '=') {
        advance(lx);
        advance(lx);
        return make_tok(TK_EQ, "==", start_line, start_col);
    }
    if (c == '!' && peek2(lx) == '=') {
        advance(lx);
        advance(lx);
        return make_tok(TK_NE, "!=", start_line, start_col);
    }
    if (c == '<' && peek2(lx) == '=') {
        advance(lx);
        advance(lx);
        return make_tok(TK_LE, "<=", start_line, start_col);
    }
    if (c == '>' && peek2(lx) == '=') {
        advance(lx);
        advance(lx);
        return make_tok(TK_GE, ">=", start_line, start_col);
    }

    switch (c) {
        case '=': advance(lx); return make_tok(TK_ASSIGN, "=", start_line, start_col);
        case '+': advance(lx); return make_tok(TK_PLUS, "+", start_line, start_col);
        case '-': advance(lx); return make_tok(TK_MINUS, "-", start_line, start_col);
        case '*': advance(lx); return make_tok(TK_MUL, "*", start_line, start_col);
        case '/': advance(lx); return make_tok(TK_DIV, "/", start_line, start_col);
        case '<': advance(lx); return make_tok(TK_LT, "<", start_line, start_col);
        case '>': advance(lx); return make_tok(TK_GT, ">", start_line, start_col);
        case '(': advance(lx); return make_tok(TK_LPAREN, "(", start_line, start_col);
        case ')': advance(lx); return make_tok(TK_RPAREN, ")", start_line, start_col);
        case '{': advance(lx); return make_tok(TK_LBRACE, "{", start_line, start_col);
        case '}': advance(lx); return make_tok(TK_RBRACE, "}", start_line, start_col);
        case ';': advance(lx); return make_tok(TK_SEMI, ";", start_line, start_col);
        case ',': advance(lx); return make_tok(TK_COMMA, ",", start_line, start_col);
        default: {
            char buf[2] = {c, '\0'};
            advance(lx);
            return make_tok(TK_ERR, buf, start_line, start_col);
        }
    }
}
