#include "viz_ascii.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static mv_status_t sample_1d(scalar_field_fn f, void* ud, double xmin, double xmax, int n, double* xs, double* ys, double* out_ymin, double* out_ymax)
{
    MVec xv;
    mv_status_t st = mvec_create(&xv, 1);
    if (st != MV_OK) return st;
    double dx = (n > 1) ? (xmax - xmin) / (n - 1) : 0.0;
    double ymin =  1e300, ymax = -1e300;
    for (int i = 0; i < n; ++i)
    {
        xs[i] = xmin + i * dx;
        xv.data[0] = xs[i];
        ys[i] = f(&xv, ud);
        if (ys[i] < ymin) ymin = ys[i];
        if (ys[i] > ymax) ymax = ys[i];
    }
    mvec_destroy(&xv);
    *out_ymin = ymin;
    *out_ymax = ymax;
    return MV_OK;
}

static int y_to_row(double y, double ymin, double ymax, int height)
{
    if (ymax <= ymin) return height / 2;
    double t = (ymax - y) / (ymax - ymin);
    int row = (int)(t * (height - 1));
    if (row < 0) row = 0;
    if (row >= height) row = height - 1;
    return row;
}

static int x_to_col(double x, double xmin, double xmax, int width)
{
    if (xmax <= xmin) return width / 2;
    double t = (x - xmin) / (xmax - xmin);
    int col = (int)(t * (width - 1));
    if (col < 0) col = 0;
    if (col >= width) col = width - 1;
    return col;
}

#define YLABEL_W 9  // left margin width

mv_status_t ascii_plot_fn(scalar_field_fn f, void* ud, double xmin, double xmax, int width, int height)
{
    if (!f || width < 4 || height < 4) return MV_ERR_NULL_PTR;
    double* xs = malloc((size_t)width * sizeof(double));
    double* ys = malloc((size_t)width * sizeof(double));
    if (!xs || !ys)
    {
        free(xs);
        free(ys);
        return MV_ERR_ALLOC;
    }
    double ymin, ymax;
    mv_status_t st = sample_1d(f, ud, xmin, xmax, width, xs, ys, &ymin, &ymax);
    if (st != MV_OK)
    {
        free(xs);
        free(ys);
        return st;
    }
    double yrng = ymax - ymin;
    if (yrng < 1e-12) yrng = 1.0;
    ymin -= yrng * 0.05;
    ymax += yrng * 0.05;
    int zero_row = (ymin < 0.0 && ymax > 0.0) ? y_to_row(0.0, ymin, ymax, height) : -1;
    int zero_col = (xmin < 0.0 && xmax > 0.0) ? x_to_col(0.0, xmin, xmax, width) : -1;
    int* curve_row = malloc((size_t)width * sizeof(int));
    if (!curve_row)
    {
        free(xs);
        free(ys);
        return MV_ERR_ALLOC;
    }
    for (int c = 0; c < width; ++c)
        curve_row[c] = y_to_row(ys[c], ymin, ymax, height);
    for (int row = 0; row < height; ++row)
    {
        double y_here = ymax - (ymax - ymin) * row / (height - 1.0);
        if (row == 0 || row == height / 2 || row == height - 1)
            printf("%*.*f |", YLABEL_W - 2, 3, y_here);
        else
            printf("%*s |", YLABEL_W - 2, "");
        for (int col = 0; col < width; ++col)
        {
            bool on_curve = (curve_row[col] == row);
            bool on_zrow  = (zero_row == row);
            bool on_zcol  = (zero_col == col);
            if (on_curve) putchar('*');
            else if (on_zrow && on_zcol) putchar('+');
            else if (on_zrow) putchar('-');
            else if (on_zcol) putchar('|');
            else putchar(' ');
        }
        putchar('\n');
    }

    double xmid = (xmin + xmax) * 0.5;
    printf("%*s +", YLABEL_W - 2, "");
    for (int c = 0; c < width; ++c) putchar('-');
    printf("\n%*s  %-10.4g", YLABEL_W - 2, "", xmin);
    int pad = width / 2 - 10;
    if (pad > 0) printf("%*s", pad, "");
    printf("%-10.4g", xmid);
    pad = width - width / 2 - 10;
    if (pad > 0) printf("%*s", pad, "");
    printf("%.4g\n", xmax);
    free(xs);
    free(ys);
    free(curve_row);
    return MV_OK;
}

mv_status_t ascii_plot_fn2(scalar_field_fn f1, void* ud1, scalar_field_fn f2, void* ud2, double xmin, double xmax, int width, int height)
{
    if (!f1 || !f2 || width < 4 || height < 4) return MV_ERR_NULL_PTR;
    double* xs = malloc((size_t)width * sizeof(double));
    double* ys1 = malloc((size_t)width * sizeof(double));
    double* ys2 = malloc((size_t)width * sizeof(double));
    int* cr1 = malloc((size_t)width * sizeof(int));
    int* cr2 = malloc((size_t)width * sizeof(int));
    if (!xs || !ys1 || !ys2 || !cr1 || !cr2)
    {
        free(xs); free(ys1); free(ys2); free(cr1); free(cr2);
        return MV_ERR_ALLOC;
    }
    double ymin1, ymax1, ymin2, ymax2;
    mv_status_t st = sample_1d(f1, ud1, xmin, xmax, width, xs, ys1, &ymin1, &ymax1);
    if (st != MV_OK) goto done;
    double* tmp = malloc((size_t)width * sizeof(double));
    if (!tmp)
    {
        st = MV_ERR_ALLOC;
        goto done;
    }
    st = sample_1d(f2, ud2, xmin, xmax, width, tmp, ys2, &ymin2, &ymax2);
    free(tmp);
    if (st != MV_OK) goto done;
    double ymin = ymin1 < ymin2 ? ymin1 : ymin2;
    double ymax = ymax1 > ymax2 ? ymax1 : ymax2;
    double yrng = ymax - ymin;
    if (yrng < 1e-12) yrng = 1.0;
    ymin -= yrng * 0.05;
    ymax += yrng * 0.05;

    int zero_row = (ymin < 0.0 && ymax > 0.0) ? y_to_row(0.0, ymin, ymax, height) : -1;
    int zero_col = (xmin < 0.0 && xmax > 0.0) ? x_to_col(0.0, xmin, xmax, width)  : -1;
    for (int c = 0; c < width; ++c)
    {
        cr1[c] = y_to_row(ys1[c], ymin, ymax, height);
        cr2[c] = y_to_row(ys2[c], ymin, ymax, height);
    }
    printf("  Legend:  * = f1   # = f2   @ = overlap\n");
    for (int row = 0; row < height; ++row)
    {
        double y_here = ymax - (ymax - ymin) * row / (height - 1.0);
        if (row == 0 || row == height / 2 || row == height - 1)
            printf("%*.*f |", YLABEL_W - 2, 3, y_here);
        else
            printf("%*s |", YLABEL_W - 2, "");
        for (int col = 0; col < width; ++col)
        {
            bool a = (cr1[col] == row), b = (cr2[col] == row);
            bool on_zrow = (zero_row == row), on_zcol = (zero_col == col);
            if (a && b) putchar('@');
            else if (a) putchar('*');
            else if (b) putchar('#');
            else if (on_zrow && on_zcol) putchar('+');
            else if (on_zrow) putchar('-');
            else if (on_zcol) putchar('|');
            else putchar(' ');
        }
        putchar('\n');
    }
done:
    free(xs); free(ys1); free(ys2); free(cr1); free(cr2);
    return st;
}

void ascii_print_mat(const MMat* m, const char* label)
{
    if (!m) return;
    if (label) printf("%s  [%zux%zu]\n", label, m->rows, m->cols);
    for (size_t i = 0; i < m->rows; ++i)
    {
        printf("  [");
        for (size_t j = 0; j < m->cols; ++j)
            printf(" %9.4f", MMAT_AT(m, i, j));
        printf(" ]\n");
    }
}

void ascii_print_vec(const MVec* v, const char* label)
{
    if (!v) return;
    if (label) printf("%s  [dim=%zu]: ", label, v->dim);
    else printf("[dim=%zu]: ", v->dim);
    putchar('[');
    for (size_t i = 0; i < v->dim; ++i)
        printf(" %9.4f", v->data[i]);
    printf(" ]\n");
}

mv_status_t ascii_histogram(const double* data, size_t n, int bins, int bar_width)
{
    if (!data || n == 0 || bins <= 0 || bar_width < 4) return MV_ERR_NULL_PTR;
    double dmin = data[0], dmax = data[0];
    for (size_t i = 1; i < n; ++i)
    {
        if (data[i] < dmin) dmin = data[i];
        if (data[i] > dmax) dmax = data[i];
    }
    double rng = dmax - dmin;
    if (rng < 1e-12) rng = 1.0;
    int* counts = calloc((size_t)bins, sizeof(int));
    if (!counts) return MV_ERR_ALLOC;
    for (size_t i = 0; i < n; ++i)
    {
        int b = (int)((data[i] - dmin) / rng * bins);
        if (b >= bins) b = bins - 1;
        counts[b]++;
    }
    int maxc = 0;
    for (int b = 0; b < bins; ++b) if (counts[b] > maxc) maxc = counts[b];
    if (maxc == 0) maxc = 1;
    printf("  Histogram  n=%zu  bins=%d  range=[%.4g, %.4g]\n", n, bins, dmin, dmax);
    double bin_w = rng / bins;
    for (int b = 0; b < bins; ++b)
    {
        double lo = dmin + b * bin_w;
        double hi = lo + bin_w;
        int bar = counts[b] * bar_width / maxc;
        printf("  [%7.3g,%7.3g) |", lo, hi);
        for (int k = 0; k < bar; ++k) putchar('#');
        printf("%*s| %d\n", bar_width - bar, "", counts[b]);
    }
    free(counts);
    return MV_OK;
}