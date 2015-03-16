/* main.ino
 * Main loop.
 * vi:set syn=cpp:
 */
 
#include <Wire.h>

#include "gps.h"

void setup()
{
    Serial.begin(9600); // Start debug output
    gps_setup();
}

void loop()
{ 
    if(millis() > gpsCheckTime)
        gps_check();
}


