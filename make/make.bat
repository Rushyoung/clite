@echo off
@chcp 65001 >nul 2>&1

for /f "delims=#" %%i in ('prompt #$E#^&echo on^&for %%a in ^(1^) do rem') do set esck=%%i
set "e=%esck%[38;5;196m"
set "i=%esck%[38;5;46m"
set "w=%esck%[38;5;226m"
set "R=%esck%[m"


tcc --version >nul 2>&1
if errorlevel 1 (
    echo [%w%Warning%R%]: TCC ^(Tiny C Compiler^) is not installed or not found in PATH.
) else (
    echo [%i%Info%R%]: TCC ^(Tiny C Compiler^) is installed.
    set "cc=tcc"
    set "CFLAGS=-O2"
    goto:compile
)

gcc --version >nul 2>&1
if errorlevel 1 (
    echo [%e%Error%R%]: gcc ^(GNU Compiler Collection^) is not installed or not found in PATH.
    exit /b 1
) 
echo [%i%Info%R%]: gcc ^(GNU Compiler Collection^) is installed.
set "cc=gcc"
set "CFLAGS=-Wall -Wextra -Wno-int-conversion -Wno-switch -std=c11 -O2"


:compile
if not exist main.c (
    echo [%w%Warning%R%]: main.c not found, checking parent directory...
    cd ..
)

if not exist main.c (
    echo [%e%Error%R%]: main.c not found in the current directory.
    exit /b 1
)

for /F "delims=" %%i in (VERSION.txt) do set "VERSION=%%i"

md build 2> nul | del /Q build\*

for %%i in (src/*.c) do (
    echo [%i%Info%R%]: Compiling %%~nxi...
    %cc% -c -o build/%%~ni.o src/%%i -Iinclude %CFLAGS% -DVERSION=%VERSION%
    if errorlevel 1 (
        echo [%e%Error%R%]: Compilation failed for %%~nxi.
        exit /b 1
    )
)

for %%i in (src/compiler/*.c) do (
    echo [%i%Info%R%]: Compiling %%~nxi...
    %cc% -c -o build/%%~ni.o src/compiler/%%i -Iinclude %CFLAGS% -DVERSION=%VERSION%
    if errorlevel 1 (
        echo [%e%Error%R%]: Compilation failed for %%~nxi.
        exit /b 1
    )
)

echo [%i%Info%R%]: Compiling main.c...
%cc% -o build/clite.exe build\*.o main.c -Iinclude %CFLAGS%

if errorlevel 1 (
    echo [%e%Error%R%]: Compilation failed for main.c.
    exit /b 1
)

echo [%i%Info%R%]: linking object files...
echo [%i%Info%R%]: build completed successfully!

if /I "%1"=="test" (
    goto:test
)

exit /b 0


:test
echo [%i%Info%R%]: Running tests...
for %%i in (example\*.c) do (
    echo [%i%Info%R%]: Testing %%~nxi...
    build\clite.exe %%i
    if errorlevel 1 (
        echo [%e%Error%R%]: Test failed for %%~nxi.
        exit /b 1
    ) else (
        echo [%i%Info%R%]: Test passed for %%~nxi.
    )
)