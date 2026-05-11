#ifndef MATHVISTA_DS_SKIPLIST_H
#define MATHVISTA_DS_SKIPLIST_H

#include <stdbool.h>
#include <limits.h>
#include "math_engine.h"
#define SKIPLIST_MAX_LEVEL 16

typedef struct SkipListNode
{
    int key;
    int value;
    int level; // number of forward pointers
    struct SkipListNode** forward;
} SkipListNode;

typedef struct
{
    SkipListNode* header;
    int level;
    int size;
    unsigned int seed;
} SkipList;

mv_status_t sl_create(SkipList* sl);
void sl_destroy(SkipList* sl);
mv_status_t sl_insert(SkipList* sl, int key, int value);
bool sl_search(const SkipList* sl, int key, int* value_out);
mv_status_t sl_delete(SkipList* sl, int key);
void sl_print(const SkipList* sl);
mv_status_t sl_export_dot(const SkipList* sl, const char* filepath);

#endif