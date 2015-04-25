/* vi:set syn=cpp: */

#include <Arduino.h>
#include <Wire.h>

#include "atomic.h"
#include "gps.h"
#include "camera.h"
#include "ntx.h"
#include "rtty.h"
#include "task.h"
#include "warn.h"

task *all_tasks[] = { 
    &gps_task,
    &cam_task,
#ifdef NTX_DEBUG
    &ntx_task,
#endif
    &rtty_task, 
    NULL
};

static volatile byte    swi_active      = 0;

void 
setup (void)
{
    task    **t;

    Serial.begin(9600); // Start debug output
    Wire.begin(); //Start I2C link

    for (t = all_tasks; *t; t++) {
        if ((*t)->setup) {
	    warnf(WDEBUG, "Calling setup for %s", (*t)->name);
            (*t)->when = (*t)->setup();
        }
        else {
            (*t)->when = TASK_RUN;
        }
    }
}

void 
loop (void)
{
    task    **t;
    long    now;
    byte    swi;
    wchan   w;
    wchan   (*r)(wchan);

    CRIT_START {
        now         = millis();
        swi         = swi_active;
        swi_active  = 0;
    } CRIT_END;

    for (t = all_tasks; *t; t++) {
        r = (*t)->run;
        if (!r) continue;

        w = (*t)->when;
        if (w.type == TASK_TYP_STOP) continue;

        if (w.time >= 0 && now > w.time) {
            (*t)->when = r(((wchan){ .time = now }));
            continue;
        }
        switch (w.type) {
        case TASK_TYP_IRQ:
            /* XXX */
            continue;
        case TASK_TYP_SWI:
            if (swi & (1 << w.value))
                (*t)->when = r(w);
            continue;
        }
    }
}

void
swi (byte i)
{
    CRIT_START {
        swi_active |= (1 << i);
    } CRIT_END;
}
