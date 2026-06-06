#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"

/* Parser 结构：持有 lexer + lookahead */
typedef struct {
    Lexer lx;
    Token look;
    int step;
    int depth;
    FILE* out;   // 推导序列输出文件
} Parser;

/* 初始化 parser：从源码字符串开始，推导序列写入 out */
void parser_init(Parser* ps, const char* source, FILE* out);

/* 进行语法分析：成功返回 0，失败返回非 0 */
int parser_parse_program(Parser* ps);

#endif
