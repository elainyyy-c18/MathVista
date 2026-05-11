#define _POSIX_C_SOURCE 200809L
#include "../../include/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void mv_timer_start(MvTimer* t)
{
    if (!t) return;
    clock_gettime(CLOCK_MONOTONIC, &t->t0);
}

double mv_timer_elapsed_ms(const MvTimer* t)
{
    if (!t) return 0.0;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double sec = (double)(now.tv_sec  - t->t0.tv_sec);
    double ns  = (double)(now.tv_nsec - t->t0.tv_nsec);
    return sec * 1000.0 + ns / 1.0e6;
}

static const char* lv_tag[]  = { "INFO ", "WARN ", "ERROR" };
static const char* lv_ansi[] = { "\033[0m", "\033[33m", "\033[31m" };
#define ANSI_RESET "\033[0m"

void mv_log(MvLogLevel lvl, const char* fmt, ...)
{
    int l = (int)lvl;
    if (l < 0 || l > 2) l = 0;
    fprintf(stderr, "%s[%s]" ANSI_RESET " ", lv_ansi[l], lv_tag[l]);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

#define BANNER_WIDTH 52

void mv_banner(const char* title)
{
    int tlen = title ? (int)strlen(title) : 0;
    int width = tlen + 4 > BANNER_WIDTH ? tlen + 4 : BANNER_WIDTH;
    putchar('\n');
    for (int i = 0; i < width; ++i) putchar('=');
    printf("\n  %s\n", title ? title : "");
    for (int i = 0; i < width; ++i) putchar('=');
    putchar('\n');
}
void mv_rule()
{
    for (int i = 0; i < BANNER_WIDTH; ++i) putchar('-');
    putchar('\n');
}