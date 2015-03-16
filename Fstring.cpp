/* Fstring.cpp
 * Functions for dealing with strings in flash.
 */

#include "ballooon.h"

static char buf[FSTRSIZ];

char *
strFdup (Fstr src)
{
    const char PROGMEM *pstr = (const char PROGMEM *)src;

    strlcpy_P(buf, pstr, sizeof buf);
    return buf;
}

char *
vbprintf (const char *fmt, va_list ap)
{
    vsnprintf(buf, sizeof buf, fmt, ap);
    return buf;
}

char *
bprintf (const char *fmt, ...)
{
    va_list ap;
    char    *rv;

    va_start(ap, fmt);
    rv = vbprintf(fmt, ap);
    va_end(ap);

    return rv;
}

char *
vFprintf (Fstr fstr, va_list ap)
{
    const char PROGMEM  *pstr;
    char                fmt[FSTRSIZ];

    pstr = (const char PROGMEM *)fstr;
    strlcpy_P(fmt, pstr, sizeof fmt);
    return vbprintf(fmt, ap);
}

char *
Fprintf (Fstr fmt, ...)
{
    va_list ap;
    char    *rv;

    va_start(ap, fmt);
    rv = vFprintf(fmt, ap);
    va_end(ap);

    return rv;
}

