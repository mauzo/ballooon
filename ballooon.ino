/* main.ino
 * Main loop.
 * vi:set syn=cpp:
 */
 
/* This has to be here or the wretched GUI won't link in the library */
#include <Wire.h>

#include "debug.h"
#include "gps.h"
#include "panic.h"

void setup()
{
    debug_setup();
}

void loop()
{ 
    PANIC_CATCH;

    gps_setup();

    while (1) {
        if(millis() > gpsCheckTime)
            gps_check();
    }

    panic("Fell out of main loop!");
}
