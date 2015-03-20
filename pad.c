/* pad.cpp
 * Scratchpad and flash strings.
 */

#include "ballooon.h"

char pad[PADSIZ];

#define vsnprintf __builtin_vsnprintf
#define snprintf __builtin_snprintf

#if 0
void
pad_dump (char *from, size_t len)
{
    static const char __flash hex[] = "0123456789abcdef";
    char    dump[16*3+2], *p;
    int     i;

    if (len > PADSIZ)
        panic(sF("Pad dump too long"));

    dump[16*3+1] = '\0';
    p = dump;

    for (i = 0; i < len; i++) {
        if (i % 16 == 8)
            *p++ = ' ';

        *p++ = ' ';
        *p++ = hex[from[i] >> 4];
        *p++ = hex[from[i] & 0xf];

        if (i % 16 == 15) {
            warn(WDUMP, dump);
            p = dump;
        }
    }
    if (p > dump) {
        *p = '\0';
        warn(WDUMP, dump);
    }
}
#endif

void
pad_dump (char *from, size_t len)
{
    char buf[4];
    int i;

    for (i = 0; i < len; i++) {
        snprintf(buf, sizeof(buf), " %02x", from[i]);
        warnx(WDUMP, buf);
        if (i % 16 == 15)
            warnx(WDUMP, "\r\n");
    }
    warnx(WDUMP, "\r\n");
}

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
