#include "../../include/memory_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#define MP_ALIGN 16

static size_t align_up(size_t n, size_t a)
{
    return (n + a - 1) & ~(a - 1);
}

mv_status_t mempool_create(MemPool* p, size_t block_size, size_t block_count)
{
    if (!p) return MV_ERR_NULL_PTR;
    if (block_count == 0) return MV_ERR_OUT_OF_RANGE;
    size_t bs = block_size < sizeof(MemPoolFreeNode) ? sizeof(MemPoolFreeNode) : block_size;
    bs = align_up(bs, MP_ALIGN);

    p->buffer = aligned_alloc(MP_ALIGN, bs * block_count);
    if (!p->buffer) return MV_ERR_ALLOC;
    p->block_size = bs;
    p->block_count = block_count;
    p->used_count = 0;
    p->free_list = NULL;
    for (size_t i = 0; i < block_count; ++i)
    {
        MemPoolFreeNode* node = (MemPoolFreeNode *)(p->buffer + i * bs);
        node->next = p->free_list;
        p->free_list = node;
    }
    return MV_OK;
}

void mempool_destroy(MemPool* p)
{
    if (!p) return;
    free(p->buffer);
    p->buffer = NULL;
    p->free_list = NULL;
    p->block_size = p->block_count = p->used_count = 0;
}

void* mempool_alloc(MemPool* p)
{
    if (!p || !p->free_list) return NULL;
    MemPoolFreeNode* node = p->free_list;
    p->free_list = node->next;
    p->used_count++;
    return node;
}

void mempool_free(MemPool* p, void* ptr)
{
    if (!p || !ptr) return;

    unsigned char* b = (unsigned char *)ptr;
    if (b < p->buffer || b >= p->buffer + p->block_size * p->block_count) return;
    if (((size_t)(b - p->buffer)) % p->block_size != 0) return;
    MemPoolFreeNode* node = (MemPoolFreeNode *)ptr;
    node->next = p->free_list;
    p->free_list = node;
    p->used_count--;
}

void mempool_reset(MemPool* p)
{
    if (!p || !p->buffer) return;
    p->free_list = NULL;
    p->used_count = 0;
    for (size_t i = 0; i < p->block_count; ++i)
    {
        MemPoolFreeNode* node = (MemPoolFreeNode *)(p->buffer + i * p->block_size);
        node->next = p->free_list;
        p->free_list = node;
    }
}

mv_status_t mempool_stats(const MemPool* p, MemPoolStats* out)
{
    if (!p || !out) return MV_ERR_NULL_PTR;
    out->block_size = p->block_size;
    out->total_blocks = p->block_count;
    out->used_blocks = p->used_count;
    out->free_blocks = p->block_count - p->used_count;
    out->utilization = p->block_count > 0 ? (double)p->used_count / (double)p->block_count : 0.0;
    return MV_OK;
}

void mempool_render_ascii(const MemPool* p)
{
    if (!p || !p->buffer) return;

    char* map = malloc(p->block_count);
    if (!map) return;
    memset(map, '#', p->block_count);
    for (MemPoolFreeNode* n = p->free_list; n != NULL; n = n->next)
    {
        size_t idx = ((unsigned char *)n - p->buffer) / p->block_size;
        if (idx < p->block_count) map[idx] = '.';
    }

    MemPoolStats s;
    mempool_stats(p, &s);
    printf("MemPool @ %p  block=%zuB  total=%zu  used=%zu  free=%zu  util=%.1f%%\n", (void *)p->buffer, s.block_size, s.total_blocks, s.used_blocks, s.free_blocks, s.utilization * 100.0);
    const size_t cols = 64;
    for (size_t i = 0; i < p->block_count; ++i)
    {
        if (i % cols == 0) printf("  %06zu | ", i);
        putchar(map[i]);
        if ((i + 1) % cols == 0) putchar('\n');
    }
    if (p->block_count % cols != 0) putchar('\n');
    free(map);
}