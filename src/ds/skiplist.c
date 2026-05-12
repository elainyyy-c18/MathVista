#include "ds_skiplist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

static int rand_level(unsigned int* seed)
{
    int lvl = 1;
    while (lvl < SKIPLIST_MAX_LEVEL)
    {
        *seed = *seed * 1664525u + 1013904223u;
        if ((*seed >> 16) & 1u) lvl++;
        else break;
    }
    return lvl;
}

static SkipListNode* node_alloc(int key, int value, int level)
{
    SkipListNode* n = malloc(sizeof(SkipListNode));
    if (!n) return NULL;
    n->forward = calloc((size_t)level, sizeof(SkipListNode *));
    if (!n->forward)
    {
        free(n);
        return NULL;
    }
    n->key = key;
    n->value = value;
    n->level = level;
    return n;
}

static void node_free(SkipListNode* n)
{
    if (!n) return;
    free(n->forward);
    free(n);
}

mv_status_t sl_create(SkipList* sl)
{
    if (!sl) return MV_ERR_NULL_PTR;
    sl->header = node_alloc(INT_MIN, 0, SKIPLIST_MAX_LEVEL);
    if (!sl->header) return MV_ERR_ALLOC;
    sl->level = 1;
    sl->size = 0;
    sl->seed = 0xdeadbeef;
    return MV_OK;
}

void sl_destroy(SkipList* sl)
{
    if (!sl) return;
    SkipListNode* cur = sl->header;
    while (cur)
    {
        SkipListNode* nxt = cur->forward[0];
        node_free(cur);
        cur = nxt;
    }
    sl->header = NULL;
    sl->level = sl->size = 0;
}

mv_status_t sl_insert(SkipList* sl, int key, int value)
{
    if (!sl) return MV_ERR_NULL_PTR;
    SkipListNode* update[SKIPLIST_MAX_LEVEL];
    SkipListNode* cur = sl->header;
    for (int i = sl->level - 1; i >= 0; --i)
    {
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
        update[i] = cur;
    }

    SkipListNode* fwd = cur->forward[0];
    if (fwd && fwd->key == key)
    {
        fwd->value = value;
        return MV_OK;
    }
    int nlvl = rand_level(&sl->seed);
    if (nlvl > sl->level)
    {
        for (int i = sl->level; i < nlvl; ++i) update[i] = sl->header;
        sl->level = nlvl;
    }
    SkipListNode* n = node_alloc(key, value, nlvl);
    if (!n) return MV_ERR_ALLOC;
    for (int i = 0; i < nlvl; ++i)
    {
        n->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = n;
    }
    sl->size++;
    return MV_OK;
}
bool sl_search(const SkipList* sl, int key, int* value_out)
{
    if (!sl) return false;
    const SkipListNode* cur = sl->header;
    for (int i = sl->level - 1; i >= 0; --i)
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
    cur = cur->forward[0];
    if (cur && cur->key == key)
    {
        if (value_out) *value_out = cur->value;
        return true;
    }
    return false;
}
mv_status_t sl_delete(SkipList* sl, int key)
{
    if (!sl) return MV_ERR_NULL_PTR;
    SkipListNode* update[SKIPLIST_MAX_LEVEL];
    SkipListNode* cur = sl->header;
    for (int i = sl->level - 1; i >= 0; --i)
    {
        while (cur->forward[i] && cur->forward[i]->key < key)
            cur = cur->forward[i];
        update[i] = cur;
    }
    SkipListNode* t = cur->forward[0];
    if (!t || t->key != key) return MV_ERR_NOT_FOUND;
    for (int i = 0; i < sl->level; ++i)
    {
        if (update[i]->forward[i] != t) break;
        update[i]->forward[i] = t->forward[i];
    }
    while (sl->level > 1 && !sl->header->forward[sl->level - 1]) sl->level--;
    node_free(t);
    sl->size--;
    return MV_OK;
}

void sl_print(const SkipList* sl)
{
    if (!sl) return;
    printf("SkipList  size=%d  active_levels=%d\n", sl->size, sl->level);
    for (int i = sl->level - 1; i >= 0; --i)
    {
        printf("  L%2d  HEAD", i);
        const SkipListNode* cur = sl->header->forward[i];
        while (cur)
        {
            printf(" -> [%d:%d]", cur->key, cur->value);
            cur = cur->forward[i];
        }
        printf(" -> NIL\n");
    }
}
mv_status_t sl_export_dot(const SkipList* sl, const char* filepath)
{
    if (!sl || !filepath) return MV_ERR_NULL_PTR;
    FILE*fp = fopen(filepath, "w");
    if (!fp) return MV_ERR_ALLOC;
    static const char *lcolors[] = 
    {
        "#222222", "#1144cc", "#11aa33", "#cc4411",
        "#aa11aa", "#11aaaa", "#887700", "#007788"
    };
    int nc = (int)(sizeof(lcolors) / sizeof(lcolors[0]));
    fprintf(fp, "digraph SkipList {\n");
    fprintf(fp, "  rankdir=LR;\n");
    fprintf(fp, "  label=\"SkipList  size=%d  levels=%d\";\n", sl->size, sl->level);
    fprintf(fp, "  labelloc=t;\n");
    fprintf(fp, "  node [fontname=\"Menlo\", fontsize=10, shape=record, style=filled];\n");
    fprintf(fp, "  edge [fontname=\"Menlo\", fontsize=8];\n\n");
    fprintf(fp, "  NIL [label=\"NIL\", fillcolor=\"#cccccc\"];\n\n");

    for (const SkipListNode* n = sl->header; n; n = n->forward[0])
    {
        if (n == sl->header)
        {
            fprintf(fp,
                "  nd_%p [label=\"{HEAD | lvl=%d | 0x%016" PRIxPTR "}\","
                " fillcolor=\"#ffeedd\"];\n",
                (void *)n, n->level, (uintptr_t)n);
        }
        else
        {
            fprintf(fp,
                "  nd_%p [label=\"{k=%d | v=%d | lvl=%d | 0x%016" PRIxPTR "}\","
                " fillcolor=\"#ddeeff\"];\n",
                (void *)n, n->key, n->value, n->level, (uintptr_t)n);
        }
    }
    fprintf(fp, "\n");
    for (const SkipListNode* n = sl->header; n; n = n->forward[0])
    {
        for (int lvl = 0; lvl < n->level; ++lvl)
        {
            const char* col = lcolors[lvl % nc];
            const char* pw = lvl == 0 ? "penwidth=2.0," : "";
            if (n->forward[lvl])
            {
                fprintf(fp,
                    "  nd_%p -> nd_%p [%slabel=\"L%d\", color=\"%s\"];\n",
                    (void *)n, (void *)n->forward[lvl], pw, lvl, col);
            }
            else
            {
                fprintf(fp,
                    "  nd_%p -> NIL [%slabel=\"L%d\", color=\"%s\", style=dashed];\n",
                    (void *)n, pw, lvl, col);
            }
        }
    }
    fprintf(fp, "}\n");
    fclose(fp);
    return MV_OK;
}