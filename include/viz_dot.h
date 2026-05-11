#ifndef MATHVISTA_VIZ_DOT_H
#define MATHVISTA_VIZ_DOT_H

#include <stddef.h>
#include <stdint.h>
#include "../include/math_engine.h"

typedef struct DotNode
{
    void* user_data;
    const char* label;
    const char** sublabels;
    size_t sublabel_count;
    struct DotNode** edges;
    const char** edge_labels;
    size_t edge_count;
    const char* color;
    const char* shape;
} DotNode;

typedef struct
{
    DotNode** roots;
    size_t root_count;
    const char* name;
    const char* rankdir;
} DotGraph;

DotNode* dot_node_create(void* user_data, const char* label, const char* color, const char* shape);
void dot_node_destroy(DotNode* n);

mv_status_t dot_node_add_edge(DotNode* src, DotNode* dst, const char* edge_label);
mv_status_t dot_node_add_sublabel(DotNode* n, const char* text);
mv_status_t dot_export(const char* filepath, const DotGraph* graph);
mv_status_t dot_export_memo_table(const char* filepath, const MemoTable* t, const char* graph_name);
mv_status_t dot_export_mem_pool(const char* filepath, const void* pool_ptr, const char* graph_name);

#endif