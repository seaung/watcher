# 编译器设置
CC = g++
TOPDIR = $(shell pwd)

# 目录设置
OBJDIR = $(TOPDIR)/obj/
SRCDIR = $(TOPDIR)/src/
BINDIR = $(TOPDIR)/bin/

# 版本信息
VERSION = 1.0.0
BUILD_TIME = $(shell date "+%Y-%m-%d %H:%M:%S")

# 编译选项
CFLAGS_COMMON = -std=c++0x -Wall -Wextra -pipe
CFLAGS_OPT = -O2 -DNDEBUG
CFLAGS_DEBUG = -g -O0 -DDEBUG

# 根据编译模式设置不同的编译选项
ifeq ($(DEBUG), 1)
    CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
    BUILD_TYPE = Debug
else
    CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_OPT)
    BUILD_TYPE = Release
endif

# 定义编译信息
CFLAGS += -DVERSION=\"$(VERSION)\"
CFLAGS += -DBUILD_TIME=\"$(BUILD_TIME)\"
CFLAGS += -DBUILD_TYPE=\"$(BUILD_TYPE)\"

# 源文件和目标文件
SRCLIST = $(wildcard $(SRCDIR)*.cpp)
OBJLIST = $(basename $(SRCLIST))
OBJTEMP1 = $(addsuffix .o ,$(OBJLIST))
OBJTEMP2 = $(notdir $(OBJTEMP1))
OBJ = $(addprefix $(OBJDIR),$(OBJTEMP2))
BIN = $(BINDIR)watcher

# 主要目标
.PHONY: all clean help

all: banner CHECKDIR $(BIN)

# 显示编译信息
banner:
	@echo "=== Building Watcher $(VERSION) ($(BUILD_TYPE)) ==="
	@echo "Compiler: $(CC)"
	@echo "Build Time: $(BUILD_TIME)"

# 创建必要的目录
CHECKDIR:
	@mkdir -p $(OBJDIR) $(BINDIR)
	@mkdir -p $(TOPDIR)/log

# 编译目标文件
$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@echo "Compiling $<..."
	@$(CC) -c $(CFLAGS) $< -o $@

# 链接可执行文件
$(BIN): $(OBJ)
	@echo "Linking $@..."
	@$(CC) $^ -o $@ $(CFLAGS)
	@echo "Build complete!"

# 清理编译产物
clean:
	@echo "Cleaning build files..."
	@rm -rf $(OBJDIR) $(BINDIR)

# 帮助信息
help:
	@echo "Available targets:"
	@echo "  all      - Build the watcher program (default)"
	@echo "  clean    - Remove all build files"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  DEBUG=1  - Build with debug information"
	@echo "  DEBUG=0  - Build with optimizations (default)"

