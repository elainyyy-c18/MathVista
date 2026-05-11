#ifndef MATHVISTA_CALCULUS_H
#define MATHVISTA_CALCULUS_H

#include "math_engine.h"

typedef double (*scalar_field_fn)(const MVec* x, void* user_data);
mv_status_t numeric_gradient(MVec* grad, scalar_field_fn f, const MVec* x, void* user_data, double h);
mv_status_t lagrange_solve(MVec* x_out, double* lambda_out, scalar_field_fn f, scalar_field_fn g, void* f_ud, void* g_ud, const MVec* x_init, int max_iter, double tol);

#endif