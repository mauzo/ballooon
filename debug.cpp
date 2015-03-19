/* debug.cpp
 * Debugging functions.
 *
 * For the moment, debugging goes via the serial port.
 */

#include "ballooon.h"

#define snprintf __builtin_snprintf

void
debug_setup (void)
{ 
    // Start debug output
    Serial.begin(9600);
}

EXT_C void
warn (byte level, const char *msg)
{
    static const char code[]    = "PEWNLD";
    char stmp[14];
    long now                    = millis();
    
    snprintf(stmp, sizeof stmp, "%2lu'%02lu.%02lu [%c] ",
        now / 60000, (now % 60000) / 1000,
        (now % 1000) / 10, code[level]);

    warnx(level, stmp);
    warnx(level, msg);
    warnx(level, "\r\n");
}

EXT_C void
warnf (byte level, const char *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    pad_vform(fmt, ap);
    va_end(ap);

    warn(level, pad);
}

EXT_C void
warnx (byte level, const char *msg)
{
    if (isF(msg)) {
        strlcpyF(pad, msg, PADSIZ);
        msg = pad;
    }
    Serial.print(msg);
}

