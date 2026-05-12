# MathVista-C

**Academic Mathematics Visualisation & Computation Engine written in C11**

MathVista-C is a C11 project that turns first-year computer science coursework into executable modules. It connects **Calculus**, **Discrete Mathematics**, and **Data Structures & Algorithms** through numerical methods, recurrence relations, dynamic programming, pointer-based data structures, graph traversal, sorting, memory management, and visual output.

The goal is not to build a polished commercial product. The goal is to show how classroom concepts can become testable C programs with clear modules, error handling, and outputs that make algorithms easier to inspect.

---

## Project Purpose / Motivation

This project was built as a learning portfolio project after studying three core first-year CS courses:

| Course | How it appears in MathVista-C |
|---|---|
| **Calculus** | numerical gradient, Hessian matrix, directional derivative, Lagrange multiplier approximation, function sampling |
| **Discrete Mathematics** | recurrence relations, memoization, dynamic programming, Stirling numbers, Catalan numbers, Dyck paths |
| **Data Structures & Algorithms** | radix tree, skip list, graph DFS/BFS, iterative QuickSort, callback-based visualisation, memory pool |

While solving textbook problems helped me understand definitions and formulas, implementing them in C forced me to think about additional engineering questions:

- How should a mathematical object such as a vector, matrix, graph, or recurrence table be represented in memory?
- How can a recursive formula be converted into memoization or bottom-up DP?
- How should numerical approximation handle step size, dimension mismatch, and convergence failure?
- How can pointer-heavy structures such as radix trees and skip lists be tested and visualized?
- How can a library return useful errors instead of crashing silently?

This is why MathVista-C is organized as a small computation engine rather than a group of unrelated homework files.

---

## Quick Start

### Windows (native cmd / PowerShell + MinGW-w64)

```bat
.\build.bat
.\bin\mathvista.exe menu
```

You can also run every demo directly:

```bat
.\bin\mathvista.exe all
```

> Tip: if you see `?` characters in the output, your console is probably not using UTF-8. Run `.\run_menu.bat` once in the same terminal session.

### Linux / macOS / MSYS2 / WSL

```bash
make
./bin/mathvista menu
```

You can also run every demo directly:

```bash
./bin/mathvista all
```

> Tip: On Windows, `.\build.bat` is used to compile the project. `.\run_menu.bat` is used to run the interactive menu with UTF-8 output, which helps avoid `?` characters in the terminal.

### Single-demo run

```bash
./bin/mathvista stirling
./bin/mathvista catalan
./bin/mathvista skiplist
./bin/mathvista radix
./bin/mathvista sort
./bin/mathvista graph
./bin/mathvista lagrange
./bin/mathvista ascii
./bin/mathvista csv
./bin/mathvista mempool
./bin/mathvista transform
```

---

## Interactive Demo Menu

`menu` opens a small CLI menu that reuses the existing demo functions:

```text
MathVista-C Interactive Demo Menu

1. Discrete Math Demos        Stirling numbers + Catalan / Dyck paths
2. Calculus / Visualization   Lagrange + ASCII plots + CSV / gnuplot export
3. QuickSort Visualization    Iterative Lomuto step-through
4. DFS / BFS Graph Traversal  Step callbacks + DOT export
5. Radix Tree Visualization   Compressed trie + DOT export
6. Skip List Visualization    Insert/search/delete + DOT export
7. Memory Pool Demo           Allocation health map + DOT export
8. 3-D Transform Demo         Matrix transform + perspective projection
9. Run All Demos
0. Exit
```

This menu is intentionally simple. It is designed for portfolio demonstration: a reviewer can compile the project, choose a topic, and immediately see the relevant algorithm or mathematical module run. On Windows, `run_menu.bat` can be used to open this menu with UTF-8 output enabled.

---

## What gets generated

Demo output files are written to `output/`:

| File | Contents |
|---|---|
| `output/skiplist.dot` | skip-list pointer diagram |
| `output/radix.dot` | compressed-trie / radix-tree diagram |
| `output/graph_dfs.dot` | DFS traversal graph |
| `output/graph_bfs.dot` | BFS traversal graph |
| `output/mempool.dot` | memory-pool block map |
| `output/sinc.csv` + `output/sinc.gp` | `sinc(x)` sampled data and gnuplot script |
| `output/gauss2d.csv` + `output/gauss2d.gp` | 2-D Gaussian surface data and gnuplot script |
| `output/matrix.csv` | 4x4 matrix export |

Render DOT files with Graphviz:

```bash
dot -Tpng output/skiplist.dot -o docs/images/skiplist.png
dot -Tpng output/radix.dot -o docs/images/radix_tree.png
dot -Tpng output/graph_dfs.dot -o docs/images/graph_dfs.png
dot -Tpng output/graph_bfs.dot -o docs/images/graph_bfs.png
dot -Tpng output/mempool.dot -o docs/images/mempool.png
```

Render gnuplot outputs:

```bash
gnuplot output/sinc.gp
gnuplot output/gauss2d.gp
```

---

## Visual Output / Screenshots

The project can produce several kinds of visual output:

- terminal ASCII plots for one-variable functions;
- terminal histogram output;
- Graphviz DOT diagrams for pointer-based structures and graph traversal;
- CSV data and gnuplot scripts for sampled functions and surfaces;
- memory-pool state visualisation for checking allocation behavior.

Suggested screenshot files for the GitHub README:

| Suggested image path | How to generate it |
|---|---|
| `docs/images/radix_tree.png` | `dot -Tpng output/radix.dot -o docs/images/radix_tree.png` |
| `docs/images/skiplist.png` | `dot -Tpng output/skiplist.dot -o docs/images/skiplist.png` |
| `docs/images/graph_bfs.png` | `dot -Tpng output/graph_bfs.dot -o docs/images/graph_bfs.png` |
| `docs/images/graph_dfs.png` | `dot -Tpng output/graph_dfs.dot -o docs/images/graph_dfs.png` |
| `docs/images/mempool.png` | `dot -Tpng output/mempool.dot -o docs/images/mempool.png` |
| `docs/images/sinc.png` | `gnuplot output/sinc.gp` then copy/move generated image if needed |
| `docs/images/gauss2d.png` | `gnuplot output/gauss2d.gp` then copy/move generated image if needed |

After generating the images, they can be embedded here:

```md
![Radix tree visualization](docs/images/radix_tree.png)
![Skip list visualization](docs/images/skiplist.png)
![BFS graph traversal](docs/images/graph_bfs.png)
![Memory pool visualization](docs/images/mempool.png)
```

The image links are listed as placeholders because the PNG files should be generated from the current program output, not invented manually.

---

## Project Structure

```text
MathVista-C/
├── include/                  # public headers
│   ├── math_engine.h         # MVec, MMat, MemoTable
│   ├── memory_pool.h         # fixed-block memory pool
│   ├── common.h              # timer, logger, banner
│   ├── discrete.h            # Stirling, Catalan, Dyck paths
│   ├── calculus.h            # gradient, Hessian, Lagrange
│   ├── transform3d.h         # 4x4 homogeneous transforms
│   ├── ds_skiplist.h         # probabilistic skip list
│   ├── ds_radix.h            # compressed radix tree
│   ├── algo_sort.h           # QuickSort with step callbacks
│   ├── algo_graph.h          # DFS + BFS with step callbacks
│   ├── viz_dot.h             # Graphviz DOT exporter
│   ├── viz_ascii.h           # terminal function plotter
│   └── viz_csv.h             # CSV + gnuplot script exporter
│
├── src/
│   ├── core/                 # math_engine.c, memory_pool.c, common.c
│   ├── discrete/             # stirling.c, catalan.c
│   ├── calculus/             # gradient.c, lagrange.c, transform3d.c
│   ├── ds/                   # skiplist.c, radix.c
│   ├── algo/                 # sort.c, graph.c
│   ├── viz/                  # viz_dot.c, viz_ascii.c, viz_csv.c
│   └── main.c                # CLI dispatcher + interactive menu
│
├── output/                   # generated DOT / CSV / gnuplot files
├── docs/images/              # suggested location for generated screenshots
├── build.bat                 # Windows native build
├── diagnose.bat              # per-file compile diagnostic for Windows
├── runmenu.bat               # chcp65001 avoiding ? characters in the terminal.
├── Makefile                  # Unix build
└── README.md
```

---

## Build System Details

Both `build.bat` and the `Makefile` compile the same C modules:

```bash
gcc -std=c11 -Wall -Wextra -O2 -Iinclude <all .c files> -o bin/mathvista -lm
```

The single `-Iinclude` flag makes every `#include "math_engine.h"` resolve correctly regardless of which subdirectory the `.c` file is in.

### Makefile targets

```bash
make            # optimized build
make lib        # produces build/libmathvista.a
make debug      # adds -g -fsanitize=address
make test       # build + run every demo
make clean      # remove build artifacts
```

### Compiler requirements

- GCC or Clang with C11 support;
- standard C library + `libm`;
- MinGW-w64 is recommended for Windows native builds;
- no large external dependency is required for the core program;
- Graphviz and gnuplot are optional tools for rendering generated visual outputs.

---

## Module Reference

### Core: `math_engine`, `memory_pool`, `common`

- `MVec` / `MMat`: dynamic-dimension vector and row-major matrix operations.
- `MemoTable`: open-addressing hash table for recurrence memoization.
- `MemPool`: fixed-size block allocator using an intrusive free list.
- `common`: logging, timer, banners, and common status handling.

### Discrete Mathematics

- Stirling numbers of the second kind using memoized recursion and bottom-up DP.
- Catalan numbers using convolution recurrence.
- Dyck path rendering for connecting Catalan numbers with valid path enumeration.

### Calculus and Numerical Methods

- `numeric_gradient`: central difference approximation.
- `numeric_hessian`: second-order numerical derivative approximation.
- `numeric_directional_deriv`: directional derivative through normalized direction vectors.
- `lagrange_solve`: approximate constrained optimization through tangent update and projection.
- `transform3d`: 4x4 homogeneous transformations and perspective projection.

### Data Structures

- Skip list with multi-level forward pointers.
- Radix tree / compressed trie with prefix splitting.
- Memory pool to practice allocation behavior and debugging.

### Algorithms

- Iterative Lomuto QuickSort with step callbacks.
- Directed graph representation with DFS and BFS step callbacks.
- Explicit stacks and queues used to avoid hiding the traversal logic.

### Visualisation

- ASCII function plotter and histogram output.
- Graphviz DOT export for data structures and graph traversal.
- CSV and gnuplot export for sampled functions and 2-D surfaces.

---

## Learning Reflection

This project helped me turn course knowledge into implementation details.

In **Calculus**, formulas such as gradients, Hessians, and directional derivatives became numerical algorithms. I had to think about how many function evaluations were required, how to choose a finite-difference step size, and how to represent vectors and matrices safely in C. The Lagrange multiplier demo also helped me connect constrained optimization with iterative numerical procedures.

In **Discrete Mathematics**, recurrence relations became executable code. Stirling numbers and Catalan numbers were no longer only definitions on paper; they became a chance to compare recursive memoization and bottom-up dynamic programming. This made the idea of “state” much more concrete.

In **Data Structures and Algorithms**, the main difficulty was not just knowing what a structure means, but implementing it correctly. A radix tree requires careful edge splitting when two strings share only part of a prefix. A skip list requires multiple forward pointers to remain consistent across levels. DFS and BFS require explicit state recording if I want to show the traversal process clearly. Implementing these in C also forced me to practice pointer management, dynamic allocation, cleanup, and error handling.

Visualization became a way to verify my own work. DOT files, ASCII plots, and CSV exports made it easier to check whether a data structure or algorithm was behaving as expected. Through this project, I learned that a useful CS project is not only about producing an answer, but also about designing a system that can be tested, inspected, and explained.

---

## Portfolio Value / What This Project Demonstrates

MathVista-C demonstrates:

- C programming with explicit pointers, arrays, structs, and memory management;
- modular project organization across headers and source files;
- numerical computation based on calculus concepts;
- recurrence, memoization, and dynamic programming from discrete mathematics;
- implementation of non-trivial data structures such as radix trees and skip lists;
- algorithmic thinking through sorting and graph traversal;
- visualization as a debugging and explanation tool;
- error handling through explicit status codes instead of hidden failures;
- the ability to connect mathematical foundations with computer science implementation.

For a transfer portfolio, the main value of this project is that it shows a first-year CS learning path: from classroom formulas and definitions to a working C system that can compute, visualize, and demonstrate the underlying concepts.

---

## CLI Demo Reference

```text
mathvista <demo>
  menu       Interactive demo selector
  stirling   Stirling S(n,k) triangle + MemoTable stats
  catalan    Catalan C(n) + Dyck paths
  skiplist   Skip list insert/search/delete + DOT export
  radix      Radix tree / compressed trie + DOT export
  sort       QuickSort step-through
  graph      DFS + BFS graph traversal + DOT export
  lagrange   Lagrange multiplier constrained optimization
  ascii      ASCII function plots and histogram
  csv        CSV + gnuplot script export
  mempool    Memory-pool state demo + DOT export
  transform  3-D rotation + perspective projection
  all        Run all demos
```

---

## Error Handling

Most library functions return `mv_status_t`. Use `mv_strerror(st)` to get a readable error message.

```c
mv_status_t st = sl_insert(&sl, key, value);
if (st != MV_OK)
    mv_log(MV_ERROR, "sl_insert: %s", mv_strerror(st));
```

| Code | Meaning |
|---|---|
| `MV_OK` | success |
| `MV_ERR_ALLOC` | `malloc`, `realloc`, or `fopen` failed |
| `MV_ERR_DIM_MISMATCH` | vector or matrix dimensions incompatible |
| `MV_ERR_OUT_OF_RANGE` | index or parameter out of valid range |
| `MV_ERR_SINGULAR` | degenerate matrix, zero norm, or zero gradient |
| `MV_ERR_OVERFLOW` | `uint64_t` arithmetic overflow detected |
| `MV_ERR_NULL_PTR` | required pointer argument was `NULL` |
| `MV_ERR_NOT_FOUND` | key absent in data structure |
| `MV_ERR_CONVERGE` | iterative solver did not converge within `max_iter` |

---

## Troubleshooting

| Symptom | Possible cause | Fix |
|---|---|---|
| `undefined reference to mvec_create` | source file missing or truncated | run `diagnose.bat` and check compile steps |
| `command syntax incorrect` from `.bat` | LF line endings in Windows batch file | re-save `.bat` with CRLF line endings |
| `implicit declaration of aligned_alloc` | older MinGW compatibility issue | use the current `memory_pool.c` implementation based on `malloc` |
| `?` characters in terminal output | Windows console is not using UTF-8 | run `.\run_menu.bat`, or manually run `chcp 65001` before `.\bin\mathvista.exe menu` |
| DOT files exist but no PNG appears | Graphviz is not installed or not run | install Graphviz and run `dot -Tpng ...` |
| CSV exists but no plot image appears | gnuplot is not installed or not run | install gnuplot and run `gnuplot output/sinc.gp` |

---

## Future Improvements

Possible future extensions:

- add more user-input examples for selected algorithms;
- generate a small gallery of rendered screenshots in `docs/images/`;
- add unit tests for edge cases such as empty radix tree, duplicate keys, and graph cycles;
- improve numerical-method documentation with derivations and error analysis;
- add more algorithms while keeping the project focused on first-year CS foundations.
