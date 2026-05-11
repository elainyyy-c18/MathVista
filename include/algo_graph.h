#ifndef MATHVISTA_ALGO_GRAPH_H
#define MATHVISTA_ALGO_GRAPH_H
#include <stddef.h>
#include "math_engine.h"

typedef enum
{
    VERT_WHITE = 0,
    VERT_GRAY = 1,
    VERT_BLACK = 2
} VertColor;
typedef enum
{
    GEV_DISCOVER,
    GEV_FINISH,
    GEV_TREE_EDGE,
    GEV_BACK_EDGE,
    GEV_CROSS_EDGE
} GraphEvent;

typedef struct
{
    GraphEvent event;
    int from;
    int to;
    int depth;
} GraphStep;
typedef void (*graph_step_cb)(const GraphStep* step, const VertColor* color, int n, void* ud);

typedef struct
{
    int n;
    int** adj;
    int* deg;
    int* cap;
} Graph;

mv_status_t graph_create(Graph* g, int n);
void graph_destroy(Graph* g);
mv_status_t graph_add_edge(Graph* g, int from, int to);
void graph_print_adj(const Graph* g);
mv_status_t graph_dfs_stepped(const Graph* g, int start, graph_step_cb cb, void* ud);

mv_status_t graph_bfs_stepped(const Graph* g, int start, graph_step_cb cb, void* ud);
void graph_print_step(const GraphStep* step, const VertColor* color, int n, void* ud);
mv_status_t graph_export_dot(const Graph* g, const VertColor* color, const char* filepath, const char* name);

#endif