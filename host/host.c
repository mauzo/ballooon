/* amd64.c
 * amd64 implementations for Arduino functions.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "host.h"

long
millis (void)
{
    struct timespec tp;
    static long start = 0;
    long now;

    clock_gettime(CLOCK_REALTIME_FAST, &tp);
    now = (long)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    if (start) {
        return now - start;
    }
    else {
        start = now;
        return 0;
    }
}

void
delay (unsigned long d)
{
    usleep(d * 1000);
}

#define GREEN   "\033[32m"
#define RESET   "\033[0m"

void
host_warn (const char *fmt, ...)
{
    va_list ap;
    char    *msg;

    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);

    fprintf(stderr, GREEN "%s" RESET, msg);
    free(msg);
}
