#include "algo_sort.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    size_t lo;
    size_t hi;
} QSRange;
typedef struct
{
    QSRange* data;
    size_t top;
    size_t cap;
} QSStack;

static mv_status_t qss_push(QSStack* s, size_t lo, size_t hi)
{
    if (s->top == s->cap)
    {
        size_t nc = s->cap ? s->cap * 2 : 32;
        QSRange* nd = realloc(s->data, nc * sizeof(QSRange));
        if (!nd) return MV_ERR_ALLOC;
        s->data = nd;
        s->cap = nc;
    }
    s->data[s->top].lo = lo;
    s->data[s->top].hi = hi;
    s->top++;
    return MV_OK;
}
static bool qss_pop(QSStack* s, size_t* lo, size_t* hi)
{
    if (s->top == 0) return false;
    s->top--;
    *lo = s->data[s->top].lo;
    *hi = s->data[s->top].hi;
    return true;
}

static void do_swap(int* arr, size_t a, size_t b, qsort_step_cb cb, void* ud, size_t n, size_t lo, size_t hi)
{
    if (a == b) return;
    int tmp = arr[a]; arr[a] = arr[b]; arr[b] = tmp;
    if (!cb) return;
    QSortStep step;
    step.event = QSORT_SWAP;
    step.lo = lo;
    step.hi = hi;
    step.pivot_final = 0;
    step.swap_a = a;
    step.swap_b = b;
    cb(arr, n, &step, ud);
}
mv_status_t quicksort_stepped(int* arr, size_t n, qsort_step_cb cb, void* ud)
{
    if (!arr) return MV_ERR_NULL_PTR;
    if (n <= 1)
    {
        if (cb && n == 1)
        {
            QSortStep done;
            done.event = QSORT_COMPLETE;
            done.lo = done.hi = done.pivot_final = done.swap_a = done.swap_b = 0;
            cb(arr, n, &done, ud);
        }
        return MV_OK;
    }
    QSStack stack = {NULL, 0, 0};
    mv_status_t st = qss_push(&stack, 0, n - 1);
    if (st != MV_OK) return st;
    size_t lo, hi;
    while (qss_pop(&stack, &lo, &hi))
    {
        if (lo >= hi) continue;
        if (cb)
        {
            QSortStep step;
            step.event = QSORT_PARTITION_BEGIN;
            step.lo = lo;
            step.hi = hi;
            step.pivot_final = step.swap_a = step.swap_b = 0;
            cb(arr, n, &step, ud);
        }
        size_t boundary = lo;
        for (size_t j = lo; j < hi; ++j)
        {
            if (arr[j] <= arr[hi])
            {
                do_swap(arr, boundary, j, cb, ud, n, lo, hi);
                boundary++;
            }
        }
        do_swap(arr, boundary, hi, cb, ud, n, lo, hi);

        if (cb)
        {
            QSortStep step;
            step.event = QSORT_PARTITION_DONE;
            step.lo = lo;
            step.hi = hi;
            step.pivot_final = boundary;
            step.swap_a = step.swap_b = 0;
            cb(arr, n, &step, ud);
        }

        if (boundary > lo)
        {
            st = qss_push(&stack, lo, boundary - 1);
            if (st != MV_OK)
            {
                free(stack.data);
                return st;
            }
        }
        if (boundary < hi)
        {
            st = qss_push(&stack, boundary + 1, hi);
            if (st != MV_OK) { free(stack.data); return st; }
        }
    }
    if (cb)
    {
        QSortStep done;
        done.event = QSORT_COMPLETE;
        done.lo = 0;
        done.hi = n - 1;
        done.pivot_final = done.swap_a = done.swap_b = 0;
        cb(arr, n, &done, ud);
    }

    free(stack.data);
    return MV_OK;
}

void qsort_print_step(const int* arr, size_t n, const QSortStep* step, void* ud)
{
    (void)ud;
    if (!arr || !step) return;
    switch (step->event)
    {
        case QSORT_PARTITION_BEGIN:
            printf("PART   [%zu..%zu]  pivot_val=%d\n", step->lo, step->hi, arr[step->hi]);
            break;
        case QSORT_SWAP:
            printf("  SWAP [%zu]<->[%zu]\n", step->swap_a, step->swap_b);
            break;
        case QSORT_PARTITION_DONE:
        {
            printf("  DONE pivot=%d @%zu  arr: ",
                   arr[step->pivot_final], step->pivot_final);
            for (size_t i = 0; i < n; ++i)
            {
                if (i == step->pivot_final) printf("[%d]", arr[i]);
                else if (i >= step->lo && i <= step->hi) printf(" %d ", arr[i]);
                else printf("(%d)", arr[i]);
            }
            putchar('\n');
            break;
        }
        case QSORT_COMPLETE:
            printf("COMPLETE  sorted: ");
            for (size_t i = 0; i < n; ++i) printf("%d ", arr[i]);
            putchar('\n');
            break;
    }
}