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

static char warn_buf[81];

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

static void
warn_write (byte flags, byte level, const char *msg)
{
    writer  *w;

    for (w = writers; w->write; w++) {
        if (level <= w->level) {
            if (flags & WARN_PGM)
                w->write_P(msg);
            else
                w->write(msg);
        }
    }
}

void
warn_flags (byte flags, byte level, const char *msg, ...)
{
    static const char   code[]    = "PEWnldD";
    long                now;
    va_list             ap;

    if (flags & WARN_STMP) {
        now = millis();
        snprintf_P(warn_buf, sizeof warn_buf, sF("[%2lu'%02lu.%02lu %c] "),
            now / 60000, (now % 60000) / 1000,
            (now % 1000) / 10, code[level]);
        warn_write(0, level, warn_buf);
    }

    if (flags & WARN_FMT) {
        va_start(ap, msg);
        if (flags & WARN_PGM)
            vsnprintf_P(warn_buf, sizeof warn_buf, msg, ap);
        else
            vsnprintf(warn_buf, sizeof warn_buf, msg, ap);
        va_end(ap);

        msg     = warn_buf;
        flags   &= ~WARN_PGM;
    }

    warn_write(flags, level, msg);

    if (flags & WARN_NL)
#ifdef HOST
        warn_write(WARN_PGM, level, sF("\n"));
#else
        warn_write(WARN_PGM, level, sF("\r\n"));
#endif
}

