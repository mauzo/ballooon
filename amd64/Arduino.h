#ifndef __AVR_COMPAT_H
#define __AVR_COMPAT_H

#include <time.h>
#include <stdio.h>

typedef unsigned char byte;
typedef unsigned char boolean;

#define true    1
#define false   0

#define memcpy_P    memcpy
#define strlcpy_P   strlcpy
#define vsnprintf_P vsnprintf

static long
millis (void)
{
    struct timespec tp;

    clock_gettime(CLOCK_VIRTUAL, &tp);
    return (long)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}

#ifdef __cplusplus

#include "HardwareSerial.h"

extern HardwareSerial Serial;

#endif /* __cplusplus */

#endif
