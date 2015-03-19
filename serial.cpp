/* serial.cpp
 * Provide a serial port interface to C.
 */

#include "ballooon.h"

void
serial_setup (void)
{ 
    // Start debug output
    Serial.begin(9600);
}

EXT_C void
serial_write (const char *msg)
{
    Serial.print(msg);
}
