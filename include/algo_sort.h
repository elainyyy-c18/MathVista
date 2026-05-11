#ifndef MATHVISTA_ALGO_SORT_H
#define MATHVISTA_ALGO_SORT_H
#include <stddef.h>
#include "math_engine.h"

typedef enum
{
    QSORT_PARTITION_BEGIN,
    QSORT_SWAP,
    QSORT_PARTITION_DONE,
    QSORT_COMPLETE
} QSortEvent;

typedef struct
{
    QSortEvent event;
    size_t lo;
    size_t hi;
    size_t pivot_final;
    size_t swap_a;
    size_t swap_b;
} QSortStep;

typedef void (*qsort_step_cb)(const int* arr, size_t n, const QSortStep* step, void* ud);
mv_status_t quicksort_stepped(int* arr, size_t n, qsort_step_cb cb, void* ud);
void qsort_print_step(const int* arr, size_t n, const QSortStep* step, void* ud);

#endif