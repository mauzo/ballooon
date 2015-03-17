/* pad.cpp
 * Scratchpad and flash strings.
 */

#include "ballooon.h"

char pad[PADSIZ];

#define vsnprintf __builtin_vsnprintf

byte
pad_fstr (fstr src)
{
    long len;
    const char PROGMEM *pstr = (const char PROGMEM *)src;

    len = strlcpy_P(pad, pstr, PADSIZ);
    return len < PADSIZ ? len : 0xff;
}

byte
pad_vform (const char *fmt, va_list ap)
{
    long len;

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

byte
pad_vfform (fstr fs, va_list ap)
{
    const char PROGMEM  *pstr;
    char                fmt[PADSIZ];
    long                len;

    pstr = (const char PROGMEM *)fs;
    len = strlcpy_P(fmt, pstr, PADSIZ);
    if (len > PADSIZ) {
        *pad = 0;
        return 0xff;
    }

    return pad_vform(fmt, ap);
}

byte
pad_fform (fstr fmt, ...)
{
    va_list ap;
    byte    len;

    va_start(ap, fmt);
    len = pad_vfform(fmt, ap);
    va_end(ap);

    return len;
}

