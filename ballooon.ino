/* vi:set syn=cpp: */

#include <Arduino.h>
#include <SdFat.h>
#include <Wire.h>

#include "gps.h"
#include "camera.h"
#include "ntx.h"
#include "rtty.h"
#include "sd.h"
#include "task.h"
#include "warn.h"

task *all_tasks[] = {
    &gps_task,
    &cam_task,
#ifdef NTX_DEBUG
    &ntx_task,
#endif
    &rtty_task,
    &sd_task,
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

