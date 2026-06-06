#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

/* Token 类型（你可以按实验一/二需要继续扩展） */
typedef enum {
    TK_EOF = 0,

    // 标识符/字面量
    TK_ID,
    TK_INT,

    // 关键字
    TK_IF,
    TK_ELSE,
    TK_WHILE,

    // 运算符
    TK_ASSIGN,      // =
    TK_PLUS, TK_MINUS, TK_MUL, TK_DIV,

    // 比较运算符
    TK_EQ,          // ==
    TK_NE,          // !=
    TK_LT, TK_LE,   // < <=
    TK_GT, TK_GE,   // > >=

    // 分隔符
    TK_LPAREN, TK_RPAREN,
    TK_LBRACE, TK_RBRACE,
    TK_SEMI,
    TK_COMMA,

    TK_ERR
} TokenKind;

/* 一个 token 的信息：类型 + 原文 + 行列号 */
typedef struct {
    TokenKind kind;
    char lexeme[128];
    int line;
    int col;
} Token;

/* 用于输出、调试：把 kind 转成字符串 */
static inline const char* token_kind_name(TokenKind k) {
    switch (k) {
        case TK_EOF: return "EOF";
        case TK_ID: return "ID";
        case TK_INT: return "INT";
        case TK_IF: return "KW_IF";
        case TK_ELSE: return "KW_ELSE";
        case TK_WHILE: return "KW_WHILE";
        case TK_ASSIGN: return "ASSIGN";
        case TK_PLUS: return "PLUS";
        case TK_MINUS: return "MINUS";
        case TK_MUL: return "MUL";
        case TK_DIV: return "DIV";
        case TK_EQ: return "EQ";
        case TK_NE: return "NE";
        case TK_LT: return "LT";
        case TK_LE: return "LE";
        case TK_GT: return "GT";
        case TK_GE: return "GE";
        case TK_LPAREN: return "LPAREN";
        case TK_RPAREN: return "RPAREN";
        case TK_LBRACE: return "LBRACE";
        case TK_RBRACE: return "RBRACE";
        case TK_SEMI: return "SEMI";
        case TK_COMMA: return "COMMA";
        case TK_ERR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/* 统一输出 token 的格式：写入文件 */
static inline void token_fprint(FILE* out, const Token* t) {
    fprintf(out, "%d:%d\t%-10s\t%s\n",
            t->line, t->col, token_kind_name(t->kind), t->lexeme);
}

#endif
