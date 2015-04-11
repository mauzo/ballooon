/* vi:set syn=cpp: */

#include <Arduino.h>
#include <Wire.h>

#include "gps.h"
#include "camera.h"
#include "radio.h"
#include "task.h"
#include "warn.h"

task *all_tasks[] = { 
    &gps_task, 
    &cam_task,
    &radio_task, 
    NULL
};

void 
setup (void)
{
    task    **t;

    Serial.begin(9600); // Start debug output
    Wire.begin(); //Start I2C link

    for (t = all_tasks; *t; t++)
        if ((*t)->setup) {
	    warnf(WDEBUG, "Calling setup for %s", (*t)->name);
            (*t)->setup();
    }
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

