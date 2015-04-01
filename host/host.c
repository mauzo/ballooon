/* amd64.c
 * amd64 implementations for Arduino functions.
 */

#include <time.h>
#include <unistd.h>

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
