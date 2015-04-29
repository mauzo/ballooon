/* sd.cpp
 * Functions to talk to the SD card.
 */

#include <string.h>

#include <Arduino.h>
#include <SdFat.h>

#include "sd.h"
#include "gps.h"
#include "warn.h"

#define SPI_SS  53

static wchan    sd_setup    (void);
static wchan    sd_run      (wchan now);

task sd_task = {
    .name   = "SD",
    .when   = TASK_STOP,

    .setup  = sd_setup,
    .run    = sd_run,
    .reset  = 0,
};

static SdFat    real_sd_vol;
static SdFat    *sd_vol = &real_sd_vol;

static wchan
sd_setup (void)
{
    File    f;
    byte    buf[] = "Time,Lat,Lon,Alt,No Sats,Fix Type";
    warnf(WDEBUG, "Buffer size: %i", sizeof(buf));
    int     rv;

    warn(WLOG, "Initialising SD card");
    //sd_vol = new SdFat;
    warn(WDEBUG, "Created SdFat object");

    if (!sd_vol->begin(SPI_SS))
        warn(WPANIC, "SD card failed to initialise");
    else
        warn(WDEBUG, "Initialised SD library");

    warn(WDEBUG, "Writing a file");
    f = sd_vol->open("test.txt", O_RDWR | O_CREAT | O_TRUNC);
    rv = f.write(buf, sizeof(buf) - 1);
    warnf(WDEBUG, "write() returned [%i]", rv);
    f.close();
    
    warn(WDEBUG, "Reading a file");
    memset(buf, 0, sizeof buf);
    f = sd_vol->open("test.txt", O_READ);
    rv = f.read(buf, sizeof(buf));
    warnf(WDEBUG, "read() returned [%i]: [%s]", rv, buf);
    f.close();

    return TASK_RUN;
}

static wchan
sd_run (wchan now)
{
    warn(WLOG, "sd_run");
    File f;
    int rv;
    
    if (!sd_vol->begin(SPI_SS))
        warn(WPANIC, "SD card failed to initialise");
    else
        warn(WDEBUG, "Initialised SD library");

    warn(WDEBUG, "Writing a file");
    f = sd_vol->open("test.txt", O_RDWR | O_AT_END);
    
    f.write(gps_last_fix.hr);
    f.write(gps_last_fix.min);
    f.write(gps_last_fix.sec);
    f.write(",");
    
    f.write(gps_last_fix.lat);
    f.write(",");
    
    f.write(gps_last_fix.lon);
    f.write(",");
    
    f.write(gps_last_fix.alt);
    f.write(",");
    
    f.write(gps_last_fix.num_sat);
    f.write(",");
    
    f.write(gps_last_fix.fix_type);
    f.write(",");
    
    f.close();
    
    warn(WDEBUG, "GPS data written to card");
    
    return TASK_TIME(now, 3000);
}

