@echo off
REM Build script for Windows
REM This script compiles the parser project

echo ========================================
echo  Parser Project Build Script (Windows)
echo ========================================
echo.

REM Check if g++ is available
where g++ >nul 2>nul
if errorlevel 1 (
    echo Error: g++ not found! Please install MinGW.
    pause
    exit /b 1
)

echo Compiling source files...
echo.

REM Create output directories
if not exist "bin" mkdir bin
if not exist "build" mkdir build

REM Compile all source files
set SRCS=src/Token.cpp src/Grammar.cpp src/Items.cpp src/Stack.cpp src/ParsingTable.cpp src/SLRParser.cpp src/LR1Parser.cpp src/Tree.cpp src/GraphvizConverter.cpp src/main.cpp

g++ -std=c++11 -Wall -O2 -o bin/parser.exe %SRCS%

if errorlevel 1 (
    echo.
    echo Compilation FAILED!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Compilation SUCCESSFUL!
echo ========================================
echo Executable: bin/parser.exe
echo.
echo Run: bin\parser.exe
echo.
pause
