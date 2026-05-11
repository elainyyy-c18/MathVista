#ifndef MATHVISTA_TRANSFORM3D_H
#define MATHVISTA_TRANSFORM3D_H

#include "math_engine.h"

mv_status_t t3d_identity(MMat* out);
mv_status_t t3d_translation(MMat* out, double tx, double ty, double tz);
mv_status_t t3d_rotation_x(MMat* out, double theta);
mv_status_t t3d_rotation_y(MMat* out, double theta);
mv_status_t t3d_rotation_z(MMat* out, double theta);

mv_status_t t3d_perspective(MMat* out, double fov_y, double aspect, double n_plane, double f_plane);

mv_status_t t3d_project(double* out_x, double* out_y, const MMat* transform, double x, double y, double z, int screen_w, int screen_h);

#endif