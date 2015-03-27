/* amd64.c
 * amd64 implementations for Arduino functions.
 */

#include <time.h>

long
millis (void)
{
    struct timespec tp;

    clock_gettime(CLOCK_VIRTUAL, &tp);
    return (long)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}

