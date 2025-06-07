@echo off
@chcp 65001 >nul 2>&1

for /f "delims=#" %%i in ('prompt #$E#^&echo on^&for %%a in ^(1^) do rem') do set esck=%%i
set "e=%esck%[38;5;196m"
set "g=%esck%[38;5;46m"
set "R=%esck%[m"

tcc --version >nul 2>&1
if errorlevel 1 (
    echo [%e%Error%R%]: TCC ^(Tiny C Compiler^) is not installed or not found in PATH.
    exit /b 1
)

set "CFLAGS=-O2"

if not exist main.c (
    cd ..
)

if not exist main.c (
    echo [%e%Error%R%]: main.c not found in the current directory.
    exit /b 1
)

md build 2> nul | del /Q build\*

for %%i in (src/*.c) do (
    echo [%g%Info%R%]: Compiling %%~nxi...
    tcc -c -o build\%%~ni.o src/%%i -Iinclude %CFLAGS%
    if errorlevel 1 (
        echo [%e%Error%R%]: Compilation failed for %%~nxi.
        exit /b 1
    )
)

echo [%g%Info%R%]: Compiling main.c...
tcc -o build/clite.exe build\*.o main.c -Iinclude %CFLAGS%

if errorlevel 1 (
    echo [%e%Error%R%]: Compilation failed for main.c.
    exit /b 1
)

echo [%g%Info%R%]: linking object files...
echo [%g%Info%R%]: build completed successfully!