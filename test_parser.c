#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

/* 读整个文件到内存 */
static char* read_all_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t rd = fread(buf, 1, (size_t)n, f);
    buf[rd] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.txt> <output_derivations.txt>\n", argv[0]);
        return 1;
    }

    char* source = read_all_file(argv[1]);
    if (!source) {
        fprintf(stderr, "Failed to open input file: %s\n", argv[1]);
        return 1;
    }

    FILE* out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "Failed to open output file: %s\n", argv[2]);
        free(source);
        return 1;
    }

    Parser ps;
    parser_init(&ps, source, out);

    int rc = parser_parse_program(&ps);

    fclose(out);
    free(source);
    return rc;
}
