/* amd64.c
 * amd64 implementations for Arduino functions.
 */

#include <time.h>

#ifndef CLOCK_REALTIME_FAST
#  define CLOCK_REALTIME_FAST CLOCK_REALTIME
#endif

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

