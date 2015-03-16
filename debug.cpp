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
warn (const char *fmt, ...)
{
    char        buf[256];
    va_list     ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    Serial.println(buf);
}
