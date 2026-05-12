#include "transform3d.h"
#include <math.h>
#include <string.h>

mv_status_t t3d_identity(MMat* out)
{
    if (!out || !out->data) return MV_ERR_NULL_PTR;
    if (out->rows != 4 || out->cols != 4) return MV_ERR_DIM_MISMATCH;
    memset(out->data, 0, 16 * sizeof(double));
    for (int i = 0; i < 4; ++i) out->data[i * 4 + i] = 1.0;
    return MV_OK;
}
mv_status_t t3d_translation(MMat* out, double tx, double ty, double tz)
{
    mv_status_t st = t3d_identity(out);
    if (st != MV_OK) return st;
    out->data[0 * 4 + 3] = tx;
    out->data[1 * 4 + 3] = ty;
    out->data[2 * 4 + 3] = tz;
    return MV_OK;
}
mv_status_t t3d_rotation_x(MMat* out, double theta)
{
    mv_status_t st = t3d_identity(out);
    if (st != MV_OK) return st;
    double c = cos(theta), s = sin(theta);
    out->data[1 * 4 + 1] = c;
    out->data[1 * 4 + 2] = -s;
    out->data[2 * 4 + 1] = s;
    out->data[2 * 4 + 2] = c;
    return MV_OK;
}
mv_status_t t3d_rotation_y(MMat* out, double theta)
{
    mv_status_t st = t3d_identity(out);
    if (st != MV_OK) return st;
    double c = cos(theta), s = sin(theta);
    out->data[0 * 4 + 0] = c;
    out->data[0 * 4 + 2] = s;
    out->data[2 * 4 + 0] = -s;
    out->data[2 * 4 + 2] = c;
    return MV_OK;
}
mv_status_t t3d_rotation_z(MMat* out, double theta)
{
    mv_status_t st = t3d_identity(out);
    if (st != MV_OK) return st;
    double c = cos(theta), s = sin(theta);
    out->data[0 * 4 + 0] = c;
    out->data[0 * 4 + 1] = -s;
    out->data[1 * 4 + 0] = s;
    out->data[1 * 4 + 1] = c;
    return MV_OK;
}

mv_status_t t3d_perspective(MMat* out, double fov_y, double aspect, double n_plane, double f_plane)
{
    if (!out || !out->data) return MV_ERR_NULL_PTR;
    if (out->rows != 4 || out->cols != 4) return MV_ERR_DIM_MISMATCH;
    if (f_plane <= n_plane || n_plane <= 0.0) return MV_ERR_OUT_OF_RANGE;
    if (aspect <= 0.0) return MV_ERR_OUT_OF_RANGE;
    memset(out->data, 0, 16 * sizeof(double));
    double t = 1.0 / tan(fov_y * 0.5);

    out->data[0 * 4 + 0] = t / aspect;
    out->data[1 * 4 + 1] = t;
    out->data[2 * 4 + 2] = -(f_plane + n_plane) / (f_plane - n_plane);
    out->data[2 * 4 + 3] = -(2.0 * f_plane * n_plane) / (f_plane - n_plane);
    out->data[3 * 4 + 2] = -1.0;
    return MV_OK;
}
mv_status_t t3d_project(double* out_x, double* out_y, const MMat* transform, double x, double y, double z, int screen_w, int screen_h)
{
    if (!out_x || !out_y || !transform || !transform->data) return MV_ERR_NULL_PTR;
    if (transform->rows != 4 || transform->cols != 4) return MV_ERR_DIM_MISMATCH;

    double v[4] = { x, y, z, 1.0 };
    double r[4] = { 0.0, 0.0, 0.0, 0.0 };
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            r[i] += transform->data[i * 4 + j] * v[j];

    if (r[3] == 0.0) return MV_ERR_SINGULAR;

    double nx = r[0] / r[3];
    double ny = r[1] / r[3];

    *out_x = (nx + 1.0) * 0.5 * (double)screen_w;
    *out_y = (1.0 - ny) * 0.5 * (double)screen_h;
    return MV_OK;
}