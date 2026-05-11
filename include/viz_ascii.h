#ifndef MATHVISTA_VIZ_ASCII_H
#define MATHVISTA_VIZ_ASCII_H
#include <stddef.h>
#include "math_engine.h"
#include "calculus.h"

mv_status_t ascii_plot_fn(scalar_field_fn f, void* ud, double xmin, double xmax, int width, int height);
mv_status_t ascii_plot_fn2(scalar_field_fn f1, void* ud1, scalar_field_fn f2, void* ud2, double xmin, double xmax, int width, int height);

void ascii_print_mat(const MMat* m, const char* label);
void ascii_print_vec(const MVec* v, const char* label);
mv_status_t ascii_histogram(const double* data, size_t n, int bins, int bar_width);

#endif