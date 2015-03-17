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
warn (fstr fmsg)
{
    pad_fstr(fmsg);
    warnx(pad);
    warnx("\r\n");
}

void
warnx (const char *msg)
{
    Serial.print(msg);
}

void
warnf (fstr fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    pad_vfform(fmt, ap);
    va_end(ap);

    warnx(pad);
    warnx("\r\n");
}
