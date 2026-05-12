# MathVista-C

**Academic Mathematics Visualisation & Computation Engine**

A high-performance C11 library demonstrating algorithm design, numerical methods, and data-structure implementation through discrete mathematics and calculus. Every module is independently testable, zero-dependency beyond the C standard library and `libm`, and produces visual output in the terminal, Graphviz DOT, or CSV / gnuplot format.

---

## Quick Start

### Windows (native cmd / PowerShell + MinGW-w64)

```powershell
.\build.bat
.\bin\mathvista.exe all
```

> Tip: if you see `?` characters in the output (e.g. `??output/...` or `4?4 matrix`),
> your console is not in UTF-8. Run `chcp 65001` once per terminal session
> to fix arrow / Greek / superscript characters.

### Linux / macOS / MSYS2 / WSL

```bash
make
./bin/mathvista all
```

### Single-demo run

```
.\bin\mathvista.exe stirling     (Windows)
./bin/mathvista stirling   (Unix)
```

Replace `stirling` with any of: `catalan`, `skiplist`, `radix`, `sort`,
`graph`, `lagrange`, `ascii`, `csv`, `mempool`, `transform`, or `all`.

---

## What gets generated

Demo output files land in `output/`:

| File | Contents |
|---|---|
| `output/skiplist.dot` | Skip-list pointer diagram |
| `output/radix.dot` | Compressed-trie tree |
| `output/graph_dfs.dot` | DFS traversal coloured by vertex state |
| `output/graph_bfs.dot` | BFS traversal |
| `output/mempool.dot` | Memory-pool block map |
| `output/sinc.csv` + `.gp` | sinc(x) data + gnuplot script |
| `output/gauss2d.csv` + `.gp` | 2-D Gaussian surface |
| `output/matrix.csv` | 4x4 matrix export |

Render DOT files: `dot -Tpng output/skiplist.dot -o skiplist.png`  
Render plots: `gnuplot output/sinc.gp`

---

## Project Structure

```
MathVista-C/
├── include/                  # 13 public headers (flat, no sub-directory)
│   ├── math_engine.h         # MVec, MMat, MemoTable
│   ├── memory_pool.h         # Fixed-block memory pool
│   ├── common.h              # Timer, logger, banner
│   ├── discrete.h            # Stirling, Catalan, Dyck paths
│   ├── calculus.h            # Gradient, Hessian, Lagrange
│   ├── transform3d.h         # 4x4 homogeneous transforms
│   ├── ds_skiplist.h         # Probabilistic skip list
│   ├── ds_radix.h            # Compressed radix tree
│   ├── algo_sort.h           # QuickSort with step callbacks
│   ├── algo_graph.h          # DFS + BFS with step callbacks
│   ├── viz_dot.h             # Graphviz DOT exporter
│   ├── viz_ascii.h           # Terminal function plotter
│   └── viz_csv.h             # CSV + gnuplot script exporter
│
├── src/
│   ├── core/                 # math_engine.c, memory_pool.c, common.c
│   ├── discrete/             # stirling.c, catalan.c
│   ├── calculus/             # gradient.c, lagrange.c, transform3d.c
│   ├── ds/                   # skiplist.c, radix.c
│   ├── algo/                 # sort.c, graph.c
│   ├── viz/                  # viz_dot.c, viz_ascii.c, viz_csv.c
│   └── main.c                # CLI dispatcher (11 named demos)
│
├── build.bat                 # Windows native build
├── diagnose.bat              # Per-file compile diagnostic for Windows
├── Makefile                  # Unix build (gmake / GNU Make)
└── README.md
```

---

## Build System Details

Both `build.bat` and the `Makefile` run the same underlying compilation:

```
gcc -std=c11 -Wall -Wextra -O2 -Iinclude  <all 16 .c files>  -o bin/mathvista  -lm
```

The single `-Iinclude` flag is what makes every `#include "math_engine.h"`
resolve correctly regardless of which subdirectory the `.c` file is in.
**Do not** use `../../include/xxx.h` relative paths — they break header-to-header
references and are unnecessary when `-Iinclude` is set.

### Makefile targets (Unix)

```makefile
make            # optimised (-O2), all warnings enabled
make lib        # produces build/libmathvista.a (link into external projects)
make debug      # adds -g -fsanitize=address for memory debugging
make test       # build + run every demo
make clean      # wipe build/
```

### Windows scripts

| Script | Purpose |
|---|---|
| `build.bat` | One-shot full build, produces `bin/mathvista.exe` |
| `diagnose.bat` | Compiles each `.c` separately to pinpoint which file fails; useful when symbols go missing |

### Compiler requirements

- GCC or Clang with C11 support (`-std=c11`)
- The only external dependency is `libm` (linked via `-lm`)
- On Windows, **MinGW-w64** is recommended (download a recent winlibs build)
- `aligned_alloc` is intentionally avoided — `malloc` is used everywhere
  because MinGW does not expose `aligned_alloc` in `<stdlib.h>`. Alignment
  is achieved by rounding block sizes up to 16 bytes before allocation,
  which `malloc` already guarantees on 64-bit systems.

---

## Module Reference

### Core (`math_engine`, `memory_pool`, `common`)

**MVec / MMat** — dynamic-dimension vector and row-major matrix with full operator set (`add`, `sub`, `scale`, `dot`, `norm`, `normalize`, `mul`, `transpose`). All mutating operations follow the output-parameter convention and return `mv_status_t` so callers can propagate errors cleanly.

**MemoTable** — open-addressing hash table keyed on `(int64, int64)` pairs:

- Power-of-2 capacity so `index = hash & (cap - 1)` avoids division in the hot path
- SplitMix64-derived mixing for uniform distribution of `(n, k)`-style keys
- Three-state slots (empty / occupied / tombstone) for correct deletion
- Automatic rehash at load factor 0.7; tombstones are purged on every resize

**MemPool** — fixed-size block allocator using an intrusive free list: each free block's first bytes store the `MemPoolFreeNode *next` pointer, so metadata overhead is zero.

### Discrete Mathematics (`discrete`)

**Stirling numbers of the 2nd kind** — `S(n, k)` via memoised top-down recursion and a separate rolling-array bottom-up DP. Both paths guard against `uint64_t` overflow using `__builtin_mul_overflow` / `__builtin_add_overflow`.

**Catalan numbers** — computed with the convolution recurrence `C(n) = sum C(i) * C(n-1-i)`. Dyck path visualiser enumerates all valid paths of order `n` in lexicographic order.

### Calculus (`calculus`, `transform3d`)

**Numerical differentiation** (`gradient.c`):

| Function | Formula | Cost |
|---|---|---|
| `numeric_gradient` | Central difference: `(f(x+h) - f(x-h)) / 2h` | `2n` evaluations |
| `numeric_hessian` | Mixed 4-point / diagonal 3-point | `O(n^2)` evaluations |
| `numeric_directional_deriv` | `(f(x+h*d) - f(x-h*d)) / 2h` | `2` evaluations |

**Lagrange multipliers** (`lagrange.c`) — projected gradient ascent for `max f(x) s.t. g(x) = 0`. Two-step iteration: tangent step + Newton projection back to the constraint.

**3-D transforms** — all represented as 4x4 homogeneous matrices in row-major order. `t3d_project` multiplies a world-space point by the combined transform, performs the perspective divide, and maps NDC to pixel coordinates.

### Data Structures (`ds_skiplist`, `ds_radix`)

**Skip list** — probabilistic multi-level linked list. Level generation uses an LCG with promotion probability 0.5.

**Radix tree** — compressed trie for NUL-terminated string keys. Insertion handles three cases: no matching child (append leaf), full edge match (descend), partial match (split edge at LCP position).

### Algorithms (`algo_sort`, `algo_graph`)

**QuickSort** — iterative Lomuto partition with an explicit `(lo, hi)` range stack. A `size_t` underflow guard protects `boundary - 1` when the pivot lands at `lo`.

**Graph traversal** — directed graph with per-vertex dynamic adjacency arrays. DFS uses an explicit `(vertex, adj_index)` frame stack so it correctly classifies `TREE_EDGE` / `BACK_EDGE` / `CROSS_EDGE`. BFS uses a pre-allocated queue.

### Visualisation (`viz_dot`, `viz_ascii`, `viz_csv`)

**viz_dot** — generic visitor pattern. `DotNode` wrappers carry `user_data` (printed as hex address) and labelled edges. Cycle safety via a hash-set of seen pointers. Specialised renderers for `MemoTable` and `MemPool`.

**viz_ascii** — two-pass row-scan strategy avoids heap canvas allocation. Character priority: `*` (curve) > `+` (axis crossing) > `-` (x=0 line) > `|` (y=0 col) > space.

**viz_csv** — `csv_export_fn2d` inserts blank lines between scanlines to satisfy gnuplot's `pm3d` grid convention.

---

## CLI Demo Reference

```
mathvista <demo>

  stirling   Stirling S(n,k) triangle (n=0..12) + MemoTable stats
  catalan    Catalan C(n=0..14) + all Dyck paths of order 3
  skiplist   10-node skip list: insert / search / delete + DOT
  radix      9-word compressed trie + DOT
  sort       QuickSort step-through on 9 elements
  graph      DFS + BFS on a 7-node directed graph with cycle
  lagrange   max x+y s.t. x^2+y^2=1  and  min x+y s.t. x^2/4+y^2=1
  ascii      sin(x), sin(x)+cos(x) overlay, x^2-2, sin histogram
  csv        sinc(x) 1-D + Gaussian 2-D surface + gnuplot scripts
  mempool    64-block pool with scattered frees: ASCII + DOT
  transform  8 unit-cube corners projected through perspective matrix
  all        All of the above
```

---

## Error Handling

All library functions return `mv_status_t`. Use `mv_strerror(st)` to get a human-readable string:

```c
mv_status_t st = sl_insert(&sl, key, value);
if (st != MV_OK)
    mv_log(MV_ERROR, "sl_insert: %s", mv_strerror(st));
```

| Code | Meaning |
|---|---|
| `MV_OK` | Success |
| `MV_ERR_ALLOC` | `malloc` / `realloc` / `fopen` failed |
| `MV_ERR_DIM_MISMATCH` | Vector or matrix dimensions incompatible |
| `MV_ERR_OUT_OF_RANGE` | Index or parameter out of valid range |
| `MV_ERR_SINGULAR` | Degenerate matrix, zero norm, or zero gradient |
| `MV_ERR_OVERFLOW` | `uint64_t` arithmetic overflow detected |
| `MV_ERR_NULL_PTR` | Required pointer argument was NULL |
| `MV_ERR_NOT_FOUND` | Key absent in data structure |
| `MV_ERR_CONVERGE` | Iterative solver did not converge within `max_iter` |

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `undefined reference to mvec_create` (and many others) | A source file (typically `math_engine.c`) was truncated to a partial copy | Run `diagnose.bat` — Step 1 shows file sizes; `math_engine.c` should be around 9.8 KB |
| `command syntax incorrect` from a `.bat` file | LF line endings (Unix) on a Windows batch file | Re-download `build.bat` / `diagnose.bat` with CRLF line endings |
| `implicit declaration of aligned_alloc` | Older `memory_pool.c` using `aligned_alloc` (not in MinGW) | Get the latest `memory_pool.c` which uses plain `malloc` |
| `?` characters in console output | Console code page is CP950/CP1252, not UTF-8 | Run `chcp 65001` in the same PowerShell session before running mathvista |
| PowerShell drops files from a multi-line `gcc ... ` command | Trailing whitespace after a backtick (`` ` ``) breaks line continuation | Use `build.bat` instead of typing the gcc command directly |