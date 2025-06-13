#!/bin/bash

# 设置颜色
e="\033[38;5;196m"
g="\033[38;5;46m"
y="\033[38;5;226m"
R="\033[m"

# 检查平台，确保是 Linux
if [[ "$(uname -s)" != "Linux" ]]; then
    echo "[Error]: This script is only supported on Linux."
    exit 1
fi

# 检查 GCC 是否安装
command -v gcc &> /dev/null
if [ $? -ne 0 ]; then
    echo "[${e}Error${R}]: GCC (GNU Compiler Collection) is not installed or not found in PATH."
    exit 1
fi
CFLAGS="-Wall -Wextra -Wno-int-conversion -Wno-switch -std=c11 -O2"

# 检查 main.c 是否存在
if [ ! -f main.c ]; then
    echo "[${y}Info${R}]: main.c not found, checking parent directory..."
    cd .. || exit
fi

if [ ! -f main.c ]; then
    echo "[${e}Error${R}]: main.c not found in the current directory."
    exit 1
fi

mkdir -p build && rm -f build/*

# 编译 src 目录下的所有 .c 文件
for file in src/*.c; do
    echo "[${g}Info${R}]: Compiling $(basename "$file")..."
    gcc -c -o "build/$(basename "${file%.*}").o" "$file" -Iinclude $CFLAGS
    if [ $? -ne 0 ]; then
        echo "[${e}Error${R}]: Compilation failed for $(basename "$file")."
        exit 1
    fi
done

# 编译 main.c
echo "[${g}Info${R}]: Compiling main.c..."
gcc -o build/clite build/*.o main.c -Iinclude $CFLAGS
if [ $? -ne 0 ]; then
    echo "[${e}Error${R}]: Compilation failed for main.c."
    exit 1
fi

# 链接完成
echo "[${g}Info${R}]: linking object files..."
echo "[${g}Info${R}]: build completed successfully!"