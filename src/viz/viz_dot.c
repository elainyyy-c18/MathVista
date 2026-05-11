#include "../../include/viz_dot.h"
#include "../../include/memory_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define VSET_MIN_CAP 64
#define VSET_MAX_LOAD 0.6

typedef struct
{
    void** slots;
    size_t capacity;
    size_t count;
} VisitedSet;

static size_t vset_next_pow2(size_t n)
{
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}
static uint64_t vset_hash_ptr(const void* p)
{
    uint64_t x = (uint64_t)(uintptr_t)p;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}
static mv_status_t vset_init(VisitedSet* s, size_t hint)
{
    size_t cap = hint < VSET_MIN_CAP ? VSET_MIN_CAP : vset_next_pow2(hint);
    s->slots = calloc(cap, sizeof(void *));
    if (!s->slots) return MV_ERR_ALLOC;
    s->capacity = cap;
    s->count = 0;
    return MV_OK;
}
static void vset_free(VisitedSet* s)
{
    free(s->slots);
    s->slots = NULL;
    s->capacity = s->count = 0;
}
static mv_status_t vset_grow(VisitedSet* s)
{
    size_t new_cap = s->capacity * 2;
    void** ns = calloc(new_cap, sizeof(void *));
    if (!ns) return MV_ERR_ALLOC;
    size_t mask = new_cap - 1;
    for (size_t i = 0; i < s->capacity; ++i)
    {
        if (!s->slots[i]) continue;
        size_t idx = (size_t)(vset_hash_ptr(s->slots[i]) & mask);
        while (ns[idx]) idx = (idx + 1) & mask;
        ns[idx] = s->slots[i];
    }
    free(s->slots);
    s->slots = ns;
    s->capacity = new_cap;
    return MV_OK;
}

static bool vset_test_and_insert(VisitedSet* s, void* p)
{
    if (!p) return true;
    if ((double)(s->count + 1) > VSET_MAX_LOAD * (double)s->capacity)
    {
        if (vset_grow(s) != MV_OK) return true;
    }
    size_t mask = s->capacity - 1;
    size_t idx = (size_t)(vset_hash_ptr(p) & mask);
    for (size_t i = 0; i < s->capacity; ++i)
    {
        if (!s->slots[idx]) { s->slots[idx] = p; s->count++; return false; }
        if (s->slots[idx] == p) return true;
        idx = (idx + 1) & mask;
    }
    return true;
}

static void escape_dot_label(const char* in, char* out)
{
    while (*in)
    {
        switch (*in)
        {
            case '"':  *out++ = '\\'; *out++ = '"';  break;
            case '\\': *out++ = '\\'; *out++ = '\\'; break;
            case '<':  *out++ = '\\'; *out++ = '<';  break;
            case '>':  *out++ = '\\'; *out++ = '>';  break;
            case '{':  *out++ = '\\'; *out++ = '{';  break;
            case '}':  *out++ = '\\'; *out++ = '}';  break;
            case '|':  *out++ = '\\'; *out++ = '|';  break;
            default:   *out++ = *in; break;
        }
        ++in;
    }
    *out = '\0';
}

static void emit_node(FILE*fp, const DotNode* n)
{
    void* id = n->user_data ? n->user_data : (void *)n;
    const char* shape = n->shape ? n->shape : "record";
    const char* color = n->color ? n->color : "#eef5ff";

    size_t buf_sz = 256
        + (n->label ? strlen(n->label) * 4 : 0)
        + n->sublabel_count * 128;
    char* buf = malloc(buf_sz);
    if (!buf) return;
    char* p = buf;
    p += sprintf(p, "{");

    if (n->label)
    {
        char* esc = malloc(strlen(n->label) * 4 + 8);
        if (esc)
        {
            escape_dot_label(n->label, esc);
            p += sprintf(p, "%s", esc);
            free(esc);
        }
    }
    else p += sprintf(p, "(null)");
    p += sprintf(p, " | \\<addr\\> 0x%016" PRIxPTR, (uintptr_t)n->user_data);
    for (size_t i = 0; i < n->sublabel_count; ++i)
    {
        if (!n->sublabels[i]) continue;
        char* esc = malloc(strlen(n->sublabels[i]) * 4 + 8);
        if (esc)
        {
            escape_dot_label(n->sublabels[i], esc);
            p += sprintf(p, " | %s", esc);
            free(esc);
        }
    }
    p += sprintf(p, "}");

    fprintf(fp,
        "  \"n_%p\" [label=\"%s\", shape=%s, style=filled,"
        " fillcolor=\"%s\", fontname=\"Menlo\", fontsize=10];\n",
        id, buf, shape, color);
    free(buf);
}

static void emit_edges(FILE*fp, const DotNode* n)
{
    void* src_id = n->user_data ? n->user_data : (void *)n;
    for (size_t i = 0; i < n->edge_count; ++i)
    {
        const DotNode* c = n->edges[i];
        if (!c) continue;
        void* dst_id = c->user_data ? c->user_data : (void *)c;
        if (n->edge_labels && n->edge_labels[i])
        {
            fprintf(fp,
                "  \"n_%p\" -> \"n_%p\" [label=\"%s\","
                " fontname=\"Menlo\", fontsize=9, color=\"#555555\"];\n",
                src_id, dst_id, n->edge_labels[i]);
        }
        else
        {
            fprintf(fp,
                "  \"n_%p\" -> \"n_%p\" [color=\"#555555\"];\n",
                src_id, dst_id);
        }
    }
}

static void dfs(FILE*fp, DotNode* node, VisitedSet* seen)
{
    if (!node) return;
    void* key = node->user_data ? node->user_data : (void *)node;
    if (vset_test_and_insert(seen, key)) return;
    emit_node(fp, node);
    emit_edges(fp, node);
    for (size_t i = 0; i < node->edge_count; ++i) dfs(fp, node->edges[i], seen);
}

DotNode* dot_node_create(void* user_data, const char* label, const char* color, const char* shape)
{
    DotNode* n = calloc(1, sizeof(DotNode));
    if (!n) return NULL;
    n->user_data = user_data;
    n->color = color;
    n->shape = shape;
    if (label)
    {
        char* copy = malloc(strlen(label) + 1);
        if (!copy)
        {
            free(n);
            return NULL;
        }
        strcpy(copy, label);
        n->label = copy;
    }
    return n;
}
void dot_node_destroy(DotNode* n)
{
    if (!n) return;
    free((void *)n->label);
    free(n->edges);
    free(n->edge_labels);
    free(n->sublabels);
    free(n);
}

mv_status_t dot_node_add_edge(DotNode* src, DotNode* dst, const char* edge_label)
{
    if (!src || !dst) return MV_ERR_NULL_PTR;
    size_t new_count = src->edge_count + 1;
    DotNode** ne = realloc(src->edges, new_count * sizeof(DotNode *));
    if (!ne) return MV_ERR_ALLOC;
    src->edges = ne;
    const char** nl = realloc(src->edge_labels, new_count * sizeof(char *));
    if (!nl) return MV_ERR_ALLOC;
    src->edge_labels = nl;
    src->edges[src->edge_count] = dst;
    src->edge_labels[src->edge_count] = edge_label;
    src->edge_count = new_count;
    return MV_OK;
}
mv_status_t dot_node_add_sublabel(DotNode* n, const char* text)
{
    if (!n || !text) return MV_ERR_NULL_PTR;
    size_t new_count = n->sublabel_count + 1;
    const char** ns = realloc(n->sublabels, new_count * sizeof(char *));
    if (!ns) return MV_ERR_ALLOC;
    n->sublabels = ns;
    n->sublabels[n->sublabel_count] = text;
    n->sublabel_count = new_count;
    return MV_OK;
}

mv_status_t dot_export(const char* filepath, const DotGraph* graph)
{
    if (!filepath || !graph) return MV_ERR_NULL_PTR;
    if (graph->root_count == 0 || !graph->roots) return MV_ERR_NULL_PTR;
    FILE*fp = fopen(filepath, "w");
    if (!fp) return MV_ERR_ALLOC;

    const char* name = graph->name ? graph->name : "G";
    const char* rankdir = graph->rankdir ? graph->rankdir : "TB";
    fprintf(fp, "digraph %s {\n", name);
    fprintf(fp, "  rankdir=%s;\n", rankdir);
    fprintf(fp, "  node [fontname=\"Menlo\", fontsize=10];\n");
    fprintf(fp, "  edge [fontname=\"Menlo\", fontsize=9];\n\n");

    VisitedSet seen;
    mv_status_t st = vset_init(&seen, graph->root_count * 8);
    if (st != MV_OK) { fclose(fp); return st; }

    for (size_t i = 0; i < graph->root_count; ++i) dfs(fp, graph->roots[i], &seen);
    vset_free(&seen);
    fprintf(fp, "}\n");
    fclose(fp);
    return MV_OK;
}

mv_status_t dot_export_memo_table(const char* filepath, const MemoTable* t, const char* graph_name)
{
    if (!filepath || !t) return MV_ERR_NULL_PTR;
    FILE*fp = fopen(filepath, "w");
    if (!fp) return MV_ERR_ALLOC;
    const char* name = graph_name ? graph_name : "MemoTable";

    fprintf(fp, "digraph %s {\n", name);
    fprintf(fp, "  rankdir=LR;\n");
    fprintf(fp, "  label=\"MemoTable  cap=%zu  count=%zu  load=%.2f\";\n", t->capacity, t->count, t->capacity ? (double)t->count / (double)t->capacity : 0.0);
    fprintf(fp, "  labelloc=t;\n");
    fprintf(fp, "  node [fontname=\"Menlo\", fontsize=10, shape=record];\n");
    fprintf(fp, "  edge [fontname=\"Menlo\", fontsize=8];\n\n");

    for (size_t i = 0; i < t->capacity; ++i)
    {
        const MemoEntry* e = &t->entries[i];
        if (e->state == 0) continue;
        if (e->state == 1)
        {
            fprintf(fp,
                "  slot_%lu [label=\"{slot %lu | key=(%" PRId64 ",%" PRId64 ")"
                " | val=%" PRIu64 " | \\<ptr\\> 0x%016" PRIxPTR "}\","
                " style=filled, fillcolor=\"#ddeeff\"];\n",
                (unsigned long)i, (unsigned long)i,
                e->key_a, e->key_b,
                e->value,
                (uintptr_t)e);
        }
        else // tombstone
        {
            fprintf(fp,
                "  slot_%lu [label=\"{slot %lu | TOMBSTONE}\","
                " style=filled, fillcolor=\"#cccccc\"];\n",
                (unsigned long)i, (unsigned long)i);
        }
    }
    fprintf(fp, "\n");
    size_t mask = t->capacity - 1;
    for (size_t i = 0; i < t->capacity; ++i)
    {
        const MemoEntry* e = &t->entries[i];
        if (e->state != 1) continue;

        uint64_t x = (uint64_t)e->key_a * 0x9E3779B97F4A7C15ULL;
        x ^= (uint64_t)e->key_b + 0xBF58476D1CE4E5B9ULL + (x << 6) + (x >> 2);
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
        x ^= (x >> 31);
        size_t ideal = (size_t)(x & mask);

        if (ideal != i)
        {
            if (t->entries[ideal].state != 0)
            {
                fprintf(fp,
                    "  slot_%lu -> slot_%lu [style=dashed,"
                    " color=\"#aa4444\", label=\"probe\"];\n",
                    (unsigned long)ideal, (unsigned long)i);
            }
        }
    }
    fprintf(fp, "}\n");
    fclose(fp);
    return MV_OK;
}

mv_status_t dot_export_mem_pool(const char* filepath, const void* pool_ptr, const char* graph_name)
{
    if (!filepath || !pool_ptr) return MV_ERR_NULL_PTR;
    const MemPool* p = (const MemPool *)pool_ptr;
    FILE*fp = fopen(filepath, "w");
    if (!fp) return MV_ERR_ALLOC;
    const char* name = graph_name ? graph_name : "MemPool";

    fprintf(fp, "digraph %s {\n", name);
    fprintf(fp, "  rankdir=LR;\n");
    fprintf(fp, "  label=\"MemPool @ 0x%016" PRIxPTR
            "  block=%luB  total=%lu  used=%lu\";\n",
            (uintptr_t)p->buffer,
            (unsigned long)p->block_size,
            (unsigned long)p->block_count,
            (unsigned long)p->used_count);
    fprintf(fp, "  labelloc=t;\n");
    fprintf(fp, "  node [fontname=\"Menlo\", fontsize=9, shape=record,"
            " width=1.0, height=0.35, fixedsize=true];\n");
    fprintf(fp, "  edge [fontname=\"Menlo\", fontsize=8];\n\n");

    char* is_free = calloc(p->block_count, 1);
    if (!is_free) { fclose(fp); return MV_ERR_ALLOC; }
    for (MemPoolFreeNode* fn = p->free_list; fn != NULL; fn = fn->next)
    {
        size_t idx = ((unsigned char *)fn - p->buffer) / p->block_size;
        if (idx < p->block_count) is_free[idx] = 1;
    }

    for (size_t i = 0; i < p->block_count; ++i)
    {
        uintptr_t addr = (uintptr_t)(p->buffer + i * p->block_size);
        const char* color = is_free[i] ? "#aaddbb" : "#ddaaaa";
        const char* status = is_free[i] ? "FREE" : "USED";
        fprintf(fp,
            "  blk_%lu [label=\"{%lu | %s | 0x%016" PRIxPTR "}\","
            " style=filled, fillcolor=\"%s\"];\n",
            (unsigned long)i, (unsigned long)i, status, addr, color);
    }
    fprintf(fp, "\n");
    for (size_t i = 0; i + 1 < p->block_count; ++i)
    {
        fprintf(fp, "  blk_%lu -> blk_%lu [style=invis];\n", (unsigned long)i, (unsigned long)(i + 1));
    }

    MemPoolFreeNode* cur = p->free_list;
    while (cur && cur->next)
    {
        size_t cur_idx = ((unsigned char *)cur - p->buffer) / p->block_size;
        size_t nxt_idx = ((unsigned char *)cur->next - p->buffer) / p->block_size;
        if (cur_idx < p->block_count && nxt_idx < p->block_count)
        {
            fprintf(fp,
                "  blk_%lu -> blk_%lu [color=\"#2255aa\","
                " style=dashed, constraint=false,"
                " label=\"free_list\"];\n",
                (unsigned long)cur_idx, (unsigned long)nxt_idx);
        }
        cur = cur->next;
    }
    free(is_free);
    fprintf(fp, "}\n");
    fclose(fp);
    return MV_OK;
}