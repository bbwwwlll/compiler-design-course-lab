#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

/* ===== 打印缩进（让推导过程更清晰） ===== */
static void indent(Parser* ps) {
    for (int i = 0; i < ps->depth; i++) fprintf(ps->out, "  ");
}

/* ===== 错误处理：直接在输出文件+stderr 写明信息 ===== */
static int parse_error(Parser* ps, const char* msg) {
    fprintf(stderr, "ParseError at %d:%d: %s (got %s '%s')\n",
            ps->look.line, ps->look.col, msg, token_kind_name(ps->look.kind), ps->look.lexeme);
    fprintf(ps->out, "ParseError at %d:%d: %s (got %s '%s')\n",
            ps->look.line, ps->look.col, msg, token_kind_name(ps->look.kind), ps->look.lexeme);
    return 2;
}

static int lex_error(Parser* ps, const char* msg) {
    fprintf(stderr, "LexError at %d:%d: %s (bad '%s')\n",
            ps->look.line, ps->look.col, msg, ps->look.lexeme);
    fprintf(ps->out, "LexError at %d:%d: %s (bad '%s')\n",
            ps->look.line, ps->look.col, msg, ps->look.lexeme);
    return 3;
}

static int advance_tok(Parser* ps) {
    ps->look = lexer_next(&ps->lx);
    if (ps->look.kind == TK_ERR) return lex_error(ps, "invalid character");
    return 0;
}

static int match(Parser* ps, TokenKind expected) {
    if (ps->look.kind != expected) {
        char buf[128];
        snprintf(buf, sizeof(buf), "expected %s", token_kind_name(expected));
        return parse_error(ps, buf);
    }
    indent(ps);
    fprintf(ps->out, "[Step %d] match: %-10s lexeme='%s'\n",
            ps->step++, token_kind_name(expected), ps->look.lexeme);
    return advance_tok(ps);
}

/* ====== 非终结符声明 ====== */
static int parse_StmtList(Parser* ps);
static int parse_Stmt(Parser* ps);
static int parse_Assign(Parser* ps);
static int parse_If(Parser* ps);
static int parse_ElseOpt(Parser* ps);
static int parse_While(Parser* ps);
static int parse_Block(Parser* ps);

// Expr family
static int parse_Expr(Parser* ps);
static int parse_RelOpt(Parser* ps);
static int parse_Add(Parser* ps);
static int parse_AddP(Parser* ps);
static int parse_Mul(Parser* ps);
static int parse_MulP(Parser* ps);
static int parse_Unary(Parser* ps);
static int parse_Primary(Parser* ps);

static int is_relop(TokenKind k) {
    return (k == TK_EQ || k == TK_NE || k == TK_LT || k == TK_LE || k == TK_GT || k == TK_GE);
}

void parser_init(Parser* ps, const char* source, FILE* out) {
    lexer_init(&ps->lx, source);
    ps->out = out;
    ps->step = 1;
    ps->depth = 0;

    ps->look = lexer_next(&ps->lx);
    if (ps->look.kind == TK_ERR) {
        // 这里不 exit，让上层返回错误码
    }
}

/* Program: P -> StmtList EOF */
int parser_parse_program(Parser* ps) {
    if (ps->look.kind == TK_ERR) return lex_error(ps, "invalid character");

    indent(ps);
    fprintf(ps->out, "[Step %d] expand: P -> StmtList EOF\n", ps->step++);
    ps->depth++;

    int rc = parse_StmtList(ps);
    if (rc) return rc;

    rc = match(ps, TK_EOF);
    if (rc) return rc;

    ps->depth--;
    fprintf(ps->out, "Parse OK.\n");
    return 0;
}

/* StmtList -> Stmt StmtList | epsilon   (epsilon when '}' or EOF) */
static int parse_StmtList(Parser* ps) {
    if (ps->look.kind == TK_RBRACE || ps->look.kind == TK_EOF) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: StmtList -> epsilon\n", ps->step++);
        return 0;
    }

    indent(ps);
    fprintf(ps->out, "[Step %d] expand: StmtList -> Stmt StmtList\n", ps->step++);
    ps->depth++;

    int rc = parse_Stmt(ps);
    if (rc) return rc;

    rc = parse_StmtList(ps);
    if (rc) return rc;

    ps->depth--;
    return 0;
}

/* Stmt -> Assign | If | While | Block */
static int parse_Stmt(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Stmt -> (Assign|If|While|Block)\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if (ps->look.kind == TK_ID) rc = parse_Assign(ps);
    else if (ps->look.kind == TK_IF) rc = parse_If(ps);
    else if (ps->look.kind == TK_WHILE) rc = parse_While(ps);
    else if (ps->look.kind == TK_LBRACE) rc = parse_Block(ps);
    else rc = parse_error(ps, "statement must start with id/if/while/{");

    ps->depth--;
    return rc;
}

/* Assign -> id '=' Expr ';' */
static int parse_Assign(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Assign -> id '=' Expr ';'\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if ((rc = match(ps, TK_ID))) return rc;
    if ((rc = match(ps, TK_ASSIGN))) return rc;
    if ((rc = parse_Expr(ps))) return rc;
    if ((rc = match(ps, TK_SEMI))) return rc;

    ps->depth--;
    return 0;
}

/* If -> 'if' '(' Expr ')' Stmt ElseOpt */
static int parse_If(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: If -> 'if' '(' Expr ')' Stmt ElseOpt\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if ((rc = match(ps, TK_IF))) return rc;
    if ((rc = match(ps, TK_LPAREN))) return rc;
    if ((rc = parse_Expr(ps))) return rc;
    if ((rc = match(ps, TK_RPAREN))) return rc;
    if ((rc = parse_Stmt(ps))) return rc;
    if ((rc = parse_ElseOpt(ps))) return rc;

    ps->depth--;
    return 0;
}

/* ElseOpt -> 'else' Stmt | epsilon */
static int parse_ElseOpt(Parser* ps) {
    if (ps->look.kind == TK_ELSE) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: ElseOpt -> 'else' Stmt\n", ps->step++);
        ps->depth++;

        int rc = 0;
        if ((rc = match(ps, TK_ELSE))) return rc;
        if ((rc = parse_Stmt(ps))) return rc;

        ps->depth--;
        return 0;
    }

    indent(ps);
    fprintf(ps->out, "[Step %d] expand: ElseOpt -> epsilon\n", ps->step++);
    return 0;
}

/* While -> 'while' '(' Expr ')' Stmt */
static int parse_While(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: While -> 'while' '(' Expr ')' Stmt\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if ((rc = match(ps, TK_WHILE))) return rc;
    if ((rc = match(ps, TK_LPAREN))) return rc;
    if ((rc = parse_Expr(ps))) return rc;
    if ((rc = match(ps, TK_RPAREN))) return rc;
    if ((rc = parse_Stmt(ps))) return rc;

    ps->depth--;
    return 0;
}

/* Block -> '{' StmtList '}' */
static int parse_Block(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Block -> '{' StmtList '}'\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if ((rc = match(ps, TK_LBRACE))) return rc;
    if ((rc = parse_StmtList(ps))) return rc;
    if ((rc = match(ps, TK_RBRACE))) return rc;

    ps->depth--;
    return 0;
}

/* Expr -> Add RelOpt */
static int parse_Expr(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Expr -> Add RelOpt\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if ((rc = parse_Add(ps))) return rc;
    if ((rc = parse_RelOpt(ps))) return rc;

    ps->depth--;
    return 0;
}

/* RelOpt -> relop Add | epsilon */
static int parse_RelOpt(Parser* ps) {
    if (is_relop(ps->look.kind)) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: RelOpt -> relop Add\n", ps->step++);
        ps->depth++;

        TokenKind op = ps->look.kind;
        int rc = 0;
        if ((rc = match(ps, op))) return rc;
        if ((rc = parse_Add(ps))) return rc;

        ps->depth--;
        return 0;
    }

    indent(ps);
    fprintf(ps->out, "[Step %d] expand: RelOpt -> epsilon\n", ps->step++);
    return 0;
}

/* Add -> Mul Add' */
static int parse_Add(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Add -> Mul Add'\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if ((rc = parse_Mul(ps))) return rc;
    if ((rc = parse_AddP(ps))) return rc;

    ps->depth--;
    return 0;
}

/* Add' -> ('+'|'-') Mul Add' | epsilon */
static int parse_AddP(Parser* ps) {
    if (ps->look.kind == TK_PLUS || ps->look.kind == TK_MINUS) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: Add' -> ('+'|'-') Mul Add'\n", ps->step++);
        ps->depth++;

        TokenKind op = ps->look.kind;
        int rc = 0;
        if ((rc = match(ps, op))) return rc;
        if ((rc = parse_Mul(ps))) return rc;
        if ((rc = parse_AddP(ps))) return rc;

        ps->depth--;
        return 0;
    }

    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Add' -> epsilon\n", ps->step++);
    return 0;
}

/* Mul -> Unary Mul' */
static int parse_Mul(Parser* ps) {
    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Mul -> Unary Mul'\n", ps->step++);
    ps->depth++;

    int rc = 0;
    if ((rc = parse_Unary(ps))) return rc;
    if ((rc = parse_MulP(ps))) return rc;

    ps->depth--;
    return 0;
}

/* Mul' -> ('*'|'/') Unary Mul' | epsilon */
static int parse_MulP(Parser* ps) {
    if (ps->look.kind == TK_MUL || ps->look.kind == TK_DIV) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: Mul' -> ('*'|'/') Unary Mul'\n", ps->step++);
        ps->depth++;

        TokenKind op = ps->look.kind;
        int rc = 0;
        if ((rc = match(ps, op))) return rc;
        if ((rc = parse_Unary(ps))) return rc;
        if ((rc = parse_MulP(ps))) return rc;

        ps->depth--;
        return 0;
    }

    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Mul' -> epsilon\n", ps->step++);
    return 0;
}

/* Unary -> ('+'|'-') Unary | Primary */
static int parse_Unary(Parser* ps) {
    if (ps->look.kind == TK_PLUS || ps->look.kind == TK_MINUS) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: Unary -> ('+'|'-') Unary\n", ps->step++);
        ps->depth++;

        TokenKind op = ps->look.kind;
        int rc = 0;
        if ((rc = match(ps, op))) return rc;
        if ((rc = parse_Unary(ps))) return rc;

        ps->depth--;
        return 0;
    }

    indent(ps);
    fprintf(ps->out, "[Step %d] expand: Unary -> Primary\n", ps->step++);
    ps->depth++;
    int rc = parse_Primary(ps);
    ps->depth--;
    return rc;
}

/* Primary -> '(' Expr ')' | id | int */
static int parse_Primary(Parser* ps) {
    if (ps->look.kind == TK_LPAREN) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: Primary -> '(' Expr ')'\n", ps->step++);
        ps->depth++;

        int rc = 0;
        if ((rc = match(ps, TK_LPAREN))) return rc;
        if ((rc = parse_Expr(ps))) return rc;
        if ((rc = match(ps, TK_RPAREN))) return rc;

        ps->depth--;
        return 0;
    }

    if (ps->look.kind == TK_ID) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: Primary -> id\n", ps->step++);
        ps->depth++;
        int rc = match(ps, TK_ID);
        ps->depth--;
        return rc;
    }

    if (ps->look.kind == TK_INT) {
        indent(ps);
        fprintf(ps->out, "[Step %d] expand: Primary -> int\n", ps->step++);
        ps->depth++;
        int rc = match(ps, TK_INT);
        ps->depth--;
        return rc;
    }

    return parse_error(ps, "expected primary: '(', id, or int");
}
