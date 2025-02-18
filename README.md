# Watcher 进程监控工具

## 项目简介

Watcher 是一个用C++开发的轻量级进程监控守护工具，用于监控指定进程的运行状态，当目标进程不存在时会自动重启。

## 主要特性

- 支持守护进程模式运行
- 自动监控目标进程状态
- 进程异常退出自动重启
- 支持日志记录功能
- 支持命令行参数配置

## 编译安装

### 环境要求

- C++ 编译器（支持C++11标准）
- Make 工具

### 编译步骤

1. 克隆代码到本地
2. 进入项目目录
3. 执行编译命令：

```bash
# 默认编译（优化模式）
make

# 调试模式编译
make DEBUG=1

# 清理编译文件
make clean
```

编译完成后，可执行文件将生成在 `bin` 目录下。

## 使用说明

### 命令行参数

```bash
Usage: watcher [p:n:vh] [par]

参数说明：
  --path, -p     指定要监控的程序路径
  --name, -n     指定要监控的程序名称
  --version, -v  显示版本信息
  --help, -h     显示帮助信息
```

### 使用示例

```bash
# 监控指定程序
./bin/watcher --path "/path/to/your/program" --name "program_name"

# 显示版本信息
./bin/watcher --version

# 显示帮助信息
./bin/watcher --help
```

### 日志

- 日志文件位置：项目目录下的 `log/watcher.log`
- 日志包含时间戳、进程ID、日志级别等信息
- 日志文件大小超过10MB时会自动清空

## 版本信息

当前版本：1.0.0


---
that's all