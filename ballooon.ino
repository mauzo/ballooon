/* main.ino
 * Main loop.
 * vi:set syn=cpp:
 */
 
/* This has to be here or the wretched GUI won't link in the library */
#include <Wire.h>

#include "ballooon.h"

task *all_tasks[] = {
    &gps_task,
    NULL
};

void setup()
{
    serial_setup();
    warn(WDEBUG, sF("Finished setup()"));
}

void loop()
{ 
    task  **t;
    long    now, w;

    for (t = all_tasks; *t; t++) {
        warnf(WDEBUG, sF("Calling setup for [%s]"), (*t)->name);
        (*t)->setup();
    }

    PANIC_CATCH;

    warn(WDEBUG, "Set setjmp");
    delay(1000);
    panic("panic");

    while (1) {
        now = millis();
        for (t = all_tasks; *t; t++) {
            w = (*t)->when;
            if (w == TASK_INACTIVE)
                continue;
            if (now > w) {
                warnf(WDEBUG, sF("Running task [%s] at [%lu]ms"),
                    (*t)->name, now);
                (*t)->run(now);
            }
        }
    }

    panic(sF("Fell out of main loop!"));
}
