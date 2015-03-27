/* vi:set syn=cpp: */

#include <Arduino.h>
#include <Wire.h>

#include "gps.h"
#include "task.h"

task *all_tasks[] = { &gps_task, NULL };

void 
setup (void)
{
    task    **t;

    Serial.begin(9600); // Start debug output
    Wire.begin(); //Start I2C link

    for (t = all_tasks; *t; t++)
        if ((*t)->setup)
            (*t)->setup();
}

void 
loop (void)
{
    task            **t;
    unsigned long   now;

    now = millis();

    for (t = all_tasks; *t; t++)
        if ((*t)->run && now > (*t)->when)
            (*t)->run(now);
}

