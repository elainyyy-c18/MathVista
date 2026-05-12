#include "algo_graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

mv_status_t graph_create(Graph* g, int n)
{
    if (!g || n <= 0) return MV_ERR_NULL_PTR;
    g->n = n;
    g->adj = calloc((size_t)n, sizeof(int *));
    g->deg = calloc((size_t)n, sizeof(int));
    g->cap = calloc((size_t)n, sizeof(int));
    if (!g->adj || !g->deg || !g->cap) { graph_destroy(g); return MV_ERR_ALLOC; }
    return MV_OK;
}
void graph_destroy(Graph* g)
{
    if (!g) return;
    if (g->adj)
    {
        for (int i = 0; i < g->n; ++i) free(g->adj[i]);
        free(g->adj);
    }
    free(g->deg);
    free(g->cap);
    g->adj = NULL;
    g->deg = g->cap = NULL;
    g->n = 0;
}
mv_status_t graph_add_edge(Graph* g, int from, int to)
{
    if (!g) return MV_ERR_NULL_PTR;
    if (from < 0 || from >= g->n || to < 0 || to >= g->n)
        return MV_ERR_OUT_OF_RANGE;
    if (g->deg[from] == g->cap[from])
    {
        int nc = g->cap[from] ? g->cap[from] * 2 : 4;
        int* na = realloc(g->adj[from], (size_t)nc * sizeof(int));
        if (!na) return MV_ERR_ALLOC;
        g->adj[from] = na;
        g->cap[from] = nc;
    }
    g->adj[from][g->deg[from]++] = to;
    return MV_OK;
}
void graph_print_adj(const Graph* g)
{
    if (!g) return;
    printf("Graph  n=%d\n", g->n);
    for (int v = 0; v < g->n; ++v)
    {
        printf("  %d ->", v);
        for (int i = 0; i < g->deg[v]; ++i) printf(" %d", g->adj[v][i]);
        putchar('\n');
    }
}

typedef struct
{
    int v;
    int i;
} DFSFrame;
mv_status_t graph_dfs_stepped(const Graph* g, int start, graph_step_cb cb, void* ud)
{
    if (!g || start < 0 || start >= g->n) return MV_ERR_NULL_PTR;
    VertColor* color = calloc((size_t)g->n, sizeof(VertColor));
    if (!color) return MV_ERR_ALLOC;
    DFSFrame* stack = malloc(((size_t)g->n + 1) * sizeof(DFSFrame));
    if (!stack)
    {
        free(color);
        return MV_ERR_ALLOC;
    }
    int top = 0;
    color[start] = VERT_GRAY;
    if (cb)
    {
        GraphStep s;
        s.event = GEV_DISCOVER;
        s.from = -1;
        s.to = start;
        s.depth = 0;
        cb(&s, color, g->n, ud);
    }
    stack[top].v = start;
    stack[top].i = 0;
    top++;
    while (top > 0)
    {
        DFSFrame* fr = &stack[top - 1];
        int depth = top - 1;
        if (fr->i < g->deg[fr->v])
        {
            int nb = g->adj[fr->v][fr->i];
            fr->i++;
            if (color[nb] == VERT_WHITE)
            {
                color[nb] = VERT_GRAY;
                if (cb)
                {
                    GraphStep te;
                    te.event = GEV_TREE_EDGE;
                    te.from = fr->v;
                    te.to = nb;
                    te.depth = depth;
                    cb(&te, color, g->n, ud);
                    GraphStep dv;
                    dv.event = GEV_DISCOVER;
                    dv.from = fr->v;
                    dv.to = nb;
                    dv.depth = depth + 1;
                    cb(&dv, color, g->n, ud);
                }
                stack[top].v = nb;
                stack[top].i = 0;
                top++;
            }
            else if (color[nb] == VERT_GRAY)
            {
                if (cb)
                {
                    GraphStep be;
                    be.event = GEV_BACK_EDGE;
                    be.from = fr->v;
                    be.to = nb;
                    be.depth = depth;
                    cb(&be, color, g->n, ud);
                }
            }
            else
            {
                if (cb)
                {
                    GraphStep ce;
                    ce.event = GEV_CROSS_EDGE;
                    ce.from = fr->v;
                    ce.to = nb;
                    ce.depth = depth;
                    cb(&ce, color, g->n, ud);
                }
            }
        }
        else
        {
            color[fr->v] = VERT_BLACK;
            if (cb)
            {
                GraphStep fn;
                fn.event = GEV_FINISH;
                fn.from = -1;
                fn.to = fr->v;
                fn.depth = depth;
                cb(&fn, color, g->n, ud);
            }
            top--;
        }
    }
    free(stack);
    free(color);
    return MV_OK;
}

typedef struct
{
    int* data;
    int head;
    int tail;
    int cap;
} BFSQueue;
static mv_status_t bq_init(BFSQueue* q, int cap)
{
    q->data = malloc((size_t)cap * sizeof(int));
    if (!q->data) return MV_ERR_ALLOC;
    q->head = q->tail = 0;
    q->cap = cap;
    return MV_OK;
}
static void bq_push(BFSQueue* q, int v)
{
    q->data[q->tail++] = v;
}
static bool bq_pop(BFSQueue* q, int* v)
{
    if (q->head == q->tail) return false;
    *v = q->data[q->head++];
    return true;
}
mv_status_t graph_bfs_stepped(const Graph* g, int start, graph_step_cb cb, void* ud)
{
    if (!g || start < 0 || start >= g->n) return MV_ERR_NULL_PTR;
    VertColor* color = calloc((size_t)g->n, sizeof(VertColor));
    int* dist = calloc((size_t)g->n, sizeof(int));
    if (!color || !dist)
    {
        free(color);
        free(dist);
        return MV_ERR_ALLOC;
    }
    BFSQueue q;
    mv_status_t st = bq_init(&q, g->n + 1);
    if (st != MV_OK)
    {
        free(color);
        free(dist);
        return st;
    }
    color[start] = VERT_GRAY;
    bq_push(&q, start);
    if (cb)
    {
        GraphStep s;
        s.event = GEV_DISCOVER;
        s.from = -1;
        s.to = start;
        s.depth = 0;
        cb(&s, color, g->n, ud);
    }
    int v;
    while (bq_pop(&q, &v))
    {
        for (int i = 0; i < g->deg[v]; ++i)
        {
            int nb = g->adj[v][i];
            if (color[nb] == VERT_WHITE)
            {
                color[nb] = VERT_GRAY;
                dist[nb] = dist[v] + 1;
                bq_push(&q, nb);
                if (cb)
                {
                    GraphStep te;
                    te.event = GEV_TREE_EDGE;
                    te.from = v;
                    te.to = nb;
                    te.depth = dist[nb];
                    cb(&te, color, g->n, ud);
                    GraphStep dv;
                    dv.event = GEV_DISCOVER;
                    dv.from = v;
                    dv.to = nb;
                    dv.depth = dist[nb];
                    cb(&dv, color, g->n, ud);
                }
            }
            else if (cb)
            {
                GraphStep ce;
                ce.event = GEV_CROSS_EDGE;
                ce.from = v;
                ce.to = nb;
                ce.depth = dist[v];
                cb(&ce, color, g->n, ud);
            }
        }
        color[v] = VERT_BLACK;
        if (cb)
        {
            GraphStep fn;
            fn.event = GEV_FINISH;
            fn.from = -1;
            fn.to = v;
            fn.depth = dist[v];
            cb(&fn, color, g->n, ud);
        }
    }

    free(q.data);
    free(color);
    free(dist);
    return MV_OK;
}

void graph_print_step(const GraphStep* step, const VertColor* color, int n, void* ud)
{
    (void)ud;
    if (!step) return;
    static const char* ev_str[] =
    {
        "DISCOVER  ", "FINISH    ",
        "TREE_EDGE ", "BACK_EDGE ", "CROSS_EDGE"
    };
    static const char* col_char[] =
    {
        "W", "G", "B"
    };
    printf("  %-10s  from=%2d  to=%2d  depth=%d  colors:[", ev_str[step->event], step->from, step->to, step->depth);
    for (int i = 0; i < n; ++i)
        printf("%d:%s%s", i, col_char[(int)color[i]], i + 1 < n ? " " : "");
    printf("]\n");
}

mv_status_t graph_export_dot(const Graph* g, const VertColor* color, const char* filepath, const char* name)
{
    if (!g || !filepath) return MV_ERR_NULL_PTR;
    FILE*fp = fopen(filepath, "w");
    if (!fp) return MV_ERR_ALLOC;
    static const char* fc[] = { "#ddeeff", "#ffdd88", "#aaaaaa" };
    fprintf(fp, "digraph %s {\n", name ? name : "G");
    fprintf(fp, "  rankdir=TB;\n");
    fprintf(fp, "  node [fontname=\"Menlo\", fontsize=12,"
            " shape=circle, style=filled, fixedsize=true,"
            " width=0.6];\n");
    fprintf(fp, "  edge [fontname=\"Menlo\", fontsize=9];\n\n");
    for (int v = 0; v < g->n; ++v)
    {
        const char* fill = color ? fc[(int)color[v]] : "#ddeeff";
        fprintf(fp, "  v%d [label=\"%d\", fillcolor=\"%s\"];\n", v, v, fill);
    }
    fprintf(fp, "\n");

    for (int v = 0; v < g->n; ++v)
        for (int i = 0; i < g->deg[v]; ++i)
            fprintf(fp, "  v%d -> v%d;\n", v, g->adj[v][i]);

    fprintf(fp, "}\n");
    fclose(fp);
    return MV_OK;
}