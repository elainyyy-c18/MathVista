#ifndef MATHVISTA_VIZ_CSV_H
#define MATHVISTA_VIZ_CSV_H
#include <stddef.h>
#include "math_engine.h"
#include "calculus.h"

mv_status_t csv_export_fn(const char* filepath, scalar_field_fn f, void* ud, double xmin, double xmax, int steps);
mv_status_t csv_export_fn2d(const char* filepath, scalar_field_fn f, void* ud, double x0, double x1, int nx, double y0, double y1, int ny);
mv_status_t csv_export_mat(const char* filepath, const MMat* m);
mv_status_t csv_export_vec(const char* filepath, const MVec* v);
mv_status_t gnuplot_write_1d(const char* script_path, const char* csv_path, const char* png_path, const char* title);
mv_status_t gnuplot_write_2d(const char* script_path, const char* csv_path, const char* png_path, const char* title, int nx, int ny);

#endif