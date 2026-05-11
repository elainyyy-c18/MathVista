# MathVista-C

**Academic Mathematics Visualisation & Computation Engine**

A high-performance C11 library demonstrating algorithm design, numerical methods, and data-structure implementation through discrete mathematics and calculus. Every module is independently testable, zero-dependency beyond the C standard library and `libm`, and produces visual output in the terminal, Graphviz DOT, or CSV / gnuplot format.

---

## Quick Start

```bash
# Build
make

# Run a single demo
./build/mathvista stirling
./build/mathvista ascii
./build/mathvista sort

# Run every demo  (~10 ms total)
./build/mathvista all

# Build with AddressSanitizer
make debug
```

Generated files land in `output/`:

| File | Contents |
|---|---|
| `output/skiplist.dot` | Skip-list pointer diagram |
| `output/radix.dot` | Compressed-trie tree |
| `output/graph_dfs.dot` | DFS traversal coloured by vertex state |
| `output/graph_bfs.dot` | BFS traversal |
| `output/mempool.dot` | Memory-pool block map |
| `output/sinc.csv` + `.gp` | sinc(x) data + gnuplot script |
| `output/gauss2d.csv` + `.gp` | 2-D Gaussian surface |
| `output/matrix.csv` | 4×4 matrix export |

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
│   ├── transform3d.h         # 4×4 homogeneous transforms
│   ├── ds_skiplist.h         # Probabilistic skip list
│   ├── ds_radix.h            # Compressed radix tree (Patricia trie)
│   ├── algo_sort.h           # QuickSort with step callbacks
│   ├── algo_graph.h          # DFS + BFS with step callbacks
│   ├── viz_dot.h             # Graphviz DOT exporter
│   ├── viz_ascii.h           # Terminal function plotter
│   └── viz_csv.h             # CSV + gnuplot script exporter
│
├── src/
│   ├── core/
│   │   ├── math_engine.c     # MVec / MMat ops, MemoTable (open addressing)
│   │   ├── memory_pool.c     # Intrusive free-list pool + ASCII health map
│   │   └── common.c          # clock_gettime timer, ANSI logger
│   ├── discrete/
│   │   ├── stirling.c        # S(n,k): memoised recursive + rolling-DP
│   │   └── catalan.c         # C(n): memoised + Dyck path visualiser
│   ├── calculus/
│   │   ├── gradient.c        # Central-difference ∇f, Hessian, D_d f
│   │   ├── lagrange.c        # Projected gradient ascent on constraint
│   │   └── transform3d.c     # Rotation / translation / perspective
│   ├── ds/
│   │   ├── skiplist.c        # LCG random levels, direct DOT output
│   │   └── radix.c           # Edge-split insert, recursive DOT output
│   ├── algo/
│   │   ├── sort.c            # Iterative Lomuto + explicit range stack
│   │   └── graph.c           # Iterative DFS (frame stack), BFS (queue)
│   ├── viz/
│   │   ├── viz_dot.c         # Generic DotNode visitor + specialised renderers
│   │   ├── viz_ascii.c       # Two-pass row-scan plotter, histogram
│   │   └── viz_csv.c         # 1-D / 2-D sampling, gnuplot script gen
│   └── main.c                # CLI dispatcher (11 named demos)
│
├── Makefile
└── README.md
```

---

## Build Options

```makefile
make            # optimised (-O2), all warnings enabled
make lib        # produces build/libmathvista.a (link into external projects)
make debug      # adds -g -fsanitize=address for memory debugging
make test       # build + run every demo
make clean      # wipe build/
```

Compiler requirements: GCC or Clang with C11 support (`-std=c11`).  
The only external dependency is `libm` (linked via `-lm`).

---

## Modules

### Core (`math_engine`, `memory_pool`, `common`)

**MVec / MMat** — dynamic-dimension vector and row-major matrix with full operator set (`add`, `sub`, `scale`, `dot`, `norm`, `normalize`, `mul`, `transpose`). All mutating operations follow the output-parameter convention and return `mv_status_t` so callers can propagate errors cleanly.

**MemoTable** — open-addressing hash table keyed on `(int64, int64)` pairs. Internal design points:

- Power-of-2 capacity → `index = hash & (cap - 1)` avoids division in the hot path
- SplitMix64-derived mixing function for uniform distribution of `(n, k)`-style keys
- Three-state slots (empty / occupied / tombstone) for correct deletion
- Automatic rehash at load factor 0.7; tombstones are purged on every resize

**MemPool** — fixed-size block allocator using an intrusive free list: each free block's first bytes store the `MemPoolFreeNode *next` pointer, so metadata overhead is zero. `mempool_free` validates that the returned pointer lies within the buffer and is block-aligned before reinserting.

---

### Discrete Mathematics (`discrete`)

**Stirling numbers of the 2nd kind** — `S(n, k)` via memoised top-down recursion and a separate rolling-array bottom-up DP. The iterative version uses `O(k)` space and is stack-safe for large `n`. Both paths guard against `uint64_t` overflow using `__builtin_mul_overflow` / `__builtin_add_overflow`.

```
S(n,k)  k=0  k=1  k=2  k=3   k=4    k=5
n=4      0    1    7    6      1      .
n=5      0    1    15   25     10     1
```

**Catalan numbers** — computed with the convolution recurrence `C(n) = Σ C(i)·C(n-1-i)`. Dyck path visualiser enumerates all valid paths of order `n` in lexicographic order (U before D) and renders each one as an ASCII grid:

```
Dyck path #0 of order 3  (UUUDDD)
   /\
  /  \
 /    \
 ______
```

---

### Calculus (`calculus`, `transform3d`)

**Numerical differentiation** (`gradient.c`):

| Function | Formula | Cost |
|---|---|---|
| `numeric_gradient` | Central difference: `(f(x+h) - f(x-h)) / 2h` | `2n` evaluations |
| `numeric_hessian` | Mixed 4-point / diagonal 3-point | `O(n²)` evaluations |
| `numeric_directional_deriv` | `(f(x+h·d̂) - f(x-h·d̂)) / 2h` | `2` evaluations |

**Lagrange multipliers** (`lagrange.c`) — projected gradient ascent for `max f(x) s.t. g(x)=0`:

1. Compute tangent component: `step = ∇f − (∇f·∇g / ‖∇g‖²) ∇g`
2. Take ascent step: `x ← x + lr · step`
3. Newton projection back to constraint: `x ← x − (g(x) / ‖∇g‖²) ∇g`

Convergence is declared when `‖step‖ < tol`. For minimisation, negate `f`.

**3-D transforms** — all represented as 4×4 homogeneous matrices in row-major order. `t3d_project` multiplies a world-space point by the combined transform, performs the perspective divide, and maps NDC `[−1,1]²` to pixel coordinates with a Y-flip.

---

### Data Structures (`ds_skiplist`, `ds_radix`)

**Skip list** — probabilistic multi-level linked list. Level generation uses an LCG (`seed = seed * 1664525 + 1013904223`) with promotion probability 0.5. The DOT exporter colour-codes each level's forward edges: level-0 edges are drawn in near-black at `penwidth=2`, higher levels use distinct hues.

**Radix tree** — compressed trie (Patricia trie) for NUL-terminated string keys. Insertion handles three cases:

| Case | Condition | Action |
|---|---|---|
| No matching child | `find_child(rem[0]) == NULL` | Append new leaf |
| Full edge match | `lcp == edge_len` | Consume and descend |
| Partial match | `lcp < edge_len` | Split edge at LCP position |

The split allocates all new nodes before mutating any existing edge string, so a mid-split allocation failure leaves the tree intact.

---

### Algorithms (`algo_sort`, `algo_graph`)

**QuickSort** — iterative Lomuto partition with an explicit `(lo, hi)` range stack. The stack grows via `realloc` starting at 32 slots. A `size_t` underflow guard protects the `boundary - 1` expression when the pivot lands at `lo`. The step callback fires on `PARTITION_BEGIN`, `SWAP`, `PARTITION_DONE`, and `COMPLETE`.

**Graph traversal** — directed graph with per-vertex dynamic adjacency arrays.

- **DFS** uses an explicit `(vertex, adj_index)` frame stack. This correctly generates `FINISH` events (frame popped when adj exhausted) and classifies edges: `BACK_EDGE` when target is GRAY (ancestor), `CROSS_EDGE` when BLACK.
- **BFS** uses a pre-allocated non-circular queue (`g->n + 1` slots; each vertex enqueued at most once). Emits `TREE_EDGE`, `CROSS_EDGE`, `FINISH`.

---

### Visualisation (`viz_dot`, `viz_ascii`, `viz_csv`)

**viz_dot** — generic visitor pattern. `DotNode` wrappers carry `user_data` (the real struct pointer, used as the node's unique Graphviz ID and printed as its hex address), sublabels, and labelled edges. Cycle safety via a hash-set of seen `user_data` pointers. Specialised renderers for `MemoTable` (shows probe-displacement dashed edges) and `MemPool` (green = free, red = allocated, dashed blue = free-list chain).

**viz_ascii** — two-pass row-scan strategy avoids heap canvas allocation. Pass 1: sample `f` at `width` x-positions, find `ymin`/`ymax`. Pass 2: for each terminal row, decide each column's character by priority: `*` (curve) > `+` (axis crossing) > `-` (x=0 line) > `|` (y=0 col) > space.

**viz_csv** — `csv_export_fn2d` inserts blank lines between scanlines to satisfy gnuplot's `pm3d` grid convention. `gnuplot_write_1d` / `gnuplot_write_2d` generate self-contained `.gp` scripts; pipe them through `gnuplot` to produce PNG output.

---

## CLI Demo Reference

```
./build/mathvista <demo>

  stirling   Stirling S(n,k) triangle (n=0..12) + MemoTable stats
  catalan    Catalan C(n=0..14) + all Dyck paths of order 3
  skiplist   10-node skip list: insert / search / delete + DOT
  radix      9-word compressed trie + DOT
  sort       QuickSort step-through on 9 elements
  graph      DFS + BFS on a 7-node directed graph with cycle
  lagrange   max x+y s.t. x²+y²=1 and min x+y s.t. x²/4+y²=1
  ascii      sin(x), sin(x)+cos(x) overlay, x²−2, sin histogram
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
{
    mv_log(MV_ERROR, "sl_insert: %s", mv_strerror(st));
}
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