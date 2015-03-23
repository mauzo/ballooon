/* pad.cpp
 * Scratchpad and flash strings.
 */

#include "ballooon.h"

char pad[PADSIZ];

#if 1
void
pad_dump (const char *msg, char *from, size_t len)
{
    static const char __flash hex[] = "0123456789abcdef";
    char    dump[16*3+2], *p;
    int     i;

    if (len > PADSIZ)
        panic(sF("Pad dump too long"));

    if (isF(msg))
        warnf(WDUMP, sF("Dumping [%u] bytes from [%x]: %"fF),
            (unsigned)len, (unsigned)from, aF(msg));
    else
        warnf(WDUMP, sF("Dumping [%u] bytes from [%x]: %s"),
            (unsigned)len, (unsigned)from, msg);

    dump[16*3+1] = '\0';
    p = dump;

    for (i = 0; i < len; i++) {
        if (i % 16 == 8)
            *p++ = ' ';

        *p++ = ' ';
        *p++ = hex[(byte)dF(from+i) >> 4];
        *p++ = hex[(byte)dF(from+i) & 0xf];

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

#else

void
pad_dump (const char *msg, char *from, size_t len)
{
    char buf[4];
    int i;

    if (isF(msg))
        warnf(WDUMP, sF("Dumping [%u] bytes from [%x]: %"fF),
            (unsigned)len, (unsigned)from, aF(msg));
    else
        warnf(WDUMP, sF("Dumping [%u] bytes from [%x]: %s"),
            (unsigned)len, (unsigned)from, msg);

    for (i = 0; i < len; i++) {
        snprintf(buf, sizeof(buf), " %02x", (byte)from[i]);
        warnx(WDUMP, buf);
        if (i % 16 == 15)
            warnx(WDUMP, "\r\n");
    }
    warnx(WDUMP, "\r\n");
}
#endif

byte
pad_vform (const char *fmt, va_list ap)
{
    long len;
    
    len = vsnprintfF(pad, PADSIZ, fmt, ap);
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
