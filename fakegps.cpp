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

/*gps_data    gps_last_fix = {
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
};*/

//Reworded for CPP...
gps_data gps_last_fix;


static wchan
fakegps_setup (void)
{
    gps_last_fix.hr     = 17;
    gps_last_fix.min    = 33;
    gps_last_fix.sec    = 25;
    gps_last_fix.lat    = 350020;
    gps_last_fix.lon    = -20400;
    gps_last_fix.alt    = 10;
    
    gps_last_fix.num_sat    = 5;
    gps_last_fix.fix_type   = 4;
    
    gps_last_fix.itow       = 0xdeadbeef;
    gps_last_fix.when       = millis();

    return TASK_DELAY(5000);
}

static wchan
fakegps_run (wchan why)
{
    gps_last_fix.when   = millis();
    gps_last_fix.alt    += 10;
    swi(SWI_GPS);
    return TASK_SWI(SWI_NTXLO);
}
