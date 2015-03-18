/* pad.cpp
 * Scratchpad and flash strings.
 */

#include "ballooon.h"

char pad[PADSIZ];

#define vsnprintf __builtin_vsnprintf

byte
pad_vform (const char *fmt, va_list ap)
{
    char bounce[PADSIZ];
    long len;
    
    if (isF(fmt)) {
        /* can't use the pad, we're about to format into it... */
        strlcpyF(bounce, fmt, PADSIZ);
        fmt = bounce;
    }
    len = vsnprintf(pad, PADSIZ, fmt, ap);
    return len < PADSIZ ? len : 0xff;
}

byte
pad_form (const char *fmt, ...)
{
    va_list ap;
    byte    len;

    va_start(ap, fmt);
    len = pad_vform(fmt, ap);
    va_end(ap);

    return len;
}
