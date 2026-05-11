#ifndef MATHVISTA_MATH_ENGINE_H
#define MATHVISTA_MATH_ENGINE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    MV_OK = 0,
    MV_ERR_ALLOC,
    MV_ERR_DIM_MISMATCH,
    MV_ERR_OUT_OF_RANGE,
    MV_ERR_SINGULAR,
    MV_ERR_OVERFLOW,
    MV_ERR_NULL_PTR,
    MV_ERR_NOT_FOUND,
    MV_ERR_CONVERGE
} mv_status_t;

const char* mv_strerror(mv_status_t s);
typedef struct
{
    size_t dim;
    double* data;
} MVec;

mv_status_t mvec_create(MVec* out, size_t dim);
mv_status_t mvec_create_from(MVec* out, const double* src, size_t dim);
void mvec_destroy(MVec* v);

mv_status_t mvec_copy(MVec* dst, const MVec* src);
mv_status_t mvec_add(MVec* out, const MVec* a, const MVec* b);
mv_status_t mvec_sub(MVec* out, const MVec* a, const MVec* b);
mv_status_t mvec_scale(MVec* v, double s);
mv_status_t mvec_dot(double* out, const MVec* a, const MVec* b);
mv_status_t mvec_norm(double* out, const MVec* v);
mv_status_t mvec_normalize(MVec* v);

typedef struct
{
    size_t rows;
    size_t cols;
    double* data;
} MMat;

#define MMAT_AT(m, i, j) ((m)->data[(i) * (m)->cols + (j)])

mv_status_t mmat_create(MMat* out, size_t rows, size_t cols);
mv_status_t mmat_identity(MMat* out, size_t n);
void mmat_destroy(MMat* m);

mv_status_t mmat_mul(MMat* out, const MMat* A, const MMat* B);
mv_status_t mmat_vec_mul(MVec* out, const MMat* A, const MVec* x);
mv_status_t mmat_transpose(MMat* out, const MMat* A);

typedef struct
{
    int64_t key_a;
    int64_t key_b;
    uint64_t value;
    uint8_t state; // 0 empty, 1 occupied, 2 tombstone
} MemoEntry;

typedef struct
{
    MemoEntry* entries;
    size_t capacity;
    size_t count;
    size_t tombstones;
} MemoTable;

mv_status_t memo_create(MemoTable* out, size_t hint);
void memo_destroy(MemoTable* t);
bool memo_get(const MemoTable* t, int64_t a, int64_t b, uint64_t* out);
mv_status_t memo_set(MemoTable* t, int64_t a, int64_t b, uint64_t value);
double memo_load_factor(const MemoTable* t);
size_t memo_size(const MemoTable* t);

#endif