# Build your own X

> 从零构建，理解原理。每个项目都是一份独立、可运行的最小实现。

## 目录

- [编译器](#编译器)
- [命令行工具](#命令行工具)
- [更多项目规划](#更多项目规划)

---

## 编译器

### 构建你自己的编译器

**[TinyCompiler](compiler)** — 一个不到 1500 行的微型编译器，将 Tiny 语言编译为 LLVM IR 并输出本地机器码。

```
语言: C++17    后端: LLVM    构建: CMake
```

**覆盖的核心概念**：词法分析（Lexer）→ 语法分析（递归下降 Parser）→ 抽象语法树（AST）→ LLVM IR 代码生成 → 目标机器码输出

**特性**：变量、算术、比较、条件分支、循环、函数、注释，5 个集成测试用例

```
cd compiler && setup_build.bat && run_test.bat 1
```

---

## 命令行工具

### 构建你自己的 Java 脚手架

**[java-cli](java-cli)** — 一个跨平台的 Java 项目 CLI，一条命令创建、编译、运行项目，内置 JDK 版本管理器。

```
语言: JavaScript    运行时: Node.js    包管理: npm
```

**覆盖的核心概念**：CLI 设计（Commander.js）→ 交互式引导 → 项目模板渲染 → 编译系统封装（javac/java）→ JDK 生命周期管理（下载/安装/切换/卸载）→ 跨平台适配

**特性**：`init` / `build` / `run` / `clean` 四大命令，自动扫描 IDEA、系统 PATH 中的 JDK，从 Adoptium API 一键下载

```
cd java-cli && setup_build.bat && java-cli init my-app
```

---

## 更多项目规划

计划陆续添加：

- [ ] **Database** — 从零构建一个简单的关系型数据库（B-Tree、SQL 解析、事务）
- [ ] **Web Server** — 从零构建一个 HTTP 服务器（TCP socket、路由、中间件）
- [ ] **Git** — 从零构建一个版本控制系统（blob/tree/commit 对象、diff、merge）
- [ ] **Docker** — 从零构建一个容器运行时（namespace、cgroup、镜像分层）
- [ ] **Regex Engine** — 从零构建一个正则引擎（NFA/DFA、回溯）
- [ ] **Interpreter** — 从零构建一个脚本语言解释器（Tree-walk、字节码 VM）

如果你有想构建的工具，欢迎提交 Issue 或 PR。

## 为什么从零构建

阅读源码是被动学习，从零构建是主动理解。当你亲手实现一个编译器、一个数据库、一个 Git 客户端时，你不再是一个"使用者"，而是真正理解了这些工具每一层的设计决策和权衡。

## 许可证

MIT
