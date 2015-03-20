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
    task  **t;

    serial_setup();
    warn(WNOTICE, sF("Ballooon, starting..."));

    panic_handler = panic_in_setup;

    for (t = all_tasks; *t; t++) {
        warnf(WDEBUG, sF("Calling setup for [%s]"), (*t)->name);
        (*t)->setup();
    }

    warn(WDEBUG, sF("Finished setup()"));
}

void loop()
{ 
    task  **t;
    long    now, w;

    setjmp(panic_jb);
    panic_handler = panic_in_loop;

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

        if (now > 5000)
            panic(sF("Time's up!"));
    }

    panic(sF("Fell out of main loop!"));
}
