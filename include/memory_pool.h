#ifndef MATHVISTA_MEMORY_POOL_H
#define MATHVISTA_MEMORY_POOL_H

#include <stddef.h>
#include "math_engine.h"

typedef struct MemPoolFreeNode
{
    struct MemPoolFreeNode* next;
} MemPoolFreeNode;

typedef struct
{
    unsigned char* buffer;
    size_t block_size;
    size_t block_count;
    size_t used_count;
    MemPoolFreeNode* free_list;
} MemPool;

typedef struct
{
    size_t block_size;
    size_t total_blocks;
    size_t used_blocks;
    size_t free_blocks;
    double utilization;
} MemPoolStats;

mv_status_t mempool_create(MemPool* p, size_t block_size, size_t block_count);
void mempool_destroy(MemPool* p);
void* mempool_alloc(MemPool* p);
void mempool_free(MemPool* p, void* ptr);
void mempool_reset(MemPool* p);
mv_status_t mempool_stats(const MemPool* p, MemPoolStats* out);
void mempool_render_ascii(const MemPool* p);

#endif