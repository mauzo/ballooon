/* debug.cpp
 * Debugging functions.
 *
 * For the moment, debugging goes via the serial port.
 */

#include <Arduino.h>
#include <HardwareSerial.h>

#include <stdarg.h>

#include "debug.h"

/* vsnprintf is normally in <stdio.h>, but the Arduino doesn't have
 * stdio so we can't include that. In fact, gcc recognises it as a
 * builtin, so we don't need a header (or an implementation to link
 * with) at all, but relying on this makes me slightly nervous so be
 * more explicit about the fact we are using the builtin.
 */
#define vsnprintf __builtin_vsnprintf

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
