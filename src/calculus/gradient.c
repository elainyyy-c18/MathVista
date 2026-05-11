#include "../../include/calculus.h"
#include "../../include/math_engine.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

mv_status_t numeric_gradient(MVec* grad, scalar_field_fn f, const MVec* x, void* user_data, double h)
{
    if (!grad || !f || !x) return MV_ERR_NULL_PTR;
    if (grad->dim != x->dim) return MV_ERR_DIM_MISMATCH;
    if (h <= 0.0) return MV_ERR_OUT_OF_RANGE;
    MVec xc;
    mv_status_t st = mvec_create(&xc, x->dim);
    if (st != MV_OK) return st;
    memcpy(xc.data, x->data, x->dim * sizeof(double));
    for (size_t i = 0; i < x->dim; ++i)
    {
        double orig = xc.data[i];
        xc.data[i] = orig + h;
        double fp = f(&xc, user_data);
        xc.data[i] = orig - h;
        double fm = f(&xc, user_data);
        xc.data[i] = orig;
        grad->data[i] = (fp - fm) / (2.0 * h);
    }
    mvec_destroy(&xc);
    return MV_OK;
}

mv_status_t numeric_hessian(MMat* out, scalar_field_fn f, const MVec* x, void* user_data, double h)
{
    if (!out || !f || !x) return MV_ERR_NULL_PTR;
    if (out->rows != x->dim || out->cols != x->dim) return MV_ERR_DIM_MISMATCH;
    if (h <= 0.0) return MV_ERR_OUT_OF_RANGE;
    size_t n = x->dim;
    double h2 = h * h;
    MVec xc;
    mv_status_t st = mvec_create(&xc, n);
    if (st != MV_OK) return st;
    memcpy(xc.data, x->data, n * sizeof(double));
    double f0 = f(&xc, user_data);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = i; j < n; ++j)
        {
            double val;
            if (i == j)
            {
                xc.data[i] = x->data[i] + h;
                double fph = f(&xc, user_data);
                xc.data[i] = x->data[i] - h;
                double fmh = f(&xc, user_data);
                xc.data[i] = x->data[i];
                val = (fph - 2.0 * f0 + fmh) / h2;
            }
            else
            {
                xc.data[i] = x->data[i] + h; xc.data[j] = x->data[j] + h;
                double fpp = f(&xc, user_data);
                xc.data[j] = x->data[j] - h;
                double fpm = f(&xc, user_data);
                xc.data[i] = x->data[i] - h; xc.data[j] = x->data[j] + h;
                double fmp = f(&xc, user_data);
                xc.data[j] = x->data[j] - h;
                double fmm = f(&xc, user_data);
                xc.data[i] = x->data[i]; xc.data[j] = x->data[j];
                val = (fpp - fpm - fmp + fmm) / (4.0 * h2);
            }
            MMAT_AT(out, i, j) = val;
            MMAT_AT(out, j, i) = val;
        }
    }
    mvec_destroy(&xc);
    return MV_OK;
}

mv_status_t numeric_directional_deriv(double* out, scalar_field_fn f, const MVec* x, const MVec* dir, void* user_data, double h)
{
    if (!out || !f || !x || !dir) return MV_ERR_NULL_PTR;
    if (x->dim != dir->dim) return MV_ERR_DIM_MISMATCH;
    if (h <= 0.0) return MV_ERR_OUT_OF_RANGE;
    double norm = 0.0;
    for (size_t i = 0; i < dir->dim; ++i) norm += dir->data[i] * dir->data[i];
    norm = sqrt(norm);
    if (norm < 1e-15) return MV_ERR_SINGULAR;
    MVec xp, xm;
    mv_status_t st = mvec_create(&xp, x->dim);
    if (st != MV_OK) return st;
    st = mvec_create(&xm, x->dim);
    if (st != MV_OK)
    {
        mvec_destroy(&xp);
        return st;
    }
    for (size_t i = 0; i < x->dim; ++i)
    {
        double di = dir->data[i] / norm;
        xp.data[i] = x->data[i] + h * di;
        xm.data[i] = x->data[i] - h * di;
    }
    *out = (f(&xp, user_data) - f(&xm, user_data)) / (2.0 * h);
    mvec_destroy(&xp);
    mvec_destroy(&xm);
    return MV_OK;
}