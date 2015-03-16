/* main.ino
 * Main loop.
 * vi:set syn=cpp:
 */
 
/* This has to be here or the wretched GUI won't link in the library */
#include <Wire.h>

#include "debug.h"
#include "gps.h"

void setup()
{
    debug_setup();
    gps_setup();
}

void loop()
{ 
    if(millis() > gpsCheckTime)
        gps_check();
}
