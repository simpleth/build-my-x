# Tiny Compiler

## 项目简介

这是一个最小可运行的编译器框架，用于学习现代编译器设计的核心概念。
它能将简化的编程语言编译为 LLVM IR，并进一步编译为可执行文件。

## 项目结构

```
compiler/
├── include/          # 头文件
│   ├── ast.h        # AST (抽象语法树) 定义
│   ├── lexer.h      # 词法分析器
│   ├── parser.h     # 语法分析器
│   └── codegen.h    # LLVM IR 代码生成器
├── src/             # 实现文件
│   ├── lexer.cpp
│   ├── parser.cpp
│   ├── codegen.cpp
│   └── main.cpp
├── tests/           # 测试样例
│   ├── test1.tiny   # 基础打印
│   ├── test2.tiny   # 算术运算
│   ├── test3.tiny   # 条件分支
│   ├── test4.tiny   # 循环
│   └── test5.tiny   # 函数定义和调用
├── runtime.c        # 运行时库 (提供 print 函数)
└── CMakeLists.txt   # 构建配置
```

## 语言特性

Tiny 语言支持以下特性：

- ✅ 变量声明和赋值 (`let x = 42;`)
- ✅ 算术运算 (`+`, `-`, `*`, `/`)
- ✅ 比较运算 (`<`, `>`, `==`)
- ✅ 条件分支 (`if ... else`)
- ✅ 循环 (`while`)
- ✅ 函数定义和调用
- ✅ 返回值 (`return`)
- ✅ 注释 (`//`)

### 语法示例

```
fn main() {
    let x = 10;
    let y = 20;
    if (x < y) {
        print(x + y);
    }
}

fn add(a, b) {
    let result = a + b;
    return result;
}
```

## 编译前提

### 必需软件

- **CMake** >= 3.14
- **C++ 编译器** (支持 C++17)
  - Windows: Visual Studio 2019+ 或 MinGW-w64
  - Linux: GCC 7+ 或 Clang 5+
  - macOS: Clang (Xcode Command Line Tools)
- **LLVM** >= 10.0

### 安装 LLVM

**Windows (MSYS2):**

本项目已配置好 MSYS2 支持。请按以下步骤安装 LLVM：

```bash
# 1. 安装 LLVM 和相关工具
install_llvm.bat

# 2. 编译项目
setup_build.bat
```

**环境恢复/卸载:**
```bash
# 运行卸载脚本，可选择不同级别
uninstall.bat

# 或手动卸载 LLVM/CMake（保留 GCC）
# 在 MSYS2 UCRT64 终端中运行:
pacman -Rns mingw-w64-ucrt-x86_64-llvm mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-cmake

# MSYS2 环境是完全隔离的，删除 C:\msys64 即可完全清除
```

**Windows (Chocolatey):**
```bash
choco install llvm
```

**Ubuntu/Debian:**
```bash
sudo apt-get install llvm-dev
```

**macOS:**
```bash
brew install llvm
```

## 构建步骤

### 1. 安装依赖并编译

**使用提供的批处理脚本（推荐）：**

```cmd
:: 1. 安装 LLVM（首次需要）
install_llvm.bat

:: 2. 编译项目
setup_build.bat
```

**手动编译：**

```bash
:: 设置环境变量
set PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%

:: 配置和编译
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### 2. 测试编译器

编译测试样例：

```cmd
:: 进入项目根目录
cd D:\build-my-x\compiler

:: 编译 test1.tiny
build\TinyCompiler.exe tests\test1.tiny -o output
```

### 3. 链接和运行

**使用 GCC 链接：**
```bash
gcc -c runtime.c
gcc output.o runtime.o -o test1.exe
test1.exe
```

**完整的一键编译运行脚本：**
```cmd
build\TinyCompiler.exe tests\test1.tiny -o test1 && ^
gcc -c runtime.c && ^
gcc test1.o runtime.o -o test1.exe && ^
test1.exe
```

## 编译器架构

编译过程分为三个主要阶段：

### 1. 前端 (Frontend)

**Lexer (词法分析器)**
- 输入：源代码字符串
- 输出：Token 序列
- 功能：识别关键字、标识符、数字、运算符等

**Parser (语法分析器)**
- 输入：Token 序列
- 输出：AST (抽象语法树)
- 算法：递归下降解析 + 运算符优先级

### 2. 中端 (Middle-end)

**CodeGen (代码生成器)**
- 输入：AST
- 输出：LLVM IR
- 功能：遍历 AST，生成 LLVM 中间表示

### 3. 后端 (Backend)

**LLVM**
- 输入：LLVM IR
- 输出：目标平台机器码
- 功能：优化、指令选择、寄存器分配

## 学习路径

建议按以下顺序学习：

1. **阅读 AST 定义** (`include/ast.h`)
   - 理解抽象语法树的结构
   - 了解语言的语法元素

2. **学习词法分析** (`src/lexer.cpp`)
   - 理解 Token 化过程
   - 学习状态机和字符处理

3. **掌握语法分析** (`src/parser.cpp`)
   - 理解递归下降解析
   - 学习运算符优先级处理

4. **探索代码生成** (`src/codegen.cpp`)
   - 理解 LLVM IR 的基本概念
   - 学习基本块、控制流图的构建

5. **实验和扩展**
   - 添加新的语言特性
   - 优化生成的 IR
   - 实现错误恢复机制

## 常见问题

### Q: 为什么选择 LLVM 作为后端？

A: LLVM 提供了成熟的优化和代码生成基础设施，让你可以专注于学习编译器前端的核心概念，而不必处理底层架构细节。

### Q: 如何调试生成的 LLVM IR？

A: 使用 `--ir` 参数可以查看生成的 LLVM IR 代码：
```bash
TinyCompiler.exe tests\test1.tiny --ir
```

### Q: 如何添加新的语言特性？

A: 
1. 在 `ast.h` 中添加新的 AST 节点
2. 在 `lexer.h` 中添加新的 Token 类型
3. 在 `parser.cpp` 中实现解析逻辑
4. 在 `codegen.cpp` 中实现代码生成

## 下一步

- [ ] 添加类型系统
- [ ] 支持字符串类型
- [ ] 实现数组和结构体
- [ ] 添加标准错误处理
- [ ] 实现更多优化 pass

## 参考资源

- [LLVM Tutorial](https://llvm.org/docs/tutorial/)
- [Crafting Interpreters](https://craftinginterpreters.com/)
- [Engineering a Compiler](https://www.amazon.com/Engineering-Compiler-Keith-Cooper/dp/012088478X)

## 许可证

MIT License
