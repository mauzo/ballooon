/* debug.cpp
 * Debugging functions.
 *
 * For the moment, debugging goes via the serial port.
 */

#include "ballooon.h"

void
debug_setup (void)
{ 
    // Start debug output
    Serial.begin(9600);
}

EXT_C void
warn (const char *msg)
{
    warnx(msg);
    warnx("\r\n");
}

EXT_C void
warnx (const char *msg)
{
    if (isF(msg)) {
        strlcpyF(pad, msg, PADSIZ);
        msg = pad;
    }
    Serial.print(msg);
}

EXT_C void
warnf (const char *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    pad_vform(fmt, ap);
    va_end(ap);

    warnx(pad);
    warnx("\r\n");
}
