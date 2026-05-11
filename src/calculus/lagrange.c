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

mv_status_t lagrange_solve(MVec* x_out, double* lambda_out, scalar_field_fn f, scalar_field_fn g, void* f_ud, void* g_ud, const MVec* x_init, int max_iter, double tol)
{
    if (!x_out || !f || !g || !x_init) return MV_ERR_NULL_PTR;
    if (x_out->dim != x_init->dim) return MV_ERR_DIM_MISMATCH;
    size_t n = x_init->dim;
    mv_status_t st = mvec_copy(x_out, x_init);
    if (st != MV_OK) return st;
    MVec grad_f, grad_g, step;
    st = mvec_create(&grad_f, n);
    if (st != MV_OK) return st;
    st = mvec_create(&grad_g, n);
    if (st != MV_OK) { mvec_destroy(&grad_f); return st; }
    st = mvec_create(&step, n);
    if (st != MV_OK) { mvec_destroy(&grad_f); mvec_destroy(&grad_g); return st; }
    const double h = 1e-6;
    const double lr = 0.05;
    const double sing_eps = 1e-20;
    int iter;
    bool converged = false;
    for (iter = 0; iter < max_iter; ++iter)
    {
        st = numeric_gradient(&grad_f, f, x_out, f_ud, h);
        if (st != MV_OK) goto cleanup;
        st = numeric_gradient(&grad_g, g, x_out, g_ud, h);
        if (st != MV_OK) goto cleanup;

        double gg, fg;
        mvec_dot(&gg, &grad_g, &grad_g);
        if (gg < sing_eps)
        {
            st = MV_ERR_SINGULAR;
            goto cleanup;
        }
        mvec_dot(&fg, &grad_f, &grad_g);
        double proj_coef = fg / gg;
        for (size_t i = 0; i < n; ++i)
            step.data[i] = grad_f.data[i] - proj_coef * grad_g.data[i];

        for (size_t i = 0; i < n; ++i) x_out->data[i] += lr * step.data[i];

        st = numeric_gradient(&grad_g, g, x_out, g_ud, h);
        if (st != MV_OK) goto cleanup;
        double gx = g(x_out, g_ud);
        mvec_dot(&gg, &grad_g, &grad_g);
        if (gg < sing_eps)
        {
            st = MV_ERR_SINGULAR;
            goto cleanup;
        }
        for (size_t i = 0; i < n; ++i)
            x_out->data[i] -= (gx / gg) * grad_g.data[i];

        double tan_norm;
        mvec_norm(&tan_norm, &step);
        if (tan_norm < tol) { converged = true; break; }
    }
    if (lambda_out)
    {
        st = numeric_gradient(&grad_f, f, x_out, f_ud, h);
        if (st == MV_OK) st = numeric_gradient(&grad_g, g, x_out, g_ud, h);
        if (st == MV_OK)
        {
            double fg, gg;
            mvec_dot(&fg, &grad_f, &grad_g);
            mvec_dot(&gg, &grad_g, &grad_g);
            *lambda_out = (gg > sing_eps) ? fg / gg : 0.0;
        }
    }

cleanup:
    mvec_destroy(&grad_f);
    mvec_destroy(&grad_g);
    mvec_destroy(&step);
    if (st != MV_OK) return st;
    return converged ? MV_OK : MV_ERR_CONVERGE;
}