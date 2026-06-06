#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stddef.h>

/* Lexer：只维护扫描状态 */
typedef struct {
    const char* src;
    size_t pos;
    int line;
    int col;
    int error_line;
    int error_col;
    char error_message[64];
} Lexer;

/* 从源码字符串初始化 */
void lexer_init(Lexer* lx, const char* source);

/* 读取下一个 token（遇到非法字符返回 TK_ERR） */
Token lexer_next(Lexer* lx);

#endif
