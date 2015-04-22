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
    void    (*write_P)(const char * msg);
    byte    level;
} writer;

static void     serial_write    (const char *msg);
static void     serial_write_P  (const char *msg);

static writer writers[] = {
    { serial_write, serial_write_P, WDUMP },
    { NULL, 0 }
};

static void
serial_write (const char *msg)
{
    Serial.print(msg);
}

static void
serial_write_P (const char *msg)
{
    Serial.print(reinterpret_cast<const __FlashStringHelper *>(msg));
}

void
warn_nl (byte level)
{
#ifdef HOST
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
    
    snprintf_P(stmp, sizeof stmp, sF("[%2lu'%02lu.%02lu %c] "),
        now / 60000, (now % 60000) / 1000,
        (now % 1000) / 10, code[level]);

    warnx_R(level, stmp);
}

void
warn_P (byte level, const char *msg)
{
    warn_stamp(level);
    warnx_P(level, msg);
    warn_nl(level);
}

void
warn_R (byte level, const char *msg)
{
    warn_stamp(level);
    warnx_R(level, msg);
    warn_nl(level);
}

void
warnf_P (byte level, const char *fmt, ...)
{
    va_list     ap;
    char        buf[81];

    va_start(ap, fmt);
    vsnprintf_P(buf, sizeof buf, fmt, ap);
    va_end(ap);

    warn_R(level, buf);
}

void
warnf_R (byte level, const char *fmt, ...)
{
    va_list     ap;
    char        buf[81];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    warn_R(level, buf);
}

void
warnx_P (byte level, const char *msg)
{
    writer *w;

    for (w = writers; w->write; w++)
        if (level <= w->level)
            w->write_P(msg);
}

void
warnx_R (byte level, const char *msg)
{
    writer *w;

    for (w = writers; w->write; w++)
        if (level <= w->level)
            w->write(msg);
}

void
warnxf_P (byte level, const char *fmt, ...)
{
    va_list     ap;
    char        buf[81];

    va_start(ap, fmt);
    vsnprintf_P(buf, sizeof buf, fmt, ap);
    va_end(ap);

    warnx_R(level, buf);
}

void
warnxf_R (byte level, const char *fmt, ...)
{
    va_list     ap;
    char        buf[81];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    warnx_R(level, buf);
}
