@echo off
setlocal enabledelayedexpansion

echo === Step 1: Check file inventory ===
echo.
set MISSING=0
for %%f in
(
    src\core\math_engine.c
    src\core\memory_pool.c
    src\core\common.c
    src\discrete\stirling.c
    src\discrete\catalan.c
    src\calculus\gradient.c
    src\calculus\lagrange.c
    src\calculus\transform3d.c
    src\ds\skiplist.c
    src\ds\radix.c
    src\algo\sort.c
    src\algo\graph.c
    src\viz\viz_dot.c
    src\viz\viz_ascii.c
    src\viz\viz_csv.c
    src\main.c
) do
(
    if exist %%f
    (
        for %%A in (%%f) do echo   OK      %%~zA bytes   %%f
    )
    else
    (
        echo   MISSING                 %%f
        set MISSING=1
    )
)

if !MISSING! == 1
(
    echo.
    echo *** Some source files are missing. Stopping. ***
    exit /b 1
)

echo.
echo === Step 2: Compile each file individually ===
echo.
if not exist build mkdir build

set FAILED=0
for %%f in
(
    src\core\math_engine.c
    src\core\memory_pool.c
    src\core\common.c
    src\discrete\stirling.c
    src\discrete\catalan.c
    src\calculus\gradient.c
    src\calculus\lagrange.c
    src\calculus\transform3d.c
    src\ds\skiplist.c
    src\ds\radix.c
    src\algo\sort.c
    src\algo\graph.c
    src\viz\viz_dot.c
    src\viz\viz_ascii.c
    src\viz\viz_csv.c
    src\main.c
) do
(
    gcc -std=c11 -Iinclude -c %%f -o build\%%~nf.o 2>build\%%~nf.err
    if errorlevel 1
    (
        echo   FAIL    %%f
        type build\%%~nf.err
        echo.
        set FAILED=1
    )
    else
    (
        echo   OK      %%f
        del build\%%~nf.err 2>nul
    )
)

if !FAILED! == 1
(
    echo.
    echo *** Some files failed to compile. See errors above. ***
    exit /b 1
)

echo.
echo === Step 3: Link all object files ===
echo.
if not exist bin mkdir bin
gcc build\math_engine.o build\memory_pool.o build\common.o build\stirling.o build\catalan.o build\gradient.o build\lagrange.o build\transform3d.o build\skiplist.o build\radix.o build\sort.o build\graph.o build\viz_dot.o build\viz_ascii.o build\viz_csv.o build\main.o -o bin\mathvista.exe -lm

if errorlevel 1
(
    echo.
    echo *** Link failed. ***
    exit /b 1
)

echo.
echo === BUILD SUCCESSFUL ===
echo Run with:  bin\mathvista all