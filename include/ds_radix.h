#ifndef MATHVISTA_DS_RADIX_H
#define MATHVISTA_DS_RADIX_H

#include <stddef.h>
#include <stdbool.h>
#include "math_engine.h"

typedef struct RadixNode
{
    char* edge;
    size_t edge_len;
    bool is_terminal;
    int value;
    struct RadixNode** children;
    size_t child_count;
} RadixNode;

typedef struct
{
    RadixNode* root;
    size_t size;
} RadixTree;

mv_status_t radix_create(RadixTree* rt);
void radix_destroy(RadixTree* rt);
mv_status_t radix_insert(RadixTree* rt, const char* key, int value);
bool radix_search(const RadixTree* rt, const char* key, int* value_out);
void radix_print(const RadixTree* rt);
mv_status_t radix_export_dot(const RadixTree* rt, const char* filepath);

#endif