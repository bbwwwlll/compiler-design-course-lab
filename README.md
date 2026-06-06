# 编译方法课程实验

## 项目简介

本项目为编译方法课程实验代码整理，使用 C 语言实现了一个简单的词法分析器和递归下降语法分析器。

- 词法分析器读取源程序字符流，输出 token 序列。
- 语法分析器读取源程序字符流，按自定义 CFG 进行自顶向下分析，输出推导与匹配过程。
- 程序包含基础错误处理，例如非法字符、未闭合块注释、语法缺失符号等。

## 实验要求概述

实验要求来自 `4_课程实验要求.pdf`：

- Task 1：Lexical Analyzer Programming。
- Task 2：Syntax Parser Programming。
- 编程语言不限，建议使用 C 或 Java。
- 词法分析输入为字符流和自定义正则表达式集合，输出为 token 序列，单词类别由实现者定义，错误处理可包含。
- 语法分析输入为字符流和自定义 CFG，若采用自顶向下方法需输出推导序列，若采用自底向上方法需输出归约序列，句子类别由实现者定义，错误处理可包含。
- 提交材料包括实验报告和源代码，报告需包含实验目的、内容、方法、假设、FA 描述、数据结构、核心算法、运行用例、问题与解决、心得等；源代码目录需包含输入文件、输出文件、源程序文件、报告文件及其他相关文件。

## 目录结构

```text
.
├── 4_课程实验要求.pdf        # 实验要求文档
├── lexer.c / lexer.h          # 词法分析器
├── parser.c / parser.h        # 递归下降语法分析器
├── token.h                    # token 类型和输出格式
├── test_lexer.c               # 词法分析命令行入口
├── test_parser.c              # 语法分析命令行入口
├── tests/                     # 拆分后的测试输入
├── tests_input_lexer.txt      # 原始词法测试输入
├── tests_input_parser.txt     # 原始语法测试输入
├── Makefile                   # 构建脚本
└── README.md
```

## 环境依赖

- C 编译器：GCC、Clang 或兼容 C11 的编译器。
- Make：可选。没有 Make 时可直接使用 gcc 命令编译。

## 构建方法

使用 Make：

```sh
make
```

或直接使用 gcc：

```sh
gcc -std=c11 -Wall -Wextra -pedantic lexer.c test_lexer.c -o test_lexer
gcc -std=c11 -Wall -Wextra -pedantic lexer.c parser.c test_parser.c -o test_parser
```

Windows 下可将输出名改为 `test_lexer.exe`、`test_parser.exe`。

## 运行方法

词法分析：

```sh
./test_lexer tests/lexer_valid.txt output/lexer_valid.out
```

语法分析：

```sh
./test_parser tests/parser_valid.txt output/parser_valid.out
```

Windows PowerShell 示例：

```powershell
New-Item -ItemType Directory -Force output
.\test_lexer.exe tests\lexer_valid.txt output\lexer_valid.out
.\test_parser.exe tests\parser_valid.txt output\parser_valid.out
```

## 测试样例

测试样例位于 `tests/`：

- `lexer_valid.txt`：合法词法输入。
- `lexer_error.txt`：包含非法字符 `@`。
- `lexer_unclosed_comment.txt`：包含未闭合块注释。
- `parser_valid.txt`：合法语法输入。
- `parser_error_missing_semi.txt`：缺少分号。
- `parser_error_bad_char.txt`：语法分析前遇到非法字符。

原始混合测试文件 `tests_input_lexer.txt` 和 `tests_input_parser.txt` 仍保留，便于对照课程实验过程。

## 输出说明

词法分析输出每个 token 的行列号、类型和原始词素，例如：

```text
1:1       LBRACE     {
2:3       ID         x
```

语法分析输出自顶向下分析过程，包括产生式展开、终结符匹配和最终结果：

```text
[Step 1] expand: P -> StmtList EOF
[Step 2] expand: StmtList -> Stmt StmtList
...
Parse OK.
```

若出错，程序会在标准错误和输出文件中写入错误位置及原因。

## 实验完成情况

- 已完成 Task 1 词法分析：支持标识符、整数、关键字、运算符、分隔符、空白和注释跳过，输出 token 序列。
- 已完成 Task 2 语法分析：使用递归下降方法分析赋值语句、`if-else`、`while`、块语句和表达式，输出推导与匹配过程。
- 已包含基础错误处理：非法字符、未闭合块注释、语法符号缺失等。
- 未实现语义分析、中间代码生成、符号表管理；PDF 未将这些内容列为本实验 Task 1/Task 2 的必做项。

## 注意事项

- 源码和测试文件建议使用 UTF-8 编码。
- 程序不依赖硬编码绝对路径，输入和输出文件均由命令行参数指定。
- `output/` 为运行输出目录，已加入 `.gitignore`。
- PDF 要求的实验报告未包含在当前整理结果中，如课程提交需要，请另行补充报告文件。

## 课程说明

本仓库为编译方法课程实验代码整理，仅用于学习与交流。
