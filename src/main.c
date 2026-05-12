#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <direct.h>
#define MV_MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define MV_MKDIR(d) mkdir((d), 0755)
#endif

#include "common.h"
#include "math_engine.h"
#include "memory_pool.h"
#include "discrete.h"
#include "calculus.h"
#include "transform3d.h"
#include "ds_skiplist.h"
#include "ds_radix.h"
#include "algo_sort.h"
#include "algo_graph.h"
#include "viz_dot.h"
#include "viz_ascii.h"
#include "viz_csv.h"

#define OUT(path) "output/" path

static void demo_stirling()
{
    mv_banner("Stirling Numbers of the 2nd Kind  S(n,k)");
    MemoTable memo;
    memo_create(&memo, 64);
    MvTimer t;
    mv_timer_start(&t);
    const int N = 12;
    printf("        ");
    for (int k = 0; k <= N; ++k) printf("  k=%-4d", k);
    putchar('\n');
    mv_rule();
    for (int n = 0; n <= N; ++n)
    {
        printf("  n=%-3d |", n);
        for (int k = 0; k <= N; ++k)
        {
            uint64_t v;
            stirling2_memoized(n, k, &memo, &v);
            if (k <= n) printf("  %-6llu", (unsigned long long)v);
            else printf("  %-6s", ".");
        }
        putchar('\n');
    }
    printf("\n  MemoTable: capacity=%zu  entries=%zu  load=%.2f  time=%.2f ms\n", memo.capacity, memo_size(&memo), memo_load_factor(&memo), mv_timer_elapsed_ms(&t));
    uint64_t a, b;
    stirling2_memoized(15, 7, &memo, &a);
    stirling2_iterative(15, 7, &b);
    printf("  S(15,7) memo=%llu  iter=%llu  %s\n",
           (unsigned long long)a, (unsigned long long)b,
           a == b ? "✓ match" : "✗ MISMATCH");
    memo_destroy(&memo);
}

static void demo_catalan()
{
    mv_banner("Catalan Numbers  C(n)  +  Dyck Paths");
    MemoTable memo;
    memo_create(&memo, 32);
    printf("  n :  ");
    for (int n = 0; n <= 14; ++n) printf("%-8d", n);
    printf("\n  C :  ");
    for (int n = 0; n <= 14; ++n)
    {
        uint64_t c;
        catalan_memoized(n, &memo, &c);
        printf("%-8llu", (unsigned long long)c);
    }
    putchar('\n');
    memo_destroy(&memo);
    printf("\n  Dyck paths of order 3:\n");
    for (int i = 0; i < 5; ++i) catalan_render_dyck(3, i);
}
static void demo_skiplist()
{
    mv_banner("Skip List");
    SkipList sl;
    sl_create(&sl);
    int data[] = { 42, 7, 19, 3, 55, 11, 31, 27, 88, 5 };
    for (int i = 0; i < 10; ++i) sl_insert(&sl, data[i], data[i] * 100);
    sl_print(&sl);

    int v;
    printf("\n  search(19) = %s", sl_search(&sl, 19, &v) ? "found" : "not found");
    printf("  value=%d\n", v);
    sl_delete(&sl, 19);
    printf("  after delete(19): search(19) = %s\n", sl_search(&sl, 19, NULL) ? "found" : "not found");
    sl_export_dot(&sl, OUT("skiplist.dot"));
    printf("  exported → " OUT("skiplist.dot") "\n");
    sl_destroy(&sl);
}
static void demo_radix()
{
    mv_banner("Radix Tree (Compressed Trie)");
    RadixTree rt;
    radix_create(&rt);
    const char* words[] =
    {
        "apple", "application", "app", "apt",
        "banana", "band", "bandana", "ban", "cat"
    };
    int n = (int)(sizeof(words) / sizeof(words[0]));
    for (int i = 0; i < n; ++i) radix_insert(&rt, words[i], i + 1);
    radix_print(&rt);
    printf("\n  Lookups:\n");
    int v;
    for (int i = 0; i < n; ++i)
    {
        bool hit = radix_search(&rt, words[i], &v);
        printf("    %-15s  %s  val=%d\n", words[i], hit ? "HIT " : "MISS", v);
    }
    printf("    %-15s  %s\n", "apricot", radix_search(&rt, "apricot", NULL) ? "HIT" : "MISS");
    radix_export_dot(&rt, OUT("radix.dot"));
    printf("\n  exported → " OUT("radix.dot") "\n");
    radix_destroy(&rt);
}

static void demo_sort()
{
    mv_banner("QuickSort  (Iterative Lomuto, step-through)");
    int arr[] = { 64, 34, 25, 12, 22, 11, 90, 3, 47 };
    int n = (int)(sizeof(arr) / sizeof(arr[0]));
    printf("  Input:  ");
    for (int i = 0; i < n; ++i) printf("%d ", arr[i]);
    putchar('\n');
    mv_rule();
    quicksort_stepped(arr, (size_t)n, qsort_print_step, NULL);
}

static void demo_graph()
{
    mv_banner("Graph  DFS + BFS  (directed, n=7)");
    Graph g;
    graph_create(&g, 7);
    graph_add_edge(&g, 0, 1);
    graph_add_edge(&g, 0, 2);
    graph_add_edge(&g, 1, 3);
    graph_add_edge(&g, 1, 2);
    graph_add_edge(&g, 2, 4);
    graph_add_edge(&g, 3, 0);
    graph_add_edge(&g, 3, 5);
    graph_add_edge(&g, 4, 6);
    graph_add_edge(&g, 6, 5);
    graph_print_adj(&g);
    printf("\n--- DFS from 0 ---\n");
    graph_dfs_stepped(&g, 0, graph_print_step, NULL);
    graph_export_dot(&g, NULL, OUT("graph_dfs.dot"), "DFS");
    printf("\n--- BFS from 0 ---\n");
    graph_bfs_stepped(&g, 0, graph_print_step, NULL);
    graph_export_dot(&g, NULL, OUT("graph_bfs.dot"), "BFS");
    printf("\n  exported → " OUT("graph_dfs.dot")"  " OUT("graph_bfs.dot") "\n");
    graph_destroy(&g);
}

static double obj_xpy(const MVec* x, void* ud)
{
    (void)ud;
    return x->data[0] + x->data[1];
}
static double con_circle(const MVec* x, void* ud)
{
    (void)ud;
    return x->data[0]*x->data[0] + x->data[1]*x->data[1] - 1.0;
}
static double obj_nxpy(const MVec* x, void* ud)
{
    (void)ud;
    return -(x->data[0] + x->data[1]);
}
static double con_ellipse(const MVec* x, void* ud)
{
    (void)ud;
    return x->data[0]*x->data[0]/4.0 + x->data[1]*x->data[1] - 1.0;
}

static void demo_lagrange()
{
    mv_banner("Lagrange Multipliers");
    printf("  max x+y  s.t.  x²+y²=1\n");
    double xi[] = { 0.9, 0.1 };
    MVec xinit, xout;
    mvec_create_from(&xinit, xi, 2);
    mvec_create(&xout, 2);
    double lam;
    mv_status_t st = lagrange_solve(&xout, &lam, obj_xpy, con_circle, NULL, NULL, &xinit, 2000, 1e-9);
    printf("  result : x=(%.6f, %.6f)  λ=%.6f  status=%s\n", xout.data[0], xout.data[1], lam, mv_strerror(st));
    printf("  analytic: (%.6f, %.6f)  λ=%.6f\n", sqrt(2.0)/2, sqrt(2.0)/2, sqrt(2.0)/2);
    mvec_destroy(&xinit); mvec_destroy(&xout);
    printf("\n  min x+y  s.t.  x²/4+y²=1\n");
    double xi2[] = { -1.0, -0.5 };
    MVec xi2v, xo2;
    mvec_create_from(&xi2v, xi2, 2);
    mvec_create(&xo2, 2);
    st = lagrange_solve(&xo2, &lam, obj_nxpy, con_ellipse, NULL, NULL, &xi2v, 2000, 1e-9);
    printf("  result : x=(%.6f, %.6f)  g(x)=%.2e  status=%s\n", xo2.data[0], xo2.data[1], xo2.data[0]*xo2.data[0]/4.0 + xo2.data[1]*xo2.data[1] - 1.0, mv_strerror(st));
    mvec_destroy(&xi2v); mvec_destroy(&xo2);
}

static double fn_sin(const MVec* x, void* ud)
{
    (void)ud;
    return sin(x->data[0]);
}
static double fn_cos(const MVec* x, void* ud)
{
    (void)ud;
    return cos(x->data[0]);
}
static double fn_x2(const MVec* x, void* ud)
{
    (void)ud;
    return x->data[0]*x->data[0] - 2.0;
}

static void demo_ascii()
{
    mv_banner("ASCII Function Plotter");
    printf("  f(x) = sin(x)  on [-2π, 2π]\n\n");
    ascii_plot_fn(fn_sin, NULL, -6.28, 6.28, 72, 16);
    printf("\n  f1(x)=sin(x)  f2(x)=cos(x)  on [0, 2π]\n\n");
    ascii_plot_fn2(fn_sin, NULL, fn_cos, NULL, 0.0, 6.28, 72, 14);
    printf("\n  f(x) = x²-2  on [-3, 3]  (zero crossings visible)\n\n");
    ascii_plot_fn(fn_x2, NULL, -3.0, 3.0, 72, 14);

    double samples[200];
    for (int i = 0; i < 200; ++i)
        samples[i] = sin(i * 6.28318 / 200.0);
    printf("\n  Histogram of sin(x)  (200 samples, 16 bins)\n\n");
    ascii_histogram(samples, 200, 16, 40);
}

static double fn_sinc(const MVec* x, void* ud)
{
    (void)ud;
    double v = x->data[0];
    return fabs(v) < 1e-9 ? 1.0 : sin(v) / v;
}
static double fn_gauss(const MVec* x, void* ud)
{
    (void)ud;
    return exp(-(x->data[0]*x->data[0] + x->data[1]*x->data[1]) / 2.0);
}
static void demo_csv()
{
    mv_banner("CSV + gnuplot Script Export");
    csv_export_fn(OUT("sinc.csv"), fn_sinc, NULL, -20.0, 20.0, 400);
    gnuplot_write_1d(OUT("sinc.gp"), OUT("sinc.csv"), OUT("sinc.png"), "sinc(x) = sin(x)/x");
    printf("  sinc(x)    → " OUT("sinc.csv") "  " OUT("sinc.gp") "\n");
    csv_export_fn2d(OUT("gauss2d.csv"), fn_gauss, NULL, -3.0, 3.0, 40, -3.0, 3.0, 40);
    gnuplot_write_2d(OUT("gauss2d.gp"), OUT("gauss2d.csv"), OUT("gauss2d.png"), "Gaussian  e^{-(x²+y²)/2}", 40, 40);
    printf("  Gaussian2D → " OUT("gauss2d.csv") "  " OUT("gauss2d.gp") "\n");

    MMat m;
    mmat_identity(&m, 4);
    MMAT_AT(&m, 0, 2) = 3.14; MMAT_AT(&m, 1, 3) = -2.71;
    csv_export_mat(OUT("matrix.csv"), &m);
    printf("  4×4 matrix → " OUT("matrix.csv") "\n");
    ascii_print_mat(&m, "  matrix");
    mmat_destroy(&m);

    printf("\n  Render with:\n"
           "    gnuplot " OUT("sinc.gp") "\n"
           "    gnuplot " OUT("gauss2d.gp") "\n");
}

static void demo_mempool()
{
    mv_banner("Memory Pool  Health Check");
    MemPool pool;
    mempool_create(&pool, 48, 64);
    void* ptrs[40];
    for (int i = 0; i < 40; ++i) ptrs[i] = mempool_alloc(&pool);
    int primes[] = { 2,3,5,7,11,13,17,19,23,29,31,37 };
    for (int i = 0; i < 12; ++i) mempool_free(&pool, ptrs[primes[i]]);
    mempool_render_ascii(&pool);
    dot_export_mem_pool(OUT("mempool.dot"), &pool, "MemPool");
    printf("  exported → " OUT("mempool.dot") "\n");
    mempool_destroy(&pool);
}

static void demo_transform()
{
    mv_banner("3-D Perspective Projection");
    MMat Rx, Ry, P, tmp, T;
    mmat_create(&Rx,  4, 4);
    mmat_create(&Ry,  4, 4);
    mmat_create(&P,   4, 4);
    mmat_create(&tmp, 4, 4);
    mmat_create(&T,   4, 4);
    t3d_rotation_x(&Rx, 0.3);
    t3d_rotation_y(&Ry, 0.5);
    t3d_perspective(&P, 3.14159265 / 3.0, 16.0 / 9.0, 0.1, 100.0);
    mmat_mul(&tmp, &Ry, &Rx);
    mmat_mul(&T,   &P,  &tmp);
    printf("  Unit cube (centred at z=-3) projected to 800×600 screen:\n");
    double corners[8][3] =
    {
        {-1,-1,-2},{1,-1,-2},{1,1,-2},{-1,1,-2},
        {-1,-1,-4},{1,-1,-4},{1,1,-4},{-1,1,-4}
    };
    for (int i = 0; i < 8; ++i)
    {
        double px, py;
        t3d_project(&px, &py, &T, corners[i][0], corners[i][1], corners[i][2], 800, 600);
        printf("    (%+.1f,%+.1f,%+.1f) → screen (%.1f, %.1f)\n", corners[i][0], corners[i][1], corners[i][2], px, py);
    }
    mmat_destroy(&Rx);
    mmat_destroy(&Ry);
    mmat_destroy(&P);
    mmat_destroy(&tmp);
    mmat_destroy(&T);
}

typedef struct
{
    const char* name;
    void (*fn)(void);
    const char* desc;
} Demo;
static Demo demos[] =
{
    { "stirling",  demo_stirling,  "Stirling S(n,k) triangle + memo stats"        },
    { "catalan",   demo_catalan,   "Catalan numbers + Dyck path ASCII render"      },
    { "skiplist",  demo_skiplist,  "Skip list insert/search/delete + DOT export"   },
    { "radix",     demo_radix,     "Radix tree (compressed trie) + DOT export"     },
    { "sort",      demo_sort,      "QuickSort step-through (iterative Lomuto)"     },
    { "graph",     demo_graph,     "DFS + BFS step-through + DOT export"           },
    { "lagrange",  demo_lagrange,  "Lagrange multipliers (circle + ellipse)"       },
    { "ascii",     demo_ascii,     "ASCII function plotter + histogram"            },
    { "csv",       demo_csv,       "CSV + gnuplot script export"                   },
    { "mempool",   demo_mempool,   "Memory pool health: ASCII + DOT"               },
    { "transform", demo_transform, "3-D rotation + perspective projection"         },
};

static int n_demos = (int)(sizeof(demos) / sizeof(demos[0]));
static void print_usage(const char* prog)
{
    printf("Usage: %s [demo ...]\n\n  Available demos:\n", prog);
    for (int i = 0; i < n_demos; ++i)
        printf("    %-12s  %s\n", demos[i].name, demos[i].desc);
    printf(" %-12s Run every demo\n", "all");
    printf(" %-12s Open interactive demo menu\n\n", "menu");
}
static int demo_index_by_name(const char* name) {
    for (int i = 0; i < n_demos; ++i) {
        if (strcmp(demos[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void run_demo_by_name(const char* name) {
    int idx = demo_index_by_name(name);

    if (idx < 0) {
        printf("Demo '%s' was not found.\n", name);
        return;
    }

    demos[idx].fn();
}

static int read_menu_choice(void) {
    char line[64];
    int choice;
    char extra;

    printf("Choose a demo: ");
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
    }

    if (sscanf(line, " %d %c", &choice, &extra) != 1) {
        return -1;
    }

    return choice;
}

static void wait_for_enter(void) {
    char line[64];

    printf("\nPress Enter to return to the menu...");
    fflush(stdout);

    fgets(line, sizeof(line), stdin);
}

static void run_interactive_menu(const char* prog)
{
    (void)prog;
    while (1)
    {
        printf("\n");
        mv_banner("MathVista-C Interactive Demo");
        printf(" 1. Discrete Math Demos\n");
        printf(" 2. Calculus / Visualization Demos\n");
        printf(" 3. QuickSort Visualization\n");
        printf(" 4. DFS / BFS Graph Traversal\n");
        printf(" 5. Radix Tree Visualization\n");
        printf(" 6. Skip List Visualization\n");
        printf(" 7. Memory Pool Demo\n");
        printf(" 8. 3-D Transform Demo\n");
        printf(" 9. Run All Demos\n");
        printf(" 0. Exit\n\n");

        int choice = read_menu_choice();
        printf("\n");

        switch (choice)
        {
            case 1:
                run_demo_by_name("stirling");
                run_demo_by_name("catalan");
                wait_for_enter();
                break;
            case 2:
                run_demo_by_name("lagrange");
                run_demo_by_name("ascii");
                run_demo_by_name("csv");
                wait_for_enter();
                break;
            case 3:
                run_demo_by_name("sort");
                wait_for_enter();
                break;
            case 4:
                run_demo_by_name("graph");
                wait_for_enter();
                break;
            case 5:
                run_demo_by_name("radix");
                wait_for_enter();
                break;
            case 6:
                run_demo_by_name("skiplist");
                wait_for_enter();
                break;
            case 7:
                run_demo_by_name("mempool");
                wait_for_enter();
                break;
            case 8:
                run_demo_by_name("transform");
                wait_for_enter();
                break;
            case 9:
                for (int i = 0; i < n_demos; ++i)
                {
                    demos[i].fn();
                }
                wait_for_enter();
                break;
            case 0:
                printf("Goodbye.\n");
                return;
            default:
                printf("Invalid choice. Please enter a number from 0 to 9.\n");
                wait_for_enter();
                break;
        }
    }
}

int main(int argc, char** argv)
{
    (void)MV_MKDIR("output");
    if (argc < 2)
    {
        print_usage(argv[0]);
        printf("  No demo specified – running 'stirling' as default.\n");
        demo_stirling();
        return 0;
    }
    if (strcmp(argv[1], "menu") == 0 || strcmp(argv[1], "--menu") == 0)
    {
        run_interactive_menu(argv[0]);
        return 0;
    }
    MvTimer total;
    mv_timer_start(&total);
    for (int a = 1; a < argc; ++a)
    {
        if (strcmp(argv[a], "all") == 0)
        {
            for (int i = 0; i < n_demos; ++i) demos[i].fn();
            break;
        }
        bool found = false;
        for (int i = 0; i < n_demos; ++i)
        {
            if (strcmp(argv[a], demos[i].name) == 0)
            {
                demos[i].fn();
                found = true;
                break;
            }
        }
        if (!found)
        {
            mv_log(MV_WARN, "Unknown demo '%s' – skipped.", argv[a]);
            print_usage(argv[0]);
        }
    }
    printf("\n  Total elapsed: %.2f ms\n", mv_timer_elapsed_ms(&total));
    return 0;
}