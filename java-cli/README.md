# java-cli

## 项目简介

`java-cli` 是一个跨平台的 Java 项目脚手架工具，可以快速创建、编译和运行 Java 项目，并内置 JDK 版本管理功能。

一条命令创建项目，一条命令编译运行，无需手动配置环境。

## 项目结构

```
java-cli/
├── bin/
│   └── java-cli.js          # CLI 入口
├── lib/
│   ├── sdk.js               # SDK 管理（下载、安装、切换 JDK）
│   ├── project.js           # 项目管理（模板渲染、配置管理）
│   ├── build.js             # 编译系统（javac/java 封装）
│   └── utils.js             # 工具函数（跨平台兼容）
├── templates/
│   └── default/             # 默认项目模板
│       ├── src/Main.java
│       ├── .gitignore
│       └── README.md
├── tests/                   # 测试脚本
├── install_cli.bat          # Windows 环境安装
├── install_cli.sh           # Linux/macOS 环境安装
├── setup_build.bat          # Windows 编译安装
├── setup_build.sh           # Linux/macOS 编译安装
├── run_test.bat             # Windows 测试
├── run_test.sh              # Linux/macOS 测试
├── uninstall.bat            # Windows 卸载
├── uninstall.sh             # Linux/macOS 卸载
└── package.json             # NPM 包配置
```

## 功能特性

- ✅ 一键创建 Java 项目 (`java-cli init`)
- ✅ 自动编译 Java 源码 (`java-cli build`)
- ✅ 一键运行项目 (`java-cli run`)
- ✅ 清理构建产物 (`java-cli clean`)
- ✅ JDK 版本管理 (`java-cli sdk install/list/use/remove`)
- ✅ 跨平台支持 (Windows / macOS / Linux)
- ✅ 可定制项目模板

## 快速开始

### 安装

**Windows:**
```cmd
install_cli.bat
```

**Linux/macOS:**
```bash
chmod +x install_cli.sh && ./install_cli.sh
```

### 基本用法

```bash
# 创建新项目（首次会自动提示配置 JDK）
java-cli init my-app

# 进入项目目录
cd my-app

# 编译项目
java-cli build

# 运行项目
java-cli run
```

> 如果系统没有 JDK，`java-cli init` 会**交互式引导你下载安装**，无需手动配置。

## 编译前提

### 必需软件

- **Node.js** >= 14.0
- **npm** >= 6.0
- **JDK** — 不是必须预先安装，`init` 时自动引导

### JDK 环境

`java-cli` 的编译运行**依赖 JDK**，你有三种方式获取：

**方式一：系统自带 JDK**
如果 `javac` 已在 PATH 中，`java-cli init` 会自动识别并关联到项目。

**方式二：自动识别已有 JDK**
```bash
java-cli init my-app
# → 自动扫描系统 JDK、~/.jdks (IDEA) 等位置
# → 有多个时让你选择，只有一个时自动使用
```

**方式三：手动安装管理**
```bash
java-cli sdk install 21     # 下载 JDK 21
java-cli sdk list           # 查看已安装版本
java-cli sdk use 21         # 设为默认版本
java-cli sdk remove 17      # 删除不用的版本
```

SDK 安装位置：`~/.java-cli/sdks/`（默认），可用以下命令自定义：

```bash
java-cli sdk dir D:\my-jdks   # 自定义 SDK 目录
java-cli sdk dir               # 查看当前目录
```

### java-cli sdk scan <dir>

注册额外的 JDK 扫描目录。`java-cli init` 会自动扫描这些目录。

```bash
java-cli sdk scan D:\tools\jdks   # 注册扫描目录
java-cli sdk scan                  # 查看已注册的目录
java-cli sdk scan --remove D:\tools\jdks  # 移除
```

## 命令参考

### java-cli init <project-name>

创建一个新的 Java 项目。

```bash
java-cli init my-app                    # 在当前目录创建
java-cli init my-app -d ~/projects      # 在指定目录创建
java-cli init my-app -t default         # 使用指定模板
```

### java-cli build

编译当前项目。

```bash
java-cli build                          # 输出到 build/ 目录
java-cli build -o dist                  # 输出到指定目录
```

### java-cli run [main-class]

运行当前项目。

```bash
java-cli run                            # 运行默认主类 (Main)
java-cli run com.example.Main           # 运行指定主类
```

### java-cli clean

清理构建输出目录。

```bash
java-cli clean
```

### java-cli sdk <command>

管理 Java SDK。

```bash
java-cli sdk list                       # 列出已安装的 SDK
java-cli sdk install 17                 # 安装 JDK 17
java-cli sdk install 21                 # 安装 JDK 21
java-cli sdk use 21                     # 切换到 JDK 21
java-cli sdk remove 17                  # 删除 JDK 17
java-cli sdk dir                        # 查看 SDK 安装目录
java-cli sdk dir D:\my-jdks            # 自定义 SDK 安装目录
java-cli sdk scan D:\tools\jdks        # 注册额外 JDK 扫描目录
java-cli sdk scan                       # 查看已注册扫描目录
```

> 注：`java-cli config dir` 与 `java-cli sdk dir` 等价。

## 项目配置

每个 Java 项目根目录下有一个 `java-cli.json` 配置文件：

```json
{
  "name": "my-app",
  "version": "1.0.0",
  "mainClass": "Main",
  "sourceDir": "src",
  "outputDir": "build",
  "sdkVersion": "21",
  "classpath": []
}
```

| 字段 | 说明 |
|------|------|
| `name` | 项目名称 |
| `version` | 项目版本号 |
| `mainClass` | 入口类名（不含 .java 扩展名） |
| `sourceDir` | 源代码目录 |
| `outputDir` | 编译输出目录 |
| `sdkVersion` | 指定使用的 JDK 版本（可选，留空则使用系统 JDK） |
| `classpath` | 额外的 classpath 依赖（支持相对路径） |

## 架构设计

`java-cli` 分为三个核心模块：

### 1. SDK 管理 (lib/sdk.js)

负责 JDK 的生命周期管理：
- **扫描**：自动检测系统 PATH、`~/.jdks`（IDEA）、自定义目录中的 JDK
- **下载**：从 Adoptium API 获取指定版本的 JDK 发行包
- **安装**：自动解压（.tar.gz / .zip）到 `~/.java-cli/sdks/`
- **切换**：修改全局配置，切换活跃 JDK 版本
- **清理**：删除不再需要的 JDK 版本

### 2. 项目管理 (lib/project.js)

负责项目的创建和配置：
- **模板渲染**：将模板文件复制到目标目录，替换 `{{PROJECT_NAME}}` 等占位符
- **配置管理**：读写 `java-cli.json` 项目配置文件

### 3. 编译系统 (lib/build.js)

封装 `javac` 和 `java` 命令：
- **编译**：扫描 `sourceDir` 下所有 .java 文件，调用 javac 编译
- **运行**：调用 java 启动编译后的项目
- **清理**：删除 `outputDir` 目录

## 跨平台设计

| 平台 | 处理方式 |
|------|---------|
| Windows | 路径使用反斜杠，可执行文件加 `.exe` 后缀，使用 `shell: true` spawn |
| macOS | 处理 JDK `Contents/Home` 目录结构，使用 `tar` 解压 |
| Linux | 标准路径处理，使用 `tar` 解压 |

## 测试

```cmd
:: Windows
run_test.bat           # 运行全部测试
:: Linux/macOS
./run_test.sh          # 运行全部测试
```

更多详情见 `run_test.bat` / `run_test.sh`。

## 环境恢复/卸载

**Windows:**
```cmd
uninstall.bat
```

**Linux/macOS:**
```bash
./uninstall.sh
```

卸载操作会：
- 移除全局 `java-cli` 命令链接
- 清理 `node_modules` 目录
- 保留用户数据（`~/.java-cli/` 下的 SDK 和配置不会被删除）

## 许可证

MIT License
