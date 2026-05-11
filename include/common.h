#ifndef MATHVISTA_COMMON_H
#define MATHVISTA_COMMON_H
#include <time.h>
#include <stdarg.h>

typedef struct
{
    struct timespec t0;
} MvTimer;

void mv_timer_start(MvTimer* t);
double mv_timer_elapsed_ms(const MvTimer* t);
typedef enum
{
    MV_INFO = 0,
    MV_WARN,
    MV_ERROR
} MvLogLevel;

void mv_log(MvLogLevel lvl, const char* fmt, ...);
void mv_banner(const char* title);
void mv_rule();

#endif