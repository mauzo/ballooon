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
    debug_setup();
}

void loop()
{ 
    task  **t;
    long    now, w;

    for (t = all_tasks; *t; t++)
        (*t)->setup();

    PANIC_CATCH;

    while (1) {
        now = millis();
        for (t = all_tasks; *t; t++) {
            w = (*t)->when;
            if (w == TASK_INACTIVE)
                continue;
            if (now > w)
                (*t)->run(now);
        }
    }

    panic(F("Fell out of main loop!"));
}
