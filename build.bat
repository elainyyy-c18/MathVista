@echo off
echo Building MathVista-C...

if not exist bin mkdir bin
if not exist output mkdir output

gcc -std=c11 -Wall -Wextra -O2 -Iinclude src/core/math_engine.c src/core/memory_pool.c src/core/common.c src/discrete/stirling.c src/discrete/catalan.c src/calculus/gradient.c src/calculus/lagrange.c src/calculus/transform3d.c src/ds/skiplist.c src/ds/radix.c src/algo/sort.c src/algo/graph.c src/viz/viz_dot.c src/viz/viz_ascii.c src/viz/viz_csv.c src/main.c -o bin/mathvista.exe -lm

if errorlevel 1 goto fail

echo.
echo Build successful: bin\mathvista.exe
echo Run with:  bin\mathvista all
goto end

:fail
echo.
echo Build failed. Check errors above.

:end