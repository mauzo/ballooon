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
static void     time_string (gps_data *g,char *b);
static void     pos_string (gps_data *g,char *b);
static void     spd_string (gps_data *g,char *b);
static void     acc_string (gps_data *g,char *b);
static void     fix_string (gps_data *g,char *b);

task sd_task = {
    .name   = "SD",
    .when   = TASK_STOP,

    .setup  = sd_setup,
    .run    = sd_run,
    .reset  = 0,
};

static SdFat    real_sd_vol;
static SdFat    *sd_vol = &real_sd_vol;
static char     data_buf[30];

static wchan
sd_setup (void)
{
    File    f;
    byte    buf[] = "Time,Lat,Lon,Alt,velN,velE,VelD,hAcc,vAcc,sAcc,No Sats,Fix Type";
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
    
    if (!sd_vol->begin(SPI_SS))
        warn(WPANIC, "SD card failed to initialise");
    else
        warn(WDEBUG, "Initialised SD library");

    warn(WDEBUG, "Writing a file");
    f = sd_vol->open("test.txt", O_RDWR | O_AT_END);
    
    time_string(&gps_last_fix,data_buf);
    f.write(data_buf);
    
    pos_string(&gps_last_fix,data_buf);
    f.write(data_buf);
    
    spd_string(&gps_last_fix,data_buf);
    f.write(data_buf);
    
    acc_string(&gps_last_fix,data_buf);
    f.write(data_buf);
    
    fix_string(&gps_last_fix,data_buf);
    f.write(data_buf);
    
    f.write("\n");
    f.close();
    
    warn(WDEBUG, "GPS data written to card");
    
    return TASK_TIME(now, 10000);
}

void time_string
(gps_data *g, char *b)
{
    snprintf(b, sizeof(b), "%02u:%02u:%02u,", g->hr,g->min,g->sec);
}

void pos_string
(gps_data *g, char *b)
{
    long d = 10000000; // lat/lon scaling = 1e-7.
    snprintf(b, sizeof(b), "%+03li.%05li,%+03li.%05li,%lu,", g->lat/d,(g->lat % d)/100,g->lon/d,(g->lon % d)/100,g->alt); 
}

void spd_string
(gps_data *g, char *b)
{
    snprintf(b, sizeof(b), "%+li,%+li,%+li,", g->velN,g->velE,g->velD);
}

void acc_string
(gps_data *g, char *b)
{
    snprintf(b, sizeof(b), "%lu,%lu,%lu,", g->hAcc,g->vAcc,g->sAcc);
}

void fix_string
(gps_data *g, char *b)
{
    snprintf(b, sizeof(b), "%u,%u,", g->num_sat,g->fix_type);
}
