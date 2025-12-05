# 正则表达式转词法分析器 (Regex2Lex)

## 目录
1. [软件简介](#软件简介)
2. [功能概述](#功能概述)
3. [使用指南](#使用指南)
4. [正则表达式输入格式](#正则表达式输入格式)
5. [TINY语言词法分析](#tiny语言词法分析)
6. [命令行使用](#命令行使用)
7. [验证结果正确性](#验证结果正确性)
8. [常见问题](#常见问题)

---

## 软件简介

本软件是一个基于Qt6的图形化工具，用于将正则表达式转换为词法分析器。它实现了编译原理中的核心算法：

- **正则表达式 → NFA**：Thompson构造法
- **NFA → DFA**：子集构造法
- **DFA最小化**：等价类划分法
- **生成词法分析程序**：生成完整的TINY语言词法分析器

---

## 功能概述

| 功能按钮 | 说明 |
|---------|------|
| **开始分析** | 解析输入的正则表达式，生成NFA、DFA和最小化DFA |
| **查看NFA** | 显示NFA状态转换表 |
| **查看DFA** | 显示DFA状态转换表 |
| **最小化DFA** | 显示最小化后的DFA状态转换表 |
| **生成代码** | 生成C语言词法分析程序 `_lexer.c` |
| **编译** | 调用gcc编译生成可执行文件 `_lexer` |
| **测试** | 选择 `.tny` 文件进行词法分析测试 |
| **查看LEX文件** | 打开并查看已生成的 `.lex` 词法分析结果文件 |
| **打开文件** | 从文件加载正则表达式 |
| **保存文件** | 保存当前输入的正则表达式到文件 |

---

## 使用指南

### 步骤1：输入正则表达式

在输入区域输入正则表达式，或点击"打开文件"加载已有的正则表达式文件（如 `tiny_regex.txt`）。

### 步骤2：设置选项

如果目标语言不区分大小写（如TINY语言），请勾选"忽略大小写"选项。

### 步骤3：开始分析

点击绿色的"开始分析"按钮，系统将解析正则表达式并生成NFA、DFA和最小化DFA。

### 步骤4：查看状态转换表

分析完成后，可以点击：
- "查看NFA" - 查看NFA状态转换表
- "查看DFA" - 查看DFA状态转换表
- "最小化DFA" - 查看最小化DFA状态转换表

状态转换表说明：
- **标志列**：`-` 表示初态，`+` 表示终态
- **ID列**：状态编号
- **其他列**：输入字符对应的下一状态

### 步骤5：生成词法分析器

1. 点击 **"生成代码"** 按钮
2. 选择输出目录
3. 系统生成 `_lexer.c` 词法分析程序

### 步骤6：编译

1. 点击 **"编译"** 按钮
2. 系统调用 `gcc` 编译生成可执行文件 `_lexer`

### 步骤7：测试

1. 点击 **"测试"** 按钮
2. 在文件选择对话框中选择要分析的 `.tny` 文件
3. 系统自动运行词法分析器并显示结果
4. 生成的 `.lex` 文件保存在与输入文件相同的目录

---

## 正则表达式输入格式

正则表达式采用**变量定义**格式，支持变量引用和展开：

### 基本语法

```
变量名=正则表达式
```

### 规则说明

| 规则 | 说明 | 示例 |
|------|------|------|
| 变量定义 | `name=regex` | `letter=[A-Za-z]` |
| 生成DFA | 以 `_` 开头 | `_ID101=letter(letter\|digit)*` |
| 单词编码 | 名称后的数字 | `_ID101` 表示编码为 101 |
| 多单词标记 | 数字后加 `S` | `_special200S=\+\|-\|\*` 从 200 开始编码 |

### 完整示例

```
# TINY语言词法规则定义
digit=[0-9]
letter=[A-Za-z]
_ID101=letter(letter|digit)*
_NUM102=digit+
```

**说明**：
- 以 `#` 开头的行为注释
- `digit` 和 `letter` 是变量定义
- `_ID101`：标识符，编码 101
- `_NUM102`：数字，编码 102

### 支持的运算符

| 运算符 | 说明 | 示例 |
|--------|------|------|
| `[]` | 字符类 | `[A-Za-z]` 匹配任意字母 |
| `\|` | 选择 | `a\|b` 匹配 a 或 b |
| `*` | 闭包（0次或多次） | `a*` 匹配空串、a、aa... |
| `+` | 正闭包（1次或多次） | `a+` 匹配 a、aa、aaa... |
| `?` | 可选（0次或1次） | `a?` 匹配空串或 a |
| `()` | 分组 | `(ab)+` 匹配 ab、abab... |

### 转义字符

由于 `+`、`*`、`|` 等在正则表达式中有特殊含义，如果要匹配这些字符本身，需要用 `\` 转义：

| 字符 | 转义写法 | 说明 |
|------|----------|------|
| `+` | `\+` | 加号 |
| `*` | `\*` | 乘号 |
| `\|` | `\\|` | 或符号 |
| `(` | `\(` | 左括号 |
| `)` | `\)` | 右括号 |

---

## TINY语言词法分析

生成的词法分析器完整支持 TINY 语言，包括：

### 支持的Token类型

| 类型 | 说明 | 示例 |
|------|------|------|
| **KEYWORD** | 关键字（8个） | if, then, else, end, repeat, until, read, write |
| **ID** | 标识符 | x, fact, myVar |
| **NUM** | 数字 | 0, 123, 456 |
| **OPERATOR** | 运算符 | +, -, *, /, <, >, =, :=, <=, >=, <> |
| **DELIMITER** | 分隔符 | ;, (, ) |

### 特性

- **大小写不敏感**：`IF`、`If`、`if` 都识别为关键字
- **注释处理**：`{ ... }` 形式的注释被自动跳过
- **双字符运算符**：正确识别 `:=`、`<=`、`>=`、`<>`

### 示例正则表达式文件 (tiny_regex.txt)

```
# TINY语言词法规则定义
# 格式: name=regex 或 _name数字=regex (生成DFA的规则)

# 基本字符类定义
letter=[A-Za-z]
digit=[0-9]

# Token规则 (以_开头表示需要生成DFA)
_ID101=letter(letter|digit)*
_NUM102=digit+
```

### TINY测试源程序 (sample.tny)

```tiny
{ Sample TINY program - 计算阶乘 }
read x;
if 0 < x then
  fact := 1;
  repeat
    fact := fact * x;
    x := x - 1
  until x = 0;
  write fact
end
```

### 预期输出格式

```
=== Lexical Analysis Results ===

1: KEYWORD, read
2: ID, x
3: DELIMITER, ;
4: KEYWORD, if
5: NUM, 0
6: OPERATOR, <
7: ID, x
8: KEYWORD, then
9: ID, fact
10: OPERATOR, :=
11: NUM, 1
12: DELIMITER, ;
13: KEYWORD, repeat
14: ID, fact
15: OPERATOR, :=
16: ID, fact
17: OPERATOR, *
18: ID, x
19: DELIMITER, ;
20: ID, x
21: OPERATOR, :=
22: ID, x
23: OPERATOR, -
24: NUM, 1
25: KEYWORD, until
26: ID, x
27: OPERATOR, =
28: NUM, 0
29: DELIMITER, ;
30: KEYWORD, write
31: ID, fact
32: KEYWORD, end
```

---

## 命令行使用

生成的词法分析器 `_lexer` 支持命令行参数：

### 用法

```bash
# 显示帮助
./_lexer

# 分析文件，自动生成同名 .lex 文件
./_lexer sample.tny          # 生成 sample.lex

# 指定输出文件名
./_lexer sample.tny result.lex
```

### 示例

```bash
# 分析 sample.tny
$ ./_lexer sample.tny
Input file:  sample.tny
Output file: sample.lex

=== Lexical Analysis Results ===

1: KEYWORD, read
2: ID, x
3: DELIMITER, ;
...

Tokens saved to output file
```

---

## Mini-C语言词法分析

### 选择语言

在界面右上角的下拉框中选择 **"Mini-C语言"**，然后进行生成代码、编译、测试操作。

### 支持的Token类型

| 类型 | 说明 | 示例 |
|------|------|------|
| **KEYWORD** | 关键字（9个） | else, if, int, float, real, return, void, while, for |
| **ID** | 标识符（以字母或_开头） | x, _count, myVar123 |
| **NUM** | 整数 | 0, 123, 456 |
| **FLOAT** | 浮点数 | 3.14, 0.5, 2.71828 |
| **OPERATOR** | 运算符 | +, -, *, /, %, ^, ., <, >, =, <=, >=, ==, !=, ++, -- |
| **DELIMITER** | 分隔符 | ;, ,, (, ), [, ], {, } |

### 特性

- **大小写敏感**：`Int` 和 `int` 不同（只有 `int` 是关键字）
- **下划线标识符**：支持以 `_` 开头的标识符，如 `_count`
- **浮点数**：支持 `digit+.digit+` 格式
- **行注释**：`// ...` 形式的注释被自动跳过
- **自增自减**：支持 `++` 和 `--` 运算符

### Mini-C 词法规则

```
1) 关键字 (9个，必须小写):
   else, if, int, float, real, return, void, while, for

2) 专用符号:
   + - * / % ^ . < <= > >= == != = ; , ++ -- ( ) [ ] { }
   // 行注释

3) 标识符和数字:
   ID = (_|letter)(_|letter|digit)*
   NUM = digit+(.digit+)?
   letter = [a-zA-Z]
   digit = [0-9]
```

### Mini-C 示例源程序 (sample.mc)

```c
// Mini-C 综合测试程序
// 覆盖所有关键字、运算符和语法结构

int globalVar;
float pi;
real epsilon;
int arr[100];

// 计算阶乘函数
int factorial(int n)
{
    int result;
    result = 1;
    while (n > 0)
    {
        result = result * n;
        n = n - 1;
    }
    return result;
}

// 主函数
int main(void)
{
    int x;
    float f;
    
    x = 5;
    f = 3.14;
    
    // 测试条件语句
    if (x < 10)
    {
        x = x + 1;
    }
    else
    {
        x = x - 1;
    }
    
    // 测试 for 循环
    for (int i = 0; i < 10; ++i)
    {
        arr[i] = i * 2;
    }
    
    // 测试自增自减
    ++x;
    --x;
    
    return 0;
}
```

### 预期输出格式 (sample.lex)

```
=== Mini-C Lexical Analysis Results ===

1: KEYWORD, int
2: ID, globalVar
3: DELIMITER, ;
4: KEYWORD, float
5: ID, pi
6: DELIMITER, ;
7: KEYWORD, real
8: ID, epsilon
9: DELIMITER, ;
10: KEYWORD, int
11: ID, arr
12: DELIMITER, [
13: NUM, 100
14: DELIMITER, ]
15: DELIMITER, ;
16: KEYWORD, int
17: ID, factorial
18: DELIMITER, (
19: KEYWORD, int
20: ID, n
21: DELIMITER, )
...
```

### 命令行使用

```bash
# 分析 Mini-C 文件
./lexer sample.mc          # 生成 sample.lex

# 指定输出文件
./lexer sample.mc result.lex
```

---

## 验证结果正确性

### 验证NFA正确性

1. 检查NFA状态转换表：
   - 应有唯一的初态（标记为 `-`）
   - 应有唯一的终态（标记为 `+`）
   - ε（epsilon）转换用 `#` 表示

2. 手动跟踪示例输入

### 验证DFA正确性

1. DFA应该没有ε转换
2. 每个状态对于每个输入字符最多有一个转换

### 验证最小化DFA正确性

1. 状态数应该小于等于原DFA
2. 功能上等价（接受相同的字符串集合）

### 验证词法分析器正确性

| 输入 | 预期输出 |
|------|----------|
| `if` | `KEYWORD, if` |
| `IF` | `KEYWORD, if` （大小写不敏感）|
| `abc123` | `ID, abc123` |
| `123` | `NUM, 123` |
| `:=` | `OPERATOR, :=` |
| `<=` | `OPERATOR, <=` |
| `{ comment }` | （被跳过，无输出）|

---

## 常见问题

### Q1: 编译失败，提示找不到gcc

**解决**：
```bash
# Ubuntu/Debian
sudo apt install gcc

# Arch Linux
sudo pacman -S gcc

# macOS
xcode-select --install
```

### Q2: 点击"测试"没有反应

**原因**：未先生成代码或编译

**解决**：按顺序点击 "生成代码" → "编译" → "测试"

### Q3: 某些Token没有被正确识别

**原因**：输入文件可能包含不支持的字符

**解决**：
- 检查文件编码是否为 UTF-8
- 确保没有特殊的不可见字符

### Q4: 如何分析自己的 .tny 文件？

1. 完成 "生成代码" 和 "编译" 步骤
2. 点击 "测试" 按钮
3. 在弹出的文件选择对话框中选择你的 `.tny` 文件
4. 结果将显示在界面中，同时生成对应的 `.lex` 文件

### Q5: 如何在终端中使用词法分析器？

```bash
cd /path/to/output/directory
./_lexer your_program.tny
# 或指定输出文件
./_lexer your_program.tny output.lex
```

---

## 技术规格

- **开发框架**：Qt 6
- **编程语言**：C++ (GUI) / C (生成的词法分析器)
- **编译器要求**：GCC
- **支持平台**：Linux, macOS, Windows
- **支持语言**：TINY, Mini-C

---

*本软件实现了编译原理课程中正则表达式到词法分析器的完整转换流程，支持 TINY 语言和 Mini-C 语言的词法分析。*
