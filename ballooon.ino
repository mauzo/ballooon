/* main.ino
 * Main loop.
 * vi:set syn=cpp:
 */
 
#include "ballooon.h"

/* This has to be here or the wretched GUI won't link in the library */
#include <Wire.h>

void setup()
{
    serial_setup();
}

void loop()
{
    while (1) ;
}
