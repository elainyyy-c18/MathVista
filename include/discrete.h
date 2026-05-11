#ifndef MATHVISTA_DISCRETE_H
#define MATHVISTA_DISCRETE_H

#include <stdint.h>
#include "math_engine.h"

// Stirling numbers of S(n, k)
mv_status_t stirling2_memoized(int n, int k, MemoTable* memo, uint64_t* out);
mv_status_t stirling2_iterative(int n, int k, uint64_t* out);

// Catalan numbers and Dyck path visualize
mv_status_t catalan_memoized(int n, MemoTable* memo, uint64_t* out);
mv_status_t catalan_iterative(int n, uint64_t* out);
mv_status_t catalan_render_dyck(int n, int which);

#endif