#include "math_engine.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

const char* mv_strerror(mv_status_t s)
{
    switch (s)
    {
        case MV_OK: return "OK";
        case MV_ERR_ALLOC: return "Allocation failed";
        case MV_ERR_DIM_MISMATCH: return "Dimension mismatch";
        case MV_ERR_OUT_OF_RANGE: return "Out of range";
        case MV_ERR_SINGULAR: return "Singular";
        case MV_ERR_OVERFLOW: return "Numeric overflow";
        case MV_ERR_NULL_PTR: return "Null pointer";
        case MV_ERR_NOT_FOUND: return "Not found";
        case MV_ERR_CONVERGE: return "Failed to converge";
        default: return "Unknown error";
    }
}

mv_status_t mvec_create(MVec* out, size_t dim)
{
    if (!out) return MV_ERR_NULL_PTR;
    if (dim == 0) return MV_ERR_OUT_OF_RANGE;
    out->data = calloc(dim, sizeof(double));
    if (!out->data) return MV_ERR_ALLOC;
    out->dim = dim;
    return MV_OK;
}
mv_status_t mvec_create_from(MVec* out, const double* src, size_t dim)
{
    if (!src) return MV_ERR_NULL_PTR;
    mv_status_t st = mvec_create(out, dim);
    if (st != MV_OK) return st;
    memcpy(out->data, src, dim * sizeof(double));
    return MV_OK;
}
void mvec_destroy(MVec* v)
{
    if (!v) return;
    free(v->data);
    v->data = NULL;
    v->dim = 0;
}
mv_status_t mvec_copy(MVec* dst, const MVec* src)
{
    if (!dst || !src) return MV_ERR_NULL_PTR;
    if (dst->dim != src->dim) return MV_ERR_DIM_MISMATCH;
    memcpy(dst->data, src->data, src->dim * sizeof(double));
    return MV_OK;
}
mv_status_t mvec_add(MVec *out, const MVec *a, const MVec *b)
{
    if (!out || !a || !b) return MV_ERR_NULL_PTR;
    if (a->dim != b->dim || out->dim != a->dim) return MV_ERR_DIM_MISMATCH;
    for (size_t i = 0; i < a->dim; ++i) out->data[i] = a->data[i] + b->data[i];
    return MV_OK;
}

mv_status_t mvec_sub(MVec *out, const MVec *a, const MVec *b)
{
    if (!out || !a || !b) return MV_ERR_NULL_PTR;
    if (a->dim != b->dim || out->dim != a->dim) return MV_ERR_DIM_MISMATCH;
    for (size_t i = 0; i < a->dim; ++i) out->data[i] = a->data[i] - b->data[i];
    return MV_OK;
}

mv_status_t mvec_scale(MVec *v, double s)
{
    if (!v) return MV_ERR_NULL_PTR;
    for (size_t i = 0; i < v->dim; ++i) v->data[i] *= s;
    return MV_OK;
}

mv_status_t mvec_dot(double *out, const MVec *a, const MVec *b)
{
    if (!out || !a || !b) return MV_ERR_NULL_PTR;
    if (a->dim != b->dim) return MV_ERR_DIM_MISMATCH;
    double s = 0.0;
    for (size_t i = 0; i < a->dim; ++i) s += a->data[i] * b->data[i];
    *out = s;
    return MV_OK;
}

mv_status_t mvec_norm(double *out, const MVec *v)
{
    double dp;
    mv_status_t st = mvec_dot(&dp, v, v);
    if (st != MV_OK) return st;
    *out = sqrt(dp);
    return MV_OK;
}

mv_status_t mvec_normalize(MVec *v)
{
    double n;
    mv_status_t st = mvec_norm(&n, v);
    if (st != MV_OK) return st;
    if (n == 0.0) return MV_ERR_SINGULAR;
    return mvec_scale(v, 1.0 / n);
}

/* ---------------- Matrix ---------------- */

mv_status_t mmat_create(MMat *out, size_t rows, size_t cols)
{
    if (!out) return MV_ERR_NULL_PTR;
    if (rows == 0 || cols == 0) return MV_ERR_OUT_OF_RANGE;
    out->data = calloc(rows * cols, sizeof(double));
    if (!out->data) return MV_ERR_ALLOC;
    out->rows = rows;
    out->cols = cols;
    return MV_OK;
}

mv_status_t mmat_identity(MMat *out, size_t n)
{
    mv_status_t st = mmat_create(out, n, n);
    if (st != MV_OK) return st;
    for (size_t i = 0; i < n; ++i) out->data[i * n + i] = 1.0;
    return MV_OK;
}

void mmat_destroy(MMat *m)
{
    if (!m) return;
    free(m->data);
    m->data = NULL;
    m->rows = m->cols = 0;
}

mv_status_t mmat_mul(MMat *out, const MMat *A, const MMat *B)
{
    if (!out || !A || !B) return MV_ERR_NULL_PTR;
    if (A->cols != B->rows) return MV_ERR_DIM_MISMATCH;
    if (out->rows != A->rows || out->cols != B->cols) return MV_ERR_DIM_MISMATCH;

    memset(out->data, 0, out->rows * out->cols * sizeof(double));

    /* ikj loop ordering: better cache locality than ijk because B is
     * scanned row-by-row in the innermost loop. */
    for (size_t i = 0; i < A->rows; ++i)
    {
        for (size_t k = 0; k < A->cols; ++k)
        {
            double a = A->data[i * A->cols + k];
            for (size_t j = 0; j < B->cols; ++j)
            {
                out->data[i * out->cols + j] += a * B->data[k * B->cols + j];
            }
        }
    }
    return MV_OK;
}

mv_status_t mmat_vec_mul(MVec *out, const MMat *A, const MVec *x)
{
    if (!out || !A || !x) return MV_ERR_NULL_PTR;
    if (A->cols != x->dim || out->dim != A->rows) return MV_ERR_DIM_MISMATCH;
    for (size_t i = 0; i < A->rows; ++i)
    {
        double s = 0.0;
        for (size_t j = 0; j < A->cols; ++j) s += A->data[i * A->cols + j] * x->data[j];
        out->data[i] = s;
    }
    return MV_OK;
}

mv_status_t mmat_transpose(MMat *out, const MMat *A)
{
    if (!out || !A) return MV_ERR_NULL_PTR;
    if (out->rows != A->cols || out->cols != A->rows) return MV_ERR_DIM_MISMATCH;
    for (size_t i = 0; i < A->rows; ++i)
        for (size_t j = 0; j < A->cols; ++j)
            out->data[j * A->rows + i] = A->data[i * A->cols + j];
    return MV_OK;
}

/* ---------------- MemoTable: open addressing ---------------- */

#define MEMO_MIN_CAP 16
#define MEMO_MAX_LOAD 0.7

static size_t next_pow2(size_t n)
{
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* SplitMix64-style mixer combining two int64 keys. Distributes well
 * for the (n, k)-style keys that dominate this codebase. */
static uint64_t hash_pair(int64_t a, int64_t b)
{
    uint64_t x = (uint64_t)a * 0x9E3779B97F4A7C15ULL;
    x ^= (uint64_t)b + 0xBF58476D1CE4E5B9ULL + (x << 6) + (x >> 2);
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

mv_status_t memo_create(MemoTable *out, size_t hint)
{
    if (!out) return MV_ERR_NULL_PTR;
    size_t cap = hint < MEMO_MIN_CAP ? MEMO_MIN_CAP : next_pow2(hint);
    out->entries = calloc(cap, sizeof(MemoEntry));
    if (!out->entries) return MV_ERR_ALLOC;
    out->capacity = cap;
    out->count = 0;
    out->tombstones = 0;
    return MV_OK;
}

void memo_destroy(MemoTable *t)
{
    if (!t) return;
    free(t->entries);
    t->entries = NULL;
    t->capacity = t->count = t->tombstones = 0;
}

/* Probe for (a, b). Returns:
 *   *found = true,  index of the occupied matching slot
 *   *found = false, index of the first slot suitable for insertion
 *                   (preferring the earliest tombstone seen, else the
 *                    first empty slot encountered).
 *
 * Because we use power-of-2 capacity with load factor < 1, the loop
 * must terminate at an empty slot in at most `capacity` iterations. */
static size_t memo_probe(const MemoEntry *entries, size_t cap,
                         int64_t a, int64_t b, bool *found)
{
    size_t mask = cap - 1;
    size_t idx = (size_t)(hash_pair(a, b) & mask);
    size_t first_tomb = (size_t)-1;

    for (size_t i = 0; i < cap; ++i)
    {
        const MemoEntry *e = &entries[idx];
        if (e->state == 0)
        {
            *found = false;
            return (first_tomb != (size_t)-1) ? first_tomb : idx;
        }
        if (e->state == 1 && e->key_a == a && e->key_b == b)
        {
            *found = true;
            return idx;
        }
        if (e->state == 2 && first_tomb == (size_t)-1) first_tomb = idx;
        idx = (idx + 1) & mask;
    }
    *found = false;
    return first_tomb; /* table full of tombstones: caller should resize */
}

bool memo_get(const MemoTable *t, int64_t a, int64_t b, uint64_t *out)
{
    if (!t || !t->entries || t->capacity == 0) return false;
    bool found = false;
    size_t idx = memo_probe(t->entries, t->capacity, a, b, &found);
    if (!found) return false;
    if (out) *out = t->entries[idx].value;
    return true;
}

/* Rehash all live entries into a fresh table of `new_cap` slots.
 * Tombstones are dropped — that's the whole point of resize. */
static mv_status_t memo_rehash(MemoTable *t, size_t new_cap)
{
    MemoEntry *ne = calloc(new_cap, sizeof(MemoEntry));
    if (!ne) return MV_ERR_ALLOC;

    size_t mask = new_cap - 1;
    for (size_t i = 0; i < t->capacity; ++i)
    {
        MemoEntry *src = &t->entries[i];
        if (src->state != 1) continue;
        size_t idx = (size_t)(hash_pair(src->key_a, src->key_b) & mask);
        while (ne[idx].state == 1) idx = (idx + 1) & mask;
        ne[idx] = *src;
    }

    free(t->entries);
    t->entries = ne;
    t->capacity = new_cap;
    t->tombstones = 0;
    return MV_OK;
}

mv_status_t memo_set(MemoTable *t, int64_t a, int64_t b, uint64_t value)
{
    if (!t || !t->entries) return MV_ERR_NULL_PTR;

    /* Resize if adding one more would push us past load factor.
     * Both live entries and tombstones count toward the threshold,
     * because both consume probe-chain slots. */
    if ((double)(t->count + t->tombstones + 1) > MEMO_MAX_LOAD * (double)t->capacity)
    {
        mv_status_t st = memo_rehash(t, t->capacity * 2);
        if (st != MV_OK) return st;
    }

    bool found = false;
    size_t idx = memo_probe(t->entries, t->capacity, a, b, &found);
    if (idx == (size_t)-1) return MV_ERR_ALLOC;

    MemoEntry *e = &t->entries[idx];
    if (found)
    {
        e->value = value;
        return MV_OK;
    }

    if (e->state == 2) t->tombstones--;
    e->key_a = a;
    e->key_b = b;
    e->value = value;
    e->state = 1;
    t->count++;
    return MV_OK;
}

double memo_load_factor(const MemoTable *t)
{
    if (!t || t->capacity == 0) return 0.0;
    return (double)t->count / (double)t->capacity;
}

size_t memo_size(const MemoTable *t)
{
    return t ? t->count : 0;
}