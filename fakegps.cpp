#include <Arduino.h>
#include "gps.h"
#include "task.h"

static wchan    fakegps_setup   (void);
static wchan    fakegps_run     (wchan why);

task            gps_task = {
    .name   = "FakeGPS",
    .when   = TASK_STOP,

    .setup  = fakegps_setup,
    .run    = fakegps_run,
    .reset  = 0,
};

gps_data    gps_last_fix = {
    .hr     = 17,
    .min    = 33,
    .sec    = 25,
    .lat    = 350020,
    .lon    = -20400,
    .alt    = 10,

    .num_sat    = 5,
    .fix_type   = 4,

    .itow       = 0xdeadbeef,
    .when       = 0,
};

static wchan
fakegps_setup (void)
{
    return TASK_DELAY(5000);
}

static wchan
fakegps_run (wchan why)
{
    gps_last_fix.when = millis();
    return TASK_TIME(why, 10000);
}
