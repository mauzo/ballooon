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

void
warn (Fstr Fmsg)
{
    Serial.println(Fmsg);
}

void
warnx (const char *msg)
{
    Serial.print(msg);
}

void
warnf (Fstr fmt, ...)
{
    va_list     ap;
    char        *buf;

    va_start(ap, fmt);
    buf = vFprintf(fmt, ap);
    va_end(ap);

    Serial.println(buf);
}
