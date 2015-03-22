#ifndef __AVR_COMPAT_H
#define __AVR_COMPAT_H

#include <time.h>
#include <stdio.h>

#define __flash
#define PROGMEM

typedef unsigned char byte;

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

class HardwareSerial {
public:
    HardwareSerial () { }
    void begin (int) { }
    void print (const char *m)
    {
        fputs(m, stdout);
    }
};

static HardwareSerial Serial;
        
#endif /* __cplusplus */

#endif
