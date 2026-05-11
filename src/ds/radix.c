#include "../../include/ds_radix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static RadixNode* rn_alloc(const char* edge, size_t edge_len)
{
    RadixNode* n = calloc(1, sizeof(RadixNode));
    if (!n) return NULL;
    if (edge_len > 0)
    {
        n->edge = malloc(edge_len + 1);
        if (!n->edge)
        {
            free(n); return NULL;
        }
        memcpy(n->edge, edge, edge_len);
        n->edge[edge_len] = '\0';
    }
    n->edge_len = edge_len;
    return n;
}
static void rn_destroy(RadixNode* n)
{
    if (!n) return;
    for (size_t i = 0; i < n->child_count; ++i) rn_destroy(n->children[i]);
    free(n->edge);
    free(n->children);
    free(n);
}
static mv_status_t rn_add_child(RadixNode* parent, RadixNode* child)
{
    RadixNode** nc = realloc(parent->children, (parent->child_count + 1) * sizeof(RadixNode *));
    if (!nc) return MV_ERR_ALLOC;
    parent->children = nc;
    parent->children[parent->child_count++] = child;
    return MV_OK;
}
static RadixNode* rn_find_child(const RadixNode* n, char c)
{
    for (size_t i = 0; i < n->child_count; ++i)
        if (n->children[i]->edge_len > 0 && n->children[i]->edge[0] == c)
            return n->children[i];
    return NULL;
}

static void rn_replace_child(RadixNode* parent, const RadixNode* old_child, RadixNode* new_child)
{
    for (size_t i = 0; i < parent->child_count; ++i)
    {
        if (parent->children[i] == old_child)
        {
            parent->children[i] = new_child;
            return;
        }
    }
}
static size_t lcp(const char* a, size_t la, const char* b, size_t lb)
{
    size_t i = 0, lim = la < lb ? la : lb;
    while (i < lim && a[i] == b[i]) ++i;
    return i;
}

mv_status_t radix_create(RadixTree* rt)
{
    if (!rt) return MV_ERR_NULL_PTR;
    rt->root = rn_alloc(NULL, 0);
    if (!rt->root) return MV_ERR_ALLOC;
    rt->size = 0;
    return MV_OK;
}
void radix_destroy(RadixTree* rt)
{
    if (!rt) return;
    rn_destroy(rt->root);
    rt->root = NULL;
    rt->size = 0;
}

mv_status_t radix_insert(RadixTree* rt, const char* key, int value)
{
    if (!rt || !key) return MV_ERR_NULL_PTR;
    size_t klen = strlen(key);
    RadixNode* cur = rt->root;
    const char* rem = key;
    size_t rem_len = klen;
    while (rem_len > 0)
    {
        RadixNode* child = rn_find_child(cur, rem[0]);
        if (!child)
        {
            // no matching child
            RadixNode* leaf = rn_alloc(rem, rem_len);
            if (!leaf) return MV_ERR_ALLOC;
            leaf->is_terminal = true;
            leaf->value = value;
            mv_status_t st = rn_add_child(cur, leaf);
            if (st != MV_OK) { rn_destroy(leaf); return st; }
            rt->size++;
            return MV_OK;
        }
        size_t common = lcp(rem, rem_len, child->edge, child->edge_len);
        if (common == child->edge_len)
        {
            // full edge match
            rem += common;
            rem_len -= common;
            if (rem_len == 0)
            {
                if (!child->is_terminal) rt->size++;
                child->is_terminal = true;
                child->value = value;
                return MV_OK;
            }
            cur = child;
            continue;
        }
        // partial match
        RadixNode* split = rn_alloc(child->edge, common);
        if (!split) return MV_ERR_ALLOC;
        size_t suffix_len = child->edge_len - common;
        char* suffix = malloc(suffix_len + 1);
        if (!suffix)
        {
            rn_destroy(split);
            return MV_ERR_ALLOC;
        }
        memcpy(suffix, child->edge + common, suffix_len + 1);

        RadixNode* leaf = NULL;
        if (rem_len > common)
        {
            leaf = rn_alloc(rem + common, rem_len - common);
            if (!leaf) { free(suffix); rn_destroy(split); return MV_ERR_ALLOC; }
            leaf->is_terminal = true;
            leaf->value = value;
        }
        free(child->edge);
        child->edge = suffix;
        child->edge_len = suffix_len;
        rn_add_child(split, child);
        if (leaf) rn_add_child(split, leaf);
        if (rem_len <= common)
        {
            split->is_terminal = true;
            split->value = value;
        }
        rn_replace_child(cur, child, split);
        rt->size++;
        return MV_OK;
    }

    if (!cur->is_terminal) rt->size++;
    cur->is_terminal = true;
    cur->value = value;
    return MV_OK;
}

bool radix_search(const RadixTree* rt, const char* key, int* value_out)
{
    if (!rt || !key) return false;
    size_t klen = strlen(key);
    const RadixNode* cur = rt->root;
    const char* rem = key;
    size_t rem_len = klen;
    while (rem_len > 0)
    {
        const RadixNode* child = rn_find_child(cur, rem[0]);
        if (!child) return false;
        size_t common = lcp(rem, rem_len, child->edge, child->edge_len);
        if (common < child->edge_len) return false; /* only partial edge match */
        rem += common;
        rem_len -= common;
        cur = child;
    }
    if (!cur->is_terminal) return false;
    if (value_out) *value_out = cur->value;
    return true;
}

static void rn_print(const RadixNode* n, int depth)
{
    for (int i = 0; i < depth * 2; ++i) putchar(' ');
    if (n->edge_len > 0)
        printf("[\"%s\"]%s\n", n->edge, n->is_terminal ? "  *" : "");
    else
        printf("[ROOT]%s\n", n->is_terminal ? "  *" : "");
    for (size_t i = 0; i < n->child_count; ++i) rn_print(n->children[i], depth + 1);
}
void radix_print(const RadixTree* rt)
{
    if (!rt) return;
    printf("RadixTree  size=%zu\n", rt->size);
    rn_print(rt->root, 0);
}

static void rn_emit_dot(FILE*fp, const RadixNode* n)
{
    const char* color = n->is_terminal ? "#aaffaa" : "#ddeeff";
    if (n->edge_len > 0)
    {
        char safe[512] = {0};
        size_t si = 0;
        for (size_t j = 0; j < n->edge_len && si + 2 < sizeof(safe); ++j)
        {
            if (n->edge[j] == '"' || n->edge[j] == '\\') safe[si++] = '\\';
            safe[si++] = n->edge[j];
        }
        fprintf(fp,
            "  rn_%p [label=\"{edge=\\\"%s\\\" | 0x%016" PRIxPTR "}\","
            " style=filled, fillcolor=\"%s\"];\n",
            (void *)n, safe, (uintptr_t)n, color);
    }
    else
    {
        fprintf(fp,
            "  rn_%p [label=\"{ROOT | 0x%016" PRIxPTR "}\","
            " style=filled, fillcolor=\"#ffeedd\"];\n",
            (void *)n, (uintptr_t)n);
    }
    for (size_t i = 0; i < n->child_count; ++i)
    {
        fprintf(fp, "  rn_%p -> rn_%p [color=\"#555555\"];\n",
                (void *)n, (void *)n->children[i]);
        rn_emit_dot(fp, n->children[i]);
    }
}
mv_status_t radix_export_dot(const RadixTree* rt, const char* filepath)
{
    if (!rt || !filepath) return MV_ERR_NULL_PTR;
    FILE *fp = fopen(filepath, "w");
    if (!fp) return MV_ERR_ALLOC;
    fprintf(fp, "digraph RadixTree {\n");
    fprintf(fp, "  rankdir=TB;\n");
    fprintf(fp, "  label=\"RadixTree  size=%lu\";\n", (unsigned long)rt->size);
    fprintf(fp, "  labelloc=t;\n");
    fprintf(fp, "  node [fontname=\"Menlo\", fontsize=10, shape=record];\n");
    fprintf(fp, "  edge [fontname=\"Menlo\", fontsize=9];\n\n");
    rn_emit_dot(fp, rt->root);
    fprintf(fp, "}\n");
    fclose(fp);
    return MV_OK;
}