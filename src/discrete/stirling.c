#include "discrete.h"
#include "math_engine.h"
#include <stdlib.h>
#include <string.h>

static mv_status_t stirling2_rec(int n, int k, MemoTable* memo, uint64_t* out)
{
    if (n < 0 || k < 0 || k > n)
    {
        *out = 0;
        return MV_OK;
    }
    if (n == 0 && k == 0)
    {
        *out = 1;
        return MV_OK;
    }
    if (k == 0)
    {
        *out = 0;
        return MV_OK;
    }
    if (k == n || k == 1)
    {
        *out = 1;
        return MV_OK;
    }

    uint64_t cached;
    if (memo_get(memo, n, k, &cached))
    {
        *out = cached;
        return MV_OK;
    }

    uint64_t left, right;
    mv_status_t st = stirling2_rec(n - 1, k, memo, &left);
    if (st != MV_OK) return st;
    st = stirling2_rec(n - 1, k - 1, memo, &right);
    if (st != MV_OK) return st;

    uint64_t scaled, result;
    if (__builtin_mul_overflow((uint64_t)k, left, &scaled)) return MV_ERR_OVERFLOW;
    if (__builtin_add_overflow(scaled, right, &result)) return MV_ERR_OVERFLOW;
    st = memo_set(memo, n, k, result);
    if (st != MV_OK) return st;

    *out = result;
    return MV_OK;
}

mv_status_t stirling2_memoized(int n, int k, MemoTable* memo, uint64_t* out)
{
    if (!memo || !out) return MV_ERR_NULL_PTR;
    return stirling2_rec(n, k, memo, out);
}

mv_status_t stirling2_iterative(int n, int k, uint64_t* out)
{
    if (!out) return MV_ERR_NULL_PTR;
    if (n < 0 || k < 0 || k > n)
    {
        *out = 0;
        return MV_OK;
    }
    if (n == 0 && k == 0)
    {
        *out = 1;
        return MV_OK;
    }
    if (k == 0)
    {
        *out = 0;
        return MV_OK;
    }
    uint64_t* prev = calloc((size_t)k + 1, sizeof(uint64_t));
    uint64_t* curr = calloc((size_t)k + 1, sizeof(uint64_t));
    if (!prev || !curr)
    {
        free(prev);
        free(curr);
        return MV_ERR_ALLOC;
    }
    prev[0] = 1;
    for (int i = 1; i <= n; ++i)
    {
        curr[0] = 0;
        int jmax = (k < i) ? k : i;
        for (int j = 1; j <= jmax; ++j)
        {
            uint64_t scaled, result;
            if (__builtin_mul_overflow((uint64_t)j, prev[j], &scaled))
            {
                free(prev); free(curr);
                return MV_ERR_OVERFLOW;
            }
            if (__builtin_add_overflow(scaled, prev[j - 1], &result))
            {
                free(prev); free(curr);
                return MV_ERR_OVERFLOW;
            }
            curr[j] = result;
        }
        // swap
        uint64_t* tmp = prev;
        prev = curr;
        curr = tmp;
        memset(curr, 0, ((size_t)k + 1) * sizeof(uint64_t));
    }
    *out = prev[k];
    free(prev);
    free(curr);
    return MV_OK;
}