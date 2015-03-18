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
warn (fstr fmsg)
{
    pad_fstr(fmsg);
    warnx(pad);
    warnx("\r\n");
}

EXT_C void
warnx (const char *msg)
{
    Serial.print(msg);
}

EXT_C void
warnf (fstr fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    pad_vfform(fmt, ap);
    va_end(ap);

    warnx(pad);
    warnx("\r\n");
}
