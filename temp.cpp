/* temp.cpp
 * Talk to the temperature sensors.
 */

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "temp.h"
#include "warn.h"

#define PIN_BUS     4
#define RESOLUTION  9

static wchan    temp_setup      (void);
static wchan    temp_run        (wchan why);

task    temp_task   = {
    .name   = "temp",
    .when   = TASK_STOP,

    .setup  = temp_setup,
    .run    = temp_run,
    .reset  = 0,
};

byte    temp_ntemp;
float   temp_last_reading[NTEMP];

/* Actual C++ constructors... eww! */
static  OneWire             temp_bus(PIN_BUS);
static  DeviceAddress       temp_addrs[NTEMP];
static  DallasTemperature   temp_drv(&temp_bus);

static wchan
temp_setup (void)
{
    byte    *adr;

    temp_drv.begin();

    for (temp_ntemp = 0; temp_ntemp < NTEMP; temp_ntemp++) {
        adr = temp_addrs[temp_ntemp];

        if (!temp_bus.search(adr)) {
            warnf(WWARN, "Only found [%u] temperature sensors", 
                temp_ntemp);
            break;
        }

        warnf(WLOG, "Found sensor [%u] at " 
            "[%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x]", 
            temp_ntemp,
            adr[0], adr[1], adr[2], adr[3],
            adr[4], adr[5], adr[6], adr[7]
        );
        temp_drv.setResolution(adr, RESOLUTION);
    }

    return temp_ntemp ? TASK_RUN : TASK_STOP;
}

static wchan
temp_run (wchan why)
{
    byte    i;
    float   t;

    warn(WDEBUG, "Requesting temperature update");
    temp_drv.requestTemperatures();

    for (i = 0; i < temp_ntemp; i++) {
        t = temp_drv.getTempC(temp_addrs[i]);
        warnf(WLOG, "Temperature reading: sensor [%u], temp [%f]", i, t);

        temp_last_reading[i] = t;
    }

    swi(SWI_TEMP);
    return TASK_TIME(why, 5000);
}
