/* main.ino
 * Main loop.
 * vi:set syn=cpp:
 */
 
/* This has to be here or the wretched GUI won't link in the library */
#include <Wire.h>

#include "task.h"

#include "debug.h"
#include "gps.h"
#include "panic.h"

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
    long    now;

    for (t = all_tasks; *t; t++)
        (*t)->setup();

    PANIC_CATCH;

    while (1) {
        now = millis();
        for (t = all_tasks; *t; t++) {
            if (!(*t)->active)
                continue;
            if (now > (*t)->when)
                (*t)->run();
        }
    }

    panic("Fell out of main loop!");
}
