/* warn.cpp
 * Logging functions.
 *
 * For the moment, debugging goes via the serial port.
 */

#include <stdarg.h>
#include <stdio.h>

#include <Arduino.h>

#include "warn.h"

typedef struct {
    void    (*write)(const char * msg);
    byte    level;
} writer;

static void     serial_write    (const char *msg);

static writer writers[] = {
    { serial_write, WDUMP },
    { NULL, 0 }
};

static void
serial_write (const char *msg)
{
    Serial.print(msg);
}

void
warn_nl (byte level)
{
#ifdef AMD
    warnx(level, "\n");
#else
    warnx(level, "\r\n");
#endif
}

void
warn_stamp (byte level)
{
    static const char code[]    = "PEWnldD";
    char stmp[14];
    long now                    = millis();
    
    snprintf(stmp, sizeof stmp, "[%2lu'%02lu.%02lu %c] ",
        now / 60000, (now % 60000) / 1000,
        (now % 1000) / 10, code[level]);

    warnx(level, stmp);
}

void
warn (byte level, const char *msg)
{
    warn_stamp(level);
    warnx(level, msg);
    warn_nl(level);
}

void
warnf (byte level, const char *fmt, ...)
{
    va_list     ap;
    char        buf[81];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    warn(level, buf);
}

void
warnx (byte level, const char *msg)
{
    writer *w;

    for (w = writers; w->write; w++)
        if (level <= w->level)
            w->write(msg);
}

void
warnxf (byte level, const char *fmt, ...)
{
    va_list     ap;
    char        buf[81];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    warnx(level, buf);
}
