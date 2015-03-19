/* serial.cpp
 * Provide a serial port interface to C.
 */

#include "ballooon.h"

void
serial_setup (void)
{ 
    // Start debug output
    Serial.begin(9600);
    Serial.println("Testing, testing...");
}
