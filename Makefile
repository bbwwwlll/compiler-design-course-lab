CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic

.PHONY: all clean test

all: test_lexer test_parser

test_lexer: lexer.c test_lexer.c lexer.h token.h
	$(CC) $(CFLAGS) lexer.c test_lexer.c -o $@

test_parser: lexer.c parser.c test_parser.c lexer.h parser.h token.h
	$(CC) $(CFLAGS) lexer.c parser.c test_parser.c -o $@

test: all
	mkdir -p output
	./test_lexer tests/lexer_valid.txt output/lexer_valid.out
	./test_parser tests/parser_valid.txt output/parser_valid.out

clean:
	rm -f test_lexer test_parser test_lexer.exe test_parser.exe *.o *.out
