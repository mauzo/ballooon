/* vi:set syn=cpp: */

#include <Arduino.h>
#include <SdFat.h>
#include <Wire.h>

#include "atomic.h"
#include "gps.h"
#include "camera.h"
#include "ntx.h"
#include "rtty.h"
#include "sd.h"
#include "task.h"
#include "temp.h"
#include "warn.h"

task *all_tasks[] = {
    &gps_task,
    //&cam_task,
#ifdef NTX_DEBUG
    &ntx_task,
#endif
    &rtty_task, 
    //&sd_task,
    &temp_task,
    NULL
};

static volatile byte    swi_active      = 0;

void 
setup (void)
{
    task    **t;

    Serial.begin(9600); // Start debug output
    Wire.begin(); //Start I2C link
    
    //Delay to give a chance to get the monitor running, etc
    delay(3000);

    for (t = all_tasks; *t; t++) {
        if ((*t)->setup) {
	    warnf(WDEBUG, "Calling setup for [%s]", (*t)->name);
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
    byte    swis;
    wchan   w;
    wchan   (*r)(wchan);

    CRIT_START {
        now         = millis();
        swis        = swi_active;
        swi_active  = 0;
    } CRIT_END;

    for (t = all_tasks; *t; t++) {
        r = (*t)->run;
        if (!r) continue;

        w = (*t)->when;
        switch (WCHAN_TYPE(w)) {
        case TASK_TYP_STOP:
            break;
        case TASK_TYP_TIME:
            if (now > w) {
                warnf(WDEBUG, "Calling [%s] at time [%li]",
                    (*t)->name, now);
                (*t)->when = r(now);
            }
            break;
        case TASK_TYP_IRQ:
            /* XXX */
            break;
        case TASK_TYP_SWI:
            if (swis & WCHAN_VALUE(w)) {
                warnf(WDEBUG, "Calling [%s] for swi [%02x]",
                    (*t)->name, swis & WCHAN_VALUE(w));
                (*t)->when = r(w);
            }
            break;
        default:
            warn(WERROR, "Unknown wchan");
        }
    }
}

void
swi (byte i)
{
    CRIT_START {
        swi_active |= i;
    } CRIT_END;
}
