@echo off
echo Building JSON to XML Translator...

REM Check if bison is available
where bison >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: bison not found. Please install Bison (e.g., via MSYS2, Cygwin, or WSL2)
    echo See README.md for installation instructions
    exit /b 1
)

REM Check if flex is available
where flex >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: flex not found. Please install Flex (e.g., via MSYS2, Cygwin, or WSL2)
    echo See README.md for installation instructions
    exit /b 1
)

REM Check if g++ is available
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: g++ not found. Please install GCC/G++ (e.g., via MinGW, MSYS2, or WSL2)
    echo See README.md for installation instructions
    exit /b 1
)

echo Running Bison...
bison -d parser.y
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Bison failed
    exit /b 1
)

echo Running Flex...
flex scanner.l
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Flex failed
    exit /b 1
)

echo Compiling...
g++ -std=c++11 -o json2xml.exe AST.cpp parser.tab.c lex.yy.c -lfl
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Compilation failed
    exit /b 1
)

echo Build successful! Executable: json2xml.exe
echo.
echo Usage: json2xml.exe ^< input.json
