#include "../../include/discrete.h"
#include "../../include/math_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mv_status_t catalan_rec(int n, MemoTable* memo, uint64_t* out)
{
    if (n < 0) return MV_ERR_OUT_OF_RANGE;
    if (n == 0)
    {
        *out = 1;
        return MV_OK;
    }
    uint64_t cached;
    if (memo_get(memo, n, 0, &cached))
    {
        *out = cached;
        return MV_OK;
    }
    uint64_t sum = 0;
    for (int i = 0; i < n; ++i)
    {
        uint64_t a, b, prod;
        mv_status_t st = catalan_rec(i, memo, &a);
        if (st != MV_OK) return st;
        st = catalan_rec(n - 1 - i, memo, &b);
        if (st != MV_OK) return st;
        if (__builtin_mul_overflow(a, b, &prod)) return MV_ERR_OVERFLOW;
        if (__builtin_add_overflow(sum, prod, &sum)) return MV_ERR_OVERFLOW;
    }
    mv_status_t st = memo_set(memo, n, 0, sum);
    if (st != MV_OK) return st;

    *out = sum;
    return MV_OK;
}

mv_status_t catalan_memoized(int n, MemoTable* memo, uint64_t* out)
{
    if (!memo || !out) return MV_ERR_NULL_PTR;
    return catalan_rec(n, memo, out);
}
mv_status_t catalan_iterative(int n, uint64_t* out)
{
    if (!out) return MV_ERR_NULL_PTR;
    if (n < 0) return MV_ERR_OUT_OF_RANGE;
    uint64_t* c = calloc((size_t)n + 1, sizeof(uint64_t));
    if (!c) return MV_ERR_ALLOC;
    c[0] = 1;
    for (int i = 1; i <= n; ++i)
    {
        uint64_t sum = 0;
        for (int j = 0; j < i; ++j)
        {
            uint64_t prod;
            if (__builtin_mul_overflow(c[j], c[i - 1 - j], &prod))
            {
                free(c);
                return MV_ERR_OVERFLOW;
            }
            if (__builtin_add_overflow(sum, prod, &sum))
            {
                free(c);
                return MV_ERR_OVERFLOW;
            }
        }
        c[i] = sum;
    }
    *out = c[n];
    free(c);
    return MV_OK;
}

static bool dyck_gen(int n, int which, int* count, char* path, int up, int down, int len)
{
    if (len == 2 * n)
    {
        if (*count == which) return true;
        (*count)++;
        return false;
    }

    if (up < n)
    {
        path[len] = 'U';
        if (dyck_gen(n, which, count, path, up + 1, down, len + 1)) return true;
    }
    if (down < up)
    {
        path[len] = 'D';
        if (dyck_gen(n, which, count, path, up, down + 1, len + 1)) return true;
    }
    return false;
}

mv_status_t catalan_render_dyck(int n, int which)
{
    if (n <= 0 || which < 0) return MV_ERR_OUT_OF_RANGE;

    char* path = calloc((size_t)(2 * n + 1), 1);
    if (!path) return MV_ERR_ALLOC;

    int count = 0;
    if (!dyck_gen(n, which, &count, path, 0, 0, 0))
    {
        free(path);
        return MV_ERR_OUT_OF_RANGE; /* `which` exceeds C(n) - 1 */
    }
    int rows = n + 1;
    int cols = 2 * n;
    char** canvas = malloc((size_t)rows * sizeof(char *));
    if (!canvas)
    {
        free(path);
        return MV_ERR_ALLOC;
    }
    for (int i = 0; i < rows; ++i)
    {
        canvas[i] = malloc((size_t)cols + 1);
        if (!canvas[i])
        {
            for (int j = 0; j < i; ++j) free(canvas[j]);
            free(canvas);
            free(path);
            return MV_ERR_ALLOC;
        }
        memset(canvas[i], ' ', (size_t)cols);
        canvas[i][cols] = '\0';
    }
    for (int x = 0; x < cols; ++x) canvas[n][x] = '_';
    int x = 0, y = 0;
    for (int i = 0; i < 2 * n; ++i)
    {
        if (path[i] == 'U')
        {
            canvas[n - y - 1][x] = '/';
            y++;
        }
        else
        {
            canvas[n - y][x] = '\\';
            y--;
        }
        x++;
    }
    printf("Dyck path #%d of order %d  encoding: %s\n", which, n, path);
    for (int i = 0; i < rows; ++i)
    {
        printf("  %s\n", canvas[i]);
        free(canvas[i]);
    }
    free(canvas);
    free(path);
    return MV_OK;
}