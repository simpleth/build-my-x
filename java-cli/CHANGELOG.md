# Changelog

## [Unreleased]

### Added
- `java-cli init` 交互式 JDK 检测：自动扫描系统 JDK、IDEA (`~/.jdks`)、自定义目录
- `java-cli sdk scan <dir>` - 注册额外的 JDK 扫描目录
- `java-cli sdk dir` / `config dir` - 自定义 SDK 安装目录

### Fixed
- `withFileSystemTypes` → `withFileTypes` 拼写错误导致运行时崩溃
- Node.js v24 `shell: true` + 数组参数导致 DEP0190 弃用警告

## [1.0.0] - 2026-07-08

### Added
- `java-cli init` - 从模板创建 Java 项目
- `java-cli build` - 编译 Java 源代码
- `java-cli run` - 运行 Java 项目
- `java-cli clean` - 清理构建输出
- `java-cli sdk install/list/use/remove` - JDK 版本管理
- 跨平台支持 (Windows / macOS / Linux)
- 从 Adoptium API 自动下载 JDK
- 项目模板占位符替换 (`{{PROJECT_NAME}}`)
- 项目配置文件 `java-cli.json`
