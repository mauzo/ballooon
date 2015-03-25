/* vi:set syn=cpp: */

#include <Arduino.h>
#include <Wire.h>

#include "gps.h"

void 
setup (void)
{
    Serial.begin(9600); // Start debug output
    gps_setup(); 
}

void 
loop (void)
{
    gps_loop();
}

